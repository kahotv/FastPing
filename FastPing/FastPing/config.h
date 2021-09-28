#pragma once

#include <memory>
#include <string>

class config
{
public:

	//#string ip�б��ļ�
	std::string UpdateListFile = "iplist.txt";

	//#uint ��ȡ�б�ʱ���������ӣ�  Ĭ��5����Χ[5,10]
	uint32_t UpdateListTimeMinutes = 5;

	//#uint һ�ֲ��ټ�����룩 Ĭ��60����Χ[30,300]
	uint32_t PingIntervalSecond = 60;

	//#uint ÿ��IP�����������Σ�Ĭ��2����Χ[1,50]
	uint32_t PingIPCount = 10;

	//#uing ÿ��IP���ټ��(����)  Ĭ��10����Χ[10,100]
	uint32_t PingIPIntervalMs = 10;

	//#uint ���ٳ�ʱʱ�䣨���룩Ĭ��1000����Χ[100,2000]
	uint32_t PingTimeOutMs = 1000;

private:
	config() {}
public:
	static std::shared_ptr<const config> Load(const std::string& iniFilePath);

	void Show() const;

private:
	void check();
};

