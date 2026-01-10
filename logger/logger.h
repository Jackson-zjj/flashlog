#pragma once

#include "log_common.h"
#include "handler/log_factory.h"
#include "handler/log_handle.h"

#define LOGGER_INIT(handler) logger::LogFactory::GetInstance().SetLogHandle(handler)

#define LOGGER_CALL(handler, level, ...)                                                                                        \
    if (handler) {                                                                                                              \
        handler->Log(level, logger::LogSourceLoc{__FILE__, __LINE__, static_cast<const char*>(__FUNCTION__)}, __VA_ARGS__);     \
    }

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_TRACE
#define LOGGER_TRACE(handler, ...) LOGGER_CALL(handler, logger::LogLevel::kTrace, __VA_ARGS__)
#define LOG_TRACE(...) LOGGER_TRACE(logger::LogFactory::GetInstance().GetLogHandle(), __VA_ARGS__)
#else
#define LOGGER_TRACE(handler, ...) void(0)
#define LOG_TRACE(...) void(0)
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_DEBUG
#define LOGGER_DEBUG(handler, ...) LOGGER_CALL(handler, logger::LogLevel::kDebug, __VA_ARGS__)
#define LOG_DEBUG(...) LOGGER_DEBUG(logger::LogFactory::GetInstance().GetLogHandle(), __VA_ARGS__)
#else
#define LOGGER_DEBUG(handler, ...) void(0)
#define LOG_DEBUG(...) void(0)
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_INFO
#define LOGGER_INFO(handler, ...) LOGGER_CALL(handler, logger::LogLevel::kInfo, __VA_ARGS__)
#define LOG_INFO(...) LOGGER_INFO(logger::LogFactory::GetInstance().GetLogHandle(), __VA_ARGS__)
#else
#define LOGGER_INFO(handler, ...) void(0)
#define LOG_INFO(...) void(0)
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_WARN
#define LOGGER_WARN(handler, ...) LOGGER_CALL(handler, logger::LogLevel::kWarn, __VA_ARGS__)
#define LOG_WARN(...) LOGGER_WARN(logger::LogFactory::GetInstance().GetLogHandle(), __VA_ARGS__)
#else
#define LOGGER_WARN(handler, ...) void(0)
#define LOG_WARN(...) void(0)
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_ERROR
#define LOGGER_ERROR(handler, ...) LOGGER_CALL(handler, logger::LogLevel::kError, __VA_ARGS__)
#define LOG_ERROR(...) LOGGER_ERROR(logger::LogFactory::GetInstance().GetLogHandle(), __VA_ARGS__)
#else
#define LOGGER_ERROR(handler, ...) void(0)
#define LOG_ERROR(...) void(0)
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_CRITICAL
#define LOGGER_CRITICAL(handler, ...) LOGGER_CALL(handler, logger::LogLevel::kFatal, __VA_ARGS__)
#define LOG_CRITICAL(...) LOGGER_CRITICAL(logger::LogFactory::GetInstance().GetLogHandle(), __VA_ARGS__)
#else
#define LOGGER_CRITICAL(handler, ...) void(0)
#define LOG_CRITICAL(...) void(0)
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_OFF
#define LOGGER_OFF(handler, ...) LOGGER_CALL(handler, logger::LogLevel::kError, __VA_ARGS__)
#define LOG_OFF(...) LOGGER_OFF(logger::LogFactory::GetInstance().GetLogHandle(), __VA_ARGS__)
#else
#define LOGGER_OFF(handler, ...) void(0)
#define LOG_OFF(...) void(0)
#endif