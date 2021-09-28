#include "fastping.h"
#include <map>
#include <chrono>
#include <thread>
#include <algorithm>
#include <functional>

#include <memory.h>

#include <unistd.h>
#include <arpa/inet.h>

#include "util.h"
#include "icmp_util.h"

//开始测速
void fastping::Start()
{
	if (!m_run)
	{
		std::lock_guard<std::mutex> locker(m_run_lock);
		if (!m_run)
		{
			m_run = true;
			//先尝试拉取一次
			update_iplist_once(m_cfg->UpdateListFile);

			//启动拉取列表任务
			m_th_getiplist = std::thread(std::bind(&fastping::work_getiplist, this));
			//启动测速任务
			m_th_doping = std::thread(std::bind(&fastping::work_ping, this));
		}
	}
}
//停止测速
void fastping::Stop()
{
	if (m_run)
	{
		std::lock_guard<std::mutex> locker(m_run_lock);
		if (m_run)
		{
			m_run = false;
			//等待任务退出
			m_th_getiplist.join();

			m_th_doping.join();
		}
	}

}
//工作线程-更新ip列表
void fastping::work_getiplist()
{
	//更新ip列表线程
	using namespace std::chrono;

	auto last = high_resolution_clock::now();

	while (m_run)
	{
		auto now = high_resolution_clock::now();
		auto t = duration_cast<seconds>(now - last).count();

		if (t >= m_cfg->UpdateListTimeMinutes * 60)
		{
			update_iplist_once(m_cfg->UpdateListFile);

			last = now;
		}

		std::this_thread::sleep_for(milliseconds(500));
	}
}
//工作线程-ping一轮，每轮N次
void fastping::work_ping()
{
	//ping线程

	using namespace std::chrono;

	std::vector<uint32_t> iplist;
	std::unordered_map<uint32_t, ping_result> result;

	auto last = high_resolution_clock::now() - seconds(m_cfg->PingIntervalSecond + 1);

	while (m_run)
	{

		auto now = high_resolution_clock::now();
		auto t = duration_cast<seconds>(now - last).count();

		if (t >= m_cfg->PingIntervalSecond)
		{
			auto beg = high_resolution_clock::now();

			result.clear();
			//取IP列表
			get_ip_list_by_cache(iplist);
			//iplist.resize(9);
			//iplist.push_back(inet_addr("61.139.66.1"));

			//ping一轮
			ping_once(iplist,/*OUT*/ result);
			//上报ping结果
			show_ping_result(result);

			last = now;
			auto end = high_resolution_clock::now();
			printf("测速结束,总耗时: %dms\n", util::tpsub(beg, end));
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

//更新一次列表
bool fastping::update_iplist_once(const std::string& filepath)
{
	bool succ = false;
	std::vector<uint32_t> v;

	if (get_ip_list_by_file(filepath,/*OUT*/ v))
	{
		set_ip_list_cache(v);
		succ = true;
	}

	if (!succ)
	{
		printf("update_iplist_once error\n");
	}

	return succ;
}
//ping一轮
void fastping::ping_once(const std::vector<uint32_t>& iplist,/*OUT*/ std::unordered_map<uint32_t, ping_result>& result)
{
	using namespace std::chrono;
	using hight_tp = high_resolution_clock::time_point;

	uint32_t iplist_size = iplist.size();

	uint32_t PingTimeOutMs = m_cfg->PingTimeOutMs;			//测速超时时间
	uint32_t PingIPCount = m_cfg->PingIPCount;				//每个IP测速次数
	uint32_t PingIPIntervalMs = m_cfg->PingIPIntervalMs;	//每个IP测速间隔

	//理论耗时=次数*间隔+超时+1s
	printf("开始测速 次数: %d, 间隔%dms, 超时: %dms, 理论耗时(次数*间隔+超时+1秒): %dms\n",
		PingIPCount, PingIPIntervalMs, PingTimeOutMs,
		PingIPCount * PingIPIntervalMs + PingTimeOutMs + 1000
	);

	std::vector<int> s_list(PingIPCount);								//每次一个socket
	std::vector<std::thread> th_send_list(PingIPCount);					//每次一个线程
	std::vector<std::thread> th_recv_list(PingIPCount);					//每次一个线程
	std::vector <hight_tp> tp_list_send(PingIPCount);					//每次一个触发send时间
	std::vector <hight_tp> tp_list_recv(PingIPCount);					//每次一个触发recv时间
	std::vector<int64_t> tp_use_cache(PingIPCount * iplist_size, -1);		//测速结果		index*iplist_size + iplist.index

	hight_tp tp_begin_all = high_resolution_clock::now();				//总体begin时间
	hight_tp tp_begin_send = tp_begin_all + seconds(1);
	hight_tp tp_end =
		//总体end时间=总体tp_begin_send+测速间隔*测速次数+超时时间
		tp_begin_send + milliseconds(PingIPIntervalMs * PingIPCount + PingTimeOutMs);

	//printf("iplist: %d, tp_begin: %ldms, tp_end: %ldms\n", iplist_size, tpsub(tp_begin_all, tp_begin_all), tpsub(tp_begin_all, tp_end));

	//

	//struct timeval time_recv_timeout = {};
	//time_recv_timeout.tv_sec = 1;
	//time_recv_timeout.tv_usec = (PingTimeOutMs % 1000) * 1000;


	for (size_t i = 0; i < PingIPCount; i++)
	{
		//计算出每小轮的发送时间
		tp_list_send[i] = tp_begin_send + milliseconds(PingIPIntervalMs * i);
		tp_list_recv[i] = tp_begin_send + milliseconds(PingIPIntervalMs * i) + milliseconds(PingTimeOutMs);	//只等待N秒

		//申请socket
		s_list[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
		//setsockopt(s_list[i], SOL_SOCKET, SO_RCVTIMEO, &time_recv_timeout, sizeof(time_recv_timeout));

		//printf("第%d小轮: socket: %d, send_tp: %ldms, recv_tp: %ld\n", i, s_list[i], tpsub(tp_begin_all, tp_list_send[i]), tpsub(tp_begin_all, tp_list_recv[i]));
	}

	//每次收发work函数
	auto func_send = [&tp_begin_all](size_t index, int s, hight_tp tp_begin, const std::vector<uint32_t>& iplist)
	{
		size_t iplist_size = iplist.size();
		size_t count_succ = 0, count_err = 0;
		pid_t pid = getpid();
		sockaddr_in si = {};
		std::vector<uint8_t> buf_send(1024);
		uint32_t seq_begin = index * iplist_size;

		//printf("[func_send] 第%d小轮 send begin: %ldms, socket: %d\n", index, tpsub(tp_begin_all, tp_begin), s);

		std::this_thread::sleep_until(tp_begin);
		//auto beg = high_resolution_clock::now();
		//printf("[func_send] send now %ld\n", tpsub(tp_begin_all, beg));


		uint64_t now = high_resolution_clock::now().time_since_epoch().count();
		icmp_util::create_package_4_icmp_echo(buf_send, pid, now);

		auto databuf = buf_send.data();
		auto datasize = buf_send.size();

		for (uint32_t i = 0; i < iplist_size; i++)
		{
			si.sin_family = AF_INET;
			si.sin_addr.s_addr = iplist[i];

			icmp_util::change_package_4_icmp_echo(buf_send, seq_begin + i);

			size_t n = sendto(s, databuf, datasize, 0, (struct sockaddr*)&si, sizeof(si));

			if (n == datasize)
				count_succ++;
			else
				count_err++;
		}

		//auto end = high_resolution_clock::now();
		//printf("[func_send] 第%d小轮 end, 耗时: %ldms, 发送: %d/%d, err: (%d,%s)\n",index,tpsub(beg,end),count_succ, iplist.size(), errno, strerror(errno));
	};
	//小轮数，socket，结束时间点，icmp耗时缓存
	auto func_recv = [&tp_begin_all](size_t index, int s, hight_tp tp_begin, hight_tp tp_end, int64_t* tp_use_cache, uint32_t iplist_size)
	{
		char buf_recv[1024];
		sockaddr_in addr = {};
		socklen_t addrlen = sizeof(addr);

		uint32_t seq = 0;
		uint64_t tp = 0;

		//printf("[func_recv] 第%d小轮 recv tp_begin: %ld, tp_end: %ld, socket: %d\n",index, tpsub(tp_begin_all, tp_begin),tpsub(tp_begin_all, tp_end),s);
		std::this_thread::sleep_until(tp_begin);
		//auto beg = high_resolution_clock::now();
		//printf("[func_recv] recv now %ld\n", tpsub(tp_begin_all, beg));
		do
		{
			addr = {};
			addrlen = sizeof(addr);
			//printf("[func_recv] 第%d小轮 recvfrom socket: %d\n", index, s);
			auto size = recvfrom(s, buf_recv, sizeof(buf_recv), 0, (sockaddr*)&addr, &addrlen);
			auto now = high_resolution_clock::now();
			//printf("[func_recv] 第%d小轮 recvfrom tp: %ld, size: %d\n", index, tpsub(tp_begin_all, now), size);
			if (size <= 0)
			{
				if (errno == 4)
					continue;
				else
					break;
			}
			else
			{
				//接收成功，记录
				if (icmp_util::get_package_4_icmp_echo_info(buf_recv, &seq, &tp))
				{
					auto tpp = hight_tp(nanoseconds(tp));
					auto ns = (now - tpp).count();

					//seq=轮数*数量
					tp_use_cache[seq] = ns;
					/*
					if (addr.sin_addr.s_addr == inet_addr("61.139.66.1"))
					{
						printf("[func_recv] 第%d小轮 use %dms, socket: %d, addr: %s\n", index, ms, s, inet_ntoa(addr.sin_addr));
					}
					*/
				}
				else
				{
					printf("[func_recv] 第%d小轮 waht?, socket: %d, addr: %s\n", index, s, inet_ntoa(addr.sin_addr));
				}
			}

		} while (true);
		//auto end = high_resolution_clock::now();

		//printf("[func_recv] 第%d小轮 recv2 end: %ld, socket: %d\n", index, tpsub(tp_begin_all, end), s);
	};
	//优先启动recv线程
	for (size_t i = 0; i < th_recv_list.size(); i++)
	{
		th_recv_list[i] = std::thread(func_recv, i, s_list[i], tp_list_send[i], tp_list_recv[i], &tp_use_cache[0], iplist_size);
	}
	//再启动send线程
	for (size_t i = 0; i < th_send_list.size(); i++)
	{
		th_send_list[i] = std::thread(func_send, i, s_list[i], tp_list_send[i], iplist);
	}
	//printf("ready\n");
	//暂停指定时间来关闭recv socket
	std::this_thread::sleep_until(tp_end);
	//等待线程完成
	//printf("join begin\n");
	for (size_t i = 0; i < PingIPCount; i++)
	{
		shutdown(s_list[i], SHUT_RDWR);
		close(s_list[i]);
		//printf("closed socket: %d\n", s_list[i]);
	}

	for (size_t i = 0; i < PingIPCount; i++)
	{
		//printf("join 1\n");
		th_send_list[i].join();
		//printf("join 2\n");
		th_recv_list[i].join();
		//printf("join 3\n");
	}
	//printf("join end\n");
	//统计结果
	result.reserve(iplist_size);
	for (uint32_t n = 0; n < iplist_size; n++)
	{
		auto& r = result[iplist[n]];

		r.total_send = (uint16_t)PingIPCount;
		r.total_recv = 0;
		r.avg_time_us = 0;

		for (uint32_t i = 0; i < PingIPCount; i++)
		{
			int64_t time_ms = tp_use_cache[i * iplist_size + n];
			if (time_ms != -1)
			{
				r.total_recv++;
				r.avg_time_us += time_ms;
			}
		}
		//计算平均时间，只计算没丢包的
		if (r.total_recv == 0)
		{
			r.avg_time_us = -1;	//完全丢包
		}
		else
		{
			r.avg_time_us = r.avg_time_us / r.total_recv / 1000;
		}
	}

}

//打印测速结果
void fastping::show_ping_result(const std::unordered_map<uint32_t, ping_result>& result)
{
	//排序
	std::vector<uint32_t> iplist;
	std::vector<ping_result> pingresult;

	iplist.reserve(result.size());
	pingresult.reserve(result.size());

	/////////////////////////////////////////////
	//order by ip
	for (const auto& item : result)
	{
		iplist.push_back(ntohl(item.first));
	}

	std::sort(iplist.begin(), iplist.end());

	for (const auto& item : iplist)
	{
		auto it = result.find(htonl(item));
		if (it != result.end())
		{
			pingresult.push_back(it->second);
		}
	}
	/////////////////////////////////////////////

	for (size_t i = 0; i < iplist.size(); i++)
	{
		const uint32_t& ip = iplist[i];
		const ping_result& r = pingresult[i];

		double loss = 1;
		if(r.total_send != 0)
			loss = ((r.total_send - r.total_recv) / (double)r.total_send);

		printf("addr: %-15s, s/r: %3d/%-3d (%3.f%%), avg: %3d.%-3dms\n",
			inet_ntoa(in_addr{ htonl(ip) }),
			r.total_recv, r.total_send, loss * 100,
			r.avg_time_us / 1000, r.avg_time_us % 1000
		);
	}



}

//IP列表缓存操作
void fastping::get_ip_list_by_cache(/*OUT*/ std::vector<uint32_t>& v)
{
	std::lock_guard<std::mutex> locker(m_iplist_lock);

	v.resize(m_iplist.size());
	memcpy(&v[0], m_iplist.data(), m_iplist.size() * sizeof(uint32_t));
}
void fastping::set_ip_list_cache(const std::vector<uint32_t>& iplist)
{
	std::lock_guard<std::mutex> locker(m_iplist_lock);

	m_iplist.resize(iplist.size());
	memcpy(&m_iplist[0], iplist.data(), iplist.size() * sizeof(uint32_t));

	printf("set_ip_list_cache size: %d\n", iplist.size());

}

//从配置文件获取IP列表
bool fastping::get_ip_list_by_file(const std::string& filepath, std::vector<uint32_t>& out)
{
	std::string str;
	if (util::ReadFileBytes(filepath,/*OUT*/ str)) 
	{
		auto iplist = util::SplitString(str, ',');

		out.reserve(iplist.size());

		for (const auto& ip : iplist)
		{
			uint32_t ipval = inet_addr(util::Trim(ip).c_str());
			if (ipval != 0)
			{
				out.push_back(ipval);
			}
		}

		return out.size() != 0;
	}

	return false;
}