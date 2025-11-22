#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

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
					LoggerParameters(bool writeDate_, bool writeTime_, bool uniqueLogFile_)
						: writeDate(writeDate_), writeTime(writeTime_), uniqueLogFile(uniqueLogFile_)
					{
					}
			};

			inline static std::unique_ptr<Logger> instance = nullptr;

			std::mutex _logMutex;

			LoggerParameters params;
			LogLevel _loggingLevel;

			std::unique_ptr<std::ostream> _ownedStream;
			std::ostream *_loggingTarget = nullptr;

			bool printcolor;

			Logger(const LogLevel &level, std::ostream &fd, LoggerParameters params_)
				: params(params_), _loggingLevel(level), _loggingTarget(&fd),
				  printcolor((&fd == &std::cout) || (&fd == &std::cerr))
			{
			}

			Logger(const LogLevel &level, std::unique_ptr<std::ostream> fd, LoggerParameters params_)
				: params(params_), _loggingLevel(level), _ownedStream(std::move(fd)),
				  _loggingTarget(_ownedStream.get()), printcolor(false)
			{
			}

			static std::string FormatTime(const std::tm &tm, bool includeDate, bool includeTime)
			{
				std::stringstream ss;
				if (includeDate)
				{
					ss << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "/" << std::setw(2) << tm.tm_mday
					   << "/" << (1900 + tm.tm_year) << " ";
				}
				if (includeTime)
				{
					ss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":" << std::setw(2) << tm.tm_min << ":"
					   << std::setw(2) << tm.tm_sec << " ";
				}
				return ss.str();
			}

			static std::string GenerateUniqueFilename(const std::string &basePath, bool uniqueLogFile)
			{
				if (!uniqueLogFile)
				{
					return basePath + ".log";
				}

				std::time_t st = std::time(nullptr);
				auto tm = *std::localtime(&st);

				std::filesystem::path path(basePath);
				auto stem = path.stem();
				auto extension = path.extension();
				auto parent = path.parent_path();

				std::stringstream fileName;
				fileName << parent.string() << (parent.empty() ? "" : "/") << std::put_time(&tm, "%m_%d_%Y_%H_%M_%S")
						 << (stem.string().empty() ? "" : "_") << stem.string() << extension.string() << ".log";

				return fileName.str();
			}

		public:
			template <typename... Args>
			static void LogMessage(const LogLevel &severity, bool forceSeverity, Args &&...messageContent)
			{
				if (!instance)
					return;
				if ((forceSeverity == false && severity < instance->_loggingLevel) ||
					instance->_loggingLevel == LogLevel::NONE)
					return;

				auto data = Datas::datas[static_cast<int>(severity)];

				std::stringstream msg;

                std::time_t st = std::time(nullptr);
                auto tm = *localtime(&st);

                msg << FormatTime(tm, instance->params.writeDate, instance->params.writeTime);

				msg << (instance->printcolor ? data.colorCode : "") << "[" << data.name << "] ";

				if constexpr (sizeof...(Args) > 0)
				{
					(msg << ... << std::forward<Args>(messageContent));
				}

				std::string str = msg.str();

				std::unique_lock<std::mutex> lock(instance->_logMutex);
				(*instance->_loggingTarget) << str << (instance->printcolor ? Utils::Color::NONE : "") << std::endl;
			}

			static bool InitLogger(const LogLevel &level, std::string filePath = "",
								   LoggerParameters params_ = LoggerParameters(true, true, true))
			{

                if (filePath.empty() == true && params_.uniqueLogFile == false)
                    return InitLogger(level, std::cout, params_);

                filePath = GenerateUniqueFilename(filePath, params_.uniqueLogFile);
				auto file = std::make_unique<std::ofstream>(filePath);

				if (!file->is_open())
				{
					auto &err = Datas::datas[static_cast<int>(LogLevel::CRITICAL_ERROR)];
					std::cerr << err.colorCode << "[" << err.name << "] Failed to open log file." << Utils::Color::NONE
							  << std::endl;
					InitLogger(level, std::cout, params_);
					return false;
				}

				instance = std::unique_ptr<Logger>(new Logger(level, std::move(file), params_));
				LogMessage(LogLevel::INFO, true, "Logger initialized.");
				return true;
			}

			static bool InitLogger(const LogLevel &level, std::ostream &fd,
								   LoggerParameters params_ = LoggerParameters(true, true, true))
			{
				instance = std::unique_ptr<Logger>(new Logger(level, fd, params_));
				LogMessage(LogLevel::INFO, true, "Logger initialized.");
				return true;
			}

			template <typename... Args> static void LogInfo(Args &&...messageContent)
			{
				LogMessage(LogLevel::INFO, false, std::forward<Args>(messageContent)...);
			}

			template <typename... Args> static void LogWarning(Args &&...messageContent)
			{
				LogMessage(LogLevel::WARNING, false, std::forward<Args>(messageContent)...);
			}

			template <typename... Args> static void LogError(Args &&...messageContent)
			{
				LogMessage(LogLevel::ERROR, false, std::forward<Args>(messageContent)...);
			}

			template <typename... Args> static void LogCriticalError(Args &&...messageContent)
			{
				LogMessage(LogLevel::CRITICAL_ERROR, false, std::forward<Args>(messageContent)...);
			}

			template <typename... Args> static void LogDebug(Args &&...messageContent)
			{
				LogMessage(LogLevel::DEBUG, false, std::forward<Args>(messageContent)...);
			}

			Logger() = delete;
			~Logger()
			{
				if (_ownedStream)
				{
					if (auto *file = dynamic_cast<std::ofstream *>(_ownedStream.get()))
					{
						file->close();
					}
				}
			}
	};

} // namespace LoggerLib
