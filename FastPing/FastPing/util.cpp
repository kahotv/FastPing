#include "util.h"
#include <fstream>
#include <algorithm>
std::string util::Trim(const std::string& str)
{
    std::string s = str;
    if (!s.empty())
    {
        s.erase(0, s.find_first_not_of(' '));
        s.erase(s.find_last_not_of(' ') + 1);

        s.erase(0, s.find_first_not_of('\r'));
        s.erase(s.find_last_not_of('\r') + 1);

        s.erase(0, s.find_first_not_of('\n'));
        s.erase(s.find_last_not_of('\n') + 1);

        s.erase(0, s.find_first_not_of('\t'));
        s.erase(s.find_last_not_of('\t') + 1);
    }
    //ÒÆ³ý¿Õ¸ñ
    return s;
}

std::vector<std::string> util::SplitString(const std::string& data, char c)
{
    std::vector<std::string> results;
    size_t pos = 0;
    while (pos < data.size())
    {
        size_t n_pos = data.find(c, pos);
        if (std::string::npos == n_pos)
        {
            auto str = data.substr(pos);
            if (!str.empty()) results.push_back(str);
            break;
        }
        auto str = data.substr(pos, n_pos - pos);
        if (!str.empty()) results.push_back(str);
        pos = n_pos + 1;
    }
    return results;
}

std::string util::Tolower(const std::string& str)
{
    std::string ret = str;

    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

    return ret;
}

uint64_t util::tpsub(std::chrono::high_resolution_clock::time_point& tp_beg, std::chrono::high_resolution_clock::time_point& tp_end)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_beg).count();
}

bool util::ReadFileBytes(const std::string& filepath, std::string& data)
{
    std::ifstream fs;

    fs.open(filepath, std::ios_base::in | std::ios_base::binary);

    if (!fs.is_open())
    {
        return false;
    }

    int len = 0;

    fs.seekg(0, std::ios::end);
    len = (int)fs.tellg();
    fs.seekg(0, std::ios::beg);
    data.resize(len);
    fs.read(&data[0], len);
    fs.close();

    return true;
}

