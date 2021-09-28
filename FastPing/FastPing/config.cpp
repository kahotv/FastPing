#include "config.h"
#include "util.h"
std::shared_ptr<const config> config::Load(const std::string& iniFilePath)
{
	config* cfg = new config();
	std::string str;
	if (util::ReadFileBytes(iniFilePath, str))
	{
		util::ReadIni(iniFilePath, "UpdateListFile", cfg->UpdateListFile);
		util::ReadIni(iniFilePath, "UpdateListTimeMinutes", cfg->UpdateListTimeMinutes);
		util::ReadIni(iniFilePath, "PingIntervalSecond", cfg->PingIntervalSecond);
		util::ReadIni(iniFilePath, "PingIPCount", cfg->PingIPCount);
		util::ReadIni(iniFilePath, "PingIPIntervalMs", cfg->PingIPIntervalMs);
		util::ReadIni(iniFilePath, "PingTimeOutMs", cfg->PingTimeOutMs);

		cfg->check();
	}

	return std::shared_ptr<const config>(cfg);
}

void config::Show() const
{
	char buf[0x100];
	std::string str;

	sprintf(buf, "%-25s = %s\n", "UpdateListFile", UpdateListFile.c_str()); str += buf;
	sprintf(buf, "%-25s = %d\n", "UpdateListTimeMinutes", UpdateListTimeMinutes); str += buf;
	sprintf(buf, "%-25s = %d\n", "PingIntervalSecond", PingIntervalSecond); str += buf;
	sprintf(buf, "%-25s = %d\n", "PingIPCount", PingIPCount); str += buf;
	sprintf(buf, "%-25s = %d\n", "PingIPIntervalMs", PingIPIntervalMs); str += buf;
	sprintf(buf, "%-25s = %d\n", "PingTimeOutMs", PingTimeOutMs); str += buf;

	printf("%s\n", str.c_str());
}


void config::check()
{
	if (!(UpdateListTimeMinutes >= 5 && UpdateListTimeMinutes <= 10))
		UpdateListTimeMinutes = 5;

	if (!(PingIntervalSecond >= 30 && PingIntervalSecond <= 300))
		PingIntervalSecond = 60;

	if (!(PingIPCount >= 1 && PingIPCount <= 50))
		PingIPCount = 10;

	if (!(PingIPIntervalMs >= 10 && PingIPIntervalMs <= 100))
		PingIPIntervalMs = 10;

	if (!(PingTimeOutMs >= 100 && PingTimeOutMs <= 2000))
		PingTimeOutMs = 1000;
}