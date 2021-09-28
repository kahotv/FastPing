#include <cstdio>
#include <stdlib.h>
#include <memory>
#include <iostream>
#include <algorithm>
#include "config.h"
#include "fastping.h"

int main()
{
    if (system("sysctl -w net.ipv4.ping_group_range=\"0 0\"") == -1)
    {
        printf("[main]sysctl -w net.ipv4.ping_group_range=\"0 0\" failed[%d]", errno);
    }

    auto cfg = config::Load("config.ini");

    cfg->Show();

    fastping dp(cfg);

    dp.Start();

    while (true)
    {
        std::string cmd;


        std::cin >> cmd;

        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "exit")
            break;
    }

    dp.Stop();

    return 0;

    return 0;
}