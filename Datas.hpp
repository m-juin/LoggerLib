#ifndef __LOGGERLIB_DATAS_HPP__
#define __LOGGERLIB_DATAS_HPP__

#include <iostream>
#include <vector>

#include "./Utils.hpp"

namespace LoggerLib::Datas
{
    enum class E_Level
    {
        NONE = -1,
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        CRITICAL_ERROR = 4
    };

    struct LevelDatas
    {
        const std::string name;
        const std::string colorCode;
    };

    static const std::vector<LevelDatas> datas {
        {"DEBUG", Utils::Color::GREEN},
        {"INFO", Utils::Color::WHITE},
        {"WARNING", Utils::Color::YELLOW},
        {"ERROR", Utils::Color::RED},
        {"CRITICAL_ERROR", Utils::Color::RED},
    };

    struct LogMessage
    {
        const E_Level &Severity;
        const std::string &content;
        const size_t time;
    };
    

} // namespace LoggerLib::Datas

#endif // __LOGGERLIB_DATAS_HPP__