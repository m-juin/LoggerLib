#ifndef __LOGGER_UTILITY_FUNCTIONS_HPP__
#define __LOGGER_UTILITY_FUNCTIONS_HPP__

#include "./Logger.hpp"

namespace LoggerLib
{
	template <typename... Args> static void LogInfo(Args &&...messageContent)
	{
		Logger::LogMessage(LogLevel::INFO, false, std::forward<Args>(messageContent)...);
	}

	template <typename... Args> static void LogWarning(Args &&...messageContent)
	{
		Logger::LogMessage(LogLevel::WARNING, false, std::forward<Args>(messageContent)...);
	}

	template <typename... Args> static void LogError(Args &&...messageContent)
	{
		Logger::LogMessage(LogLevel::ERROR, false, std::forward<Args>(messageContent)...);
	}

	template <typename... Args> static void LogCriticalError(Args &&...messageContent)
	{
		Logger::LogMessage(LogLevel::CRITICAL_ERROR, false, std::forward<Args>(messageContent)...);
	}

	template <typename... Args> static void LogDebug(Args &&...messageContent)
	{
		Logger::LogMessage(LogLevel::DEBUG, false, std::forward<Args>(messageContent)...);
	}
} // namespace LoggerLib

#endif // __UTILITY_FUNCTIONS_HPP__