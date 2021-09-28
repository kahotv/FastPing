#include "icmp_util.h"
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <memory.h>
//效验算法（百度下有注释，但是还是看不太明白）  
unsigned short icmp_util::cal_chksum(unsigned short* addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short* w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

void icmp_util::create_package_4_icmp_echo(std::vector<uint8_t>& buf, int pid, const uint64_t timespan)
{
    struct ip* iph;
    struct icmp* icmp;

    int size = 8 + sizeof(timespan);

    buf.resize(size);

    icmp = (struct icmp*)&buf[0];
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = 0;
    icmp->icmp_id = pid;

    *(uint64_t*)icmp->icmp_data = timespan;
}
void icmp_util::change_package_4_icmp_echo(std::vector<uint8_t>& buf, size_t seq)
{
    struct icmp* z = (struct icmp*)&buf[0];
    z->icmp_seq = seq;
    z->icmp_cksum = 0;
    z->icmp_cksum = cal_chksum((unsigned short*)z, buf.size());
}

bool icmp_util::get_package_4_icmp_echo_info(const char* buf, uint32_t* seq, uint64_t* time)
{
    struct icmp* p;

    p = (struct icmp*)&buf[0];
    *seq = p->icmp_seq;
    *time = *(uint64_t*)p->icmp_data;

    return true;
}