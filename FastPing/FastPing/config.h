#pragma once

#include <memory>
#include <string>

class config
{
public:

	//#string ip列表文件
	std::string UpdateListFile = "iplist.txt";

	//#uint 拉取列表时间间隔（分钟）  默认5，范围[5,10]
	uint32_t UpdateListTimeMinutes = 5;

	//#uint 一轮测速间隔（秒） 默认60，范围[30,300]
	uint32_t PingIntervalSecond = 60;

	//#uint 每个IP测数次数（次）默认2，范围[1,50]
	uint32_t PingIPCount = 10;

	//#uing 每个IP测速间隔(毫秒)  默认10，范围[10,100]
	uint32_t PingIPIntervalMs = 10;

	//#uint 测速超时时间（毫秒）默认1000，范围[100,2000]
	uint32_t PingTimeOutMs = 1000;

private:
	config() {}
public:
	static std::shared_ptr<const config> Load(const std::string& iniFilePath);

	void Show() const;

private:
	void check();
};

