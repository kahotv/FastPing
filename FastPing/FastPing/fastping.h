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
	//��ʼ����
	void Start();
	//ֹͣ����
	void Stop();
private:
	//�����߳�-����ip�б�
	void work_getiplist();
	//�����߳�-pingһ�֣�ÿ��N��
	void work_ping();
	
	//����һ���б�
	bool update_iplist_once(const std::string& filepath);
	//pingһ��
	void ping_once(const std::vector<uint32_t>& iplist,/*OUT*/ std::unordered_map<uint32_t, ping_result>& result);


	//��ӡ���ٽ��
	void show_ping_result(const std::unordered_map<uint32_t, ping_result>& result);

	//IP�б������
	void get_ip_list_by_cache(/*OUT*/ std::vector<uint32_t>& v);
	void set_ip_list_cache(const std::vector<uint32_t>& iplist);

	//�������ļ���ȡIP�б�
	bool get_ip_list_by_file(const std::string& filepath, std::vector<uint32_t>& out);

	std::shared_ptr <const config> m_cfg;

	std::vector<uint32_t> m_iplist;
	std::mutex m_iplist_lock;

	std::thread m_th_getiplist, m_th_doping;

	std::mutex m_run_lock;
	volatile bool m_run;

};

