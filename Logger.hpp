#ifndef __LOGGERLIB_LOGGER_HPP__
#define __LOGGERLIB_LOGGER_HPP__

#include <memory>
#include <fstream>
#include <sstream>

#include "./Datas.hpp"

namespace LoggerLib
{
    class Logger
    {
    private:
        struct LoggerParameters
        {
            bool writeTime;
            LoggerParameters(bool writeTime_) : writeTime(writeTime_) {};
        };

        static std::unique_ptr<Logger> instance;

        const LoggerParameters &params;

        Logger(const Datas::E_Level &level, const std::ostream &fd = std::cout, const LoggerParameters &params_ = LoggerParameters(true)) : _loggingLevel(level), _loggingTarget(fd), params(params_)
        {
            
        }
        Datas::E_Level _loggingLevel;
        const std::ostream &_loggingTarget;

    public:
        static bool InitLogger(const Datas::E_Level &level, const std::ostream &fd = std::cout, const LoggerParameters &params_ = LoggerParameters(true));
        static bool InitLogger(const Datas::E_Level &level, const std::string &filePath = "", const LoggerParameters &params_ = LoggerParameters(true));

        static bool InitLogger(const Datas::E_Level &level, const std::string &filePath = "", const LoggerParameters &params_ = LoggerParameters(true))
        {
            if (level == Datas::E_Level::NONE || filePath == "")
                return InitLogger(level, std::cout, params_);
            else
            {
                std::fstream stream;
                try
                {
                    stream = std::fstream(filePath);
                }
                catch (const std::exception &e)
                {
                    const auto &ErrorDatas = Datas::datas[static_cast<int>(Datas::E_Level::CRITICAL_ERROR)];
                    std::cerr << ErrorDatas.colorCode << "[" << ErrorDatas.name << "] Failed to open log file." << Utils::Color::NONE << std::endl;
                    return false;
                }
                return InitLogger(level, stream, params_);
            }
        }

        static bool InitLogger(const Datas::E_Level &level, const std::ostream &fd = std::cout, const LoggerParameters &params_ = LoggerParameters(true))
        {
            instance = std::make_unique<Logger>(level, fd, params_);
            return true;
        }
        Logger() = delete;

        ~Logger() {}
    };
} // namespace LoggerLib

#endif // __LOGGERLIB_LOGGER_HPP__