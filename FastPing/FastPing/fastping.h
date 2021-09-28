#pragma once
#include "config.h"
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>

class fastping
{
private:
	struct ping_result
	{
		uint16_t total_send;
		uint16_t total_recv;
		int64_t avg_time_us;
	};

public:
	fastping(std::shared_ptr<const config> cfg)
	{
		m_cfg = cfg;
	}
	//开始测速
	void Start();
	//停止测速
	void Stop();
private:
	//工作线程-更新ip列表
	void work_getiplist();
	//工作线程-ping一轮，每轮N次
	void work_ping();
	
	//更新一次列表
	bool update_iplist_once(const std::string& filepath);
	//ping一轮
	void ping_once(const std::vector<uint32_t>& iplist,/*OUT*/ std::unordered_map<uint32_t, ping_result>& result);


	//打印测速结果
	void show_ping_result(const std::unordered_map<uint32_t, ping_result>& result);

	//IP列表缓存操作
	void get_ip_list_by_cache(/*OUT*/ std::vector<uint32_t>& v);
	void set_ip_list_cache(const std::vector<uint32_t>& iplist);

	//从配置文件获取IP列表
	bool get_ip_list_by_file(const std::string& filepath, std::vector<uint32_t>& out);

	std::shared_ptr <const config> m_cfg;

	std::vector<uint32_t> m_iplist;
	std::mutex m_iplist_lock;

	std::thread m_th_getiplist, m_th_doping;

	std::mutex m_run_lock;
	volatile bool m_run;

};

