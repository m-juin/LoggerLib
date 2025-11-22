#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include "./Datas.hpp"

namespace LoggerLib
{

    using LogLevel = LoggerLib::Datas::E_Level;

    class Logger
    {
    private:
        struct LoggerParameters
        {
            bool writeDate;
            bool writeTime;
            bool uniqueLogFile;
            LoggerParameters(bool writeDate_, bool writeTime_, bool uniqueLogFile_) : writeDate(writeDate_), writeTime(writeTime_), uniqueLogFile(uniqueLogFile_) {}
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
            if (instance->params.writeTime == true ||instance->params.writeDate == true)
            {
                std::time_t st = std::time(nullptr);
                auto tm = *localtime(&st);
                if (instance->params.writeDate == true)
                    msg << tm.tm_mon + 1 << " " << tm.tm_mday << " " << 1900 + tm.tm_year << " ";
                if (instance->params.writeTime == true)
                    msg << tm.tm_hour << ":" << tm.tm_min << ":" << tm.tm_sec << " ";
            }
            msg << (instance->printcolor ? data.colorCode : "")
                << "[" << data.name << "] ";

            (msg << ... << messageContent);

            std::string str = msg.str();

            std::unique_lock<std::mutex> lock(instance->_logMutex);
            (*instance->_loggingTarget) << str << (instance->printcolor ? Utils::Color::NONE : "") << std::endl;
        }

        static bool InitLogger(const LogLevel &level, std::string filePath = "", LoggerParameters params_ = LoggerParameters(true, true, true))
        {
            if (filePath.empty())
                return InitLogger(level, std::cout, params_);

            if (params_.uniqueLogFile)
            {
                std::stringstream fileName;
                std::time_t st = std::time(nullptr);
                auto tm = *localtime(&st);
                std::filesystem::path path(filePath);
                auto name = path.filename();
                path = path.remove_filename();
                fileName << path.string() << tm.tm_mon + 1 << "_" << tm.tm_mday << "_" << 1900 + tm.tm_year << "_" << tm.tm_hour << "_" << tm.tm_min << "_" << tm.tm_sec << (name.empty() ? "" : "_") << name.string();
                std::cout << fileName.str() << std::endl;
                filePath = fileName.str();
            }
            filePath += ".log";
            auto file = std::make_unique<std::ofstream>(filePath);

            if (!file->is_open())
            {
                auto &err = Datas::datas[static_cast<int>(LogLevel::CRITICAL_ERROR)];
                std::cerr << err.colorCode << "[" << err.name << "] Failed to open log file." << Utils::Color::NONE << std::endl;
                InitLogger(level, std::cout, params_);
                return false;
            }

            instance = std::unique_ptr<Logger>(new Logger(level, std::move(file), params_));
            LogMessage(LogLevel::INFO, true, "Logger initialized.");
            return true;
        }

        static bool InitLogger(const LogLevel &level, std::ostream &fd, LoggerParameters params_ = LoggerParameters(true, true, true))
        {
            instance = std::unique_ptr<Logger>(new Logger(level, fd, params_));
            LogMessage(LogLevel::INFO, true, "Logger initialized.");
            return true;
        }

        Logger() = delete;
        ~Logger() = default;
    };

} // namespace LoggerLib
