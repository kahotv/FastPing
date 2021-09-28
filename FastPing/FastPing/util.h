#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <sstream>
class util
{
public:
    //string
    static std::string Trim(const std::string& str);
    static std::vector<std::string> SplitString(const std::string& data, char c);
    static std::string Tolower(const std::string& str);

    //time
    static uint64_t tpsub(std::chrono::high_resolution_clock::time_point& tp_beg, std::chrono::high_resolution_clock::time_point& tp_end);


    //file
    static bool ReadFileBytes(const std::string& filepath, std::string& data);
    //ini
    template <typename T>
    static bool ReadIni(const std::string& inipath, const std::string& key, T& val)
    {
        std::string str;
        if (!util::ReadFileBytes(inipath, str))
        {
            return false;
        }

        std::string key_dst = util::Tolower(key);

        std::vector<std::string> lines = SplitString(str, '\n');

        for (auto& line : lines)
        {
            if (line.size() == 0 || line.front() == '#')
                continue;

            line = Trim(line);

            auto kv = SplitString(line, '=');

            if (kv.size() == 2)
            {
                std::string key_src = util::Tolower(util::Trim(kv[0]));

                if (key_dst == key_src)
                {
                    //stringתT
                    std::stringstream ss(kv[1]);
                    ss >> val;
                    return true;
                }
            }


        }

        return false;
    }

};

