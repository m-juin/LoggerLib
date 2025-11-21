#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iostream>

#include "./Datas.hpp"

namespace LoggerLib
{

    using LogLevel = LoggerLib::Datas::E_Level;

    class Logger
    {
    private:
        struct LoggerParameters
        {
            bool writeTime;
            LoggerParameters(bool writeTime_) : writeTime(writeTime_) {}
        };

        inline static std::unique_ptr<Logger> instance = nullptr;

        std::mutex _logMutex;

        LoggerParameters params;
        LogLevel _loggingLevel;

        std::unique_ptr<std::ostream> _ownedStream;
        std::ostream *_loggingTarget = nullptr;

        bool printcolor;

    private:
        Logger(const LogLevel &level, std::ostream &fd, LoggerParameters params_)
            : params(params_),
              _loggingLevel(level),
              _loggingTarget(&fd),
              printcolor((&fd == &std::cout) || (&fd == &std::cerr))
        {
        }

        Logger(const LogLevel &level, std::unique_ptr<std::ostream> fd, LoggerParameters params_)
            : params(params_),
              _loggingLevel(level),
              _ownedStream(std::move(fd)),
              _loggingTarget(_ownedStream.get()),
              printcolor(false)
        {
        }

    public:
        template <typename... Args>
        static void LogMessage(const LogLevel &severity, bool forceSeverity, Args &&...messageContent)
        {
            if (!instance)
                return;
            if ((forceSeverity == false && severity < instance->_loggingLevel) || instance->_loggingLevel == LogLevel::NONE)
                return;

            auto data = Datas::datas[static_cast<int>(severity)];

            std::stringstream msg;
            msg << (instance->params.writeTime ? "HOUR " : "")
                << (instance->printcolor ? data.colorCode : "")
                << "[" << data.name << "] ";

            (msg << ... << messageContent);

            std::string str = msg.str();

            std::unique_lock<std::mutex> lock(instance->_logMutex);
            (*instance->_loggingTarget) << str << (instance->printcolor ? Utils::Color::NONE : "") << std::endl;
        }

        static bool InitLogger(const LogLevel &level, std::string filePath = "", LoggerParameters params_ = LoggerParameters(true))
        {
            if (filePath.empty())
                return InitLogger(level, std::cout, params_);

            auto file = std::make_unique<std::ofstream>(filePath);

            if (!file->is_open())
            {
                auto &err = Datas::datas[static_cast<int>(LogLevel::CRITICAL_ERROR)];
                std::cerr << err.colorCode << "[" << err.name << "] Failed to open log file." << Utils::Color::NONE << std::endl;
                return false;
            }

            instance = std::unique_ptr<Logger>(new Logger(level, std::move(file), params_));
            LogMessage(LogLevel::INFO, true, "Logger initialized.");
            return true;
        }

        static bool InitLogger(const LogLevel &level, std::ostream &fd, LoggerParameters params_ = LoggerParameters(true))
        {
            instance = std::unique_ptr<Logger>(new Logger(level, fd, params_));
            LogMessage(LogLevel::INFO, true, "Logger initialized.");
            return true;
        }

        Logger() = delete;
        ~Logger() = default;
    };

} // namespace LoggerLib
