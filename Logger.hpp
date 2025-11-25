#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>

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
					bool threaded;

					LoggerParameters(bool writeDate_, bool writeTime_, bool uniqueLogFile_, bool threaded_)
						: writeDate(writeDate_), writeTime(writeTime_), uniqueLogFile(uniqueLogFile_),
						  threaded(threaded_)
					{
					}
			};

			struct MessageData
			{
					std::string severity;
					std::string message;
					std::string colorCode;
					bool printColor;
					bool writeDate;
					bool writeTime;

					MessageData( const std::string &sev, const std::string &msg, const std::string &color, bool colorFlag,
								bool date, bool time)
						: severity(sev), message(msg), colorCode(color), printColor(colorFlag), writeDate(date),
						  writeTime(time)
					{
					}
			};

			inline static std::unique_ptr<Logger> instance = nullptr;

			LoggerParameters _params;
			LogLevel _loggingLevel;
			std::unique_ptr<std::ostream> _ownedStream;
			std::ostream *_loggingTarget = nullptr;

			std::unique_ptr<std::thread> _thread = nullptr;
			std::mutex _queueMutex;
			std::atomic<bool> _threadStop;
			std::condition_variable _cv;

			std::mutex _logMutex;


			std::queue<MessageData> _waitingMessage;

			bool _printcolor;

			Logger(const LogLevel &level, std::ostream &fd, LoggerParameters params_)
				: _params(params_), _loggingLevel(level), _loggingTarget(&fd),
				  _threadStop(false), _printcolor((&fd == &std::cout) || (&fd == &std::cerr))
			{
			}

			Logger(const LogLevel &level, std::unique_ptr<std::ostream> fd, LoggerParameters params_)
				: _params(params_), _loggingLevel(level), _ownedStream(std::move(fd)),
				  _loggingTarget(_ownedStream.get()), _threadStop(false), _printcolor(false)
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

			void PrintMessage(const MessageData &msgData)
			{
				// auto data = Datas::datas[static_cast<int>(msgData.severity)];

				std::stringstream finalMsg;

				std::time_t st = std::time(nullptr);
				auto tm = *std::localtime(&st);
				finalMsg << FormatTime(tm, msgData.writeDate, msgData.writeTime);

				finalMsg << (msgData.printColor ? msgData.colorCode : "") << "[" << msgData.severity << "] " << msgData.message
						 << (msgData.printColor ? Utils::Color::NONE : "");

				std::lock_guard<std::mutex> logLock(_logMutex);
				(*_loggingTarget) << finalMsg.str() << std::endl;
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

			void ProcessMessages()
			{
				while (true)
				{
					std::unique_lock<std::mutex> lock(_queueMutex);
					_cv.wait(lock, [this]() { return !_waitingMessage.empty() || _threadStop.load(); });

					if (_threadStop.load() && _waitingMessage.empty())
					{
						return;
					}

					if (!_waitingMessage.empty())
					{
						auto message = std::move(_waitingMessage.front());
						_waitingMessage.pop();
						lock.unlock();

						PrintMessage(message);
					}
				}
			}

			void StartThread()
			{
				_threadStop = false;
				_thread = std::make_unique<std::thread>([this]() { this->ProcessMessages(); });
			}

		public:
			template <typename... Args>
			static void LogMessage(const LogLevel &severity, bool forceSeverity, Args &&...messageContent)
			{
				if (!instance)
					return;
				if ((!forceSeverity && severity < instance->_loggingLevel) || instance->_loggingLevel == LogLevel::NONE)
					return;

				std::stringstream msgStream;
				((msgStream << std::forward<Args>(messageContent)), ...);
				std::string messageStr = msgStream.str();

				auto &data = Datas::datas[static_cast<int>(severity)];

				MessageData msgData(data.name, messageStr, data.colorCode, instance->_printcolor,
									instance->_params.writeDate, instance->_params.writeTime);

				if (instance->_params.threaded)
				{
					std::lock_guard<std::mutex> lock(instance->_queueMutex);
					instance->_waitingMessage.push(std::move(msgData));
					instance->_cv.notify_one();
				}
				else
				{
					instance->PrintMessage(msgData);
				}
			}

			static bool InitLogger(const LogLevel &level, std::string filePath = "",
								   LoggerParameters params_ = LoggerParameters(true, true, true, false))
			{
				if (filePath.empty() && !params_.uniqueLogFile)
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
				if (params_.threaded)
				{
					instance->StartThread();
				}
				LogMessage(LogLevel::INFO, true, "Logger initialized.");
				return true;
			}

			static bool InitLogger(const LogLevel &level, std::ostream &fd,
								   LoggerParameters params_ = LoggerParameters(true, true, true, false))
			{
				instance = std::unique_ptr<Logger>(new Logger(level, fd, params_));
				if (params_.threaded)
				{
					instance->StartThread();
				}
				LogMessage(LogLevel::INFO, true, "Logger initialized.");
				return true;
			}

			static void Shutdown()
			{
				if (instance)
				{
					instance->_threadStop = true;
					instance->_cv.notify_one();

					if (instance->_thread && instance->_thread->joinable())
					{
						instance->_thread->join();
					}

					instance.reset();
				}
			}

			Logger() = delete;

			~Logger()
			{
				_threadStop = true;
				_cv.notify_one();

				if (_thread && _thread->joinable())
				{
					_thread->join();
				}

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

#endif // __LOGGER_HPP__