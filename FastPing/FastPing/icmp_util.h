#pragma once
#include <string>
#include <vector>
#include <inttypes.h>
class icmp_util
{
public:
	static void create_package_4_icmp_echo(std::vector<uint8_t>& buf, int pid, const uint64_t timespan);
	static void change_package_4_icmp_echo(std::vector<uint8_t>& buf, size_t seq);

	static bool get_package_4_icmp_echo_info(const char* buf, uint32_t* seq, uint64_t* time);

private:
	static unsigned short cal_chksum(unsigned short* addr, int len);
};

