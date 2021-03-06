#pragma once

#include <spdlog/spdlog.h>

#include <fstream>
#include <iostream>

#define LOG_TOGGLE TRUE

#if defined(_DEBUG) && LOG_TOGGLE

#define FARLOR_LOG_STARTUP_STDOUT() \
    try\
    {\
        spdlog::stdout_color_mt("console");\
    }\
    catch (const spdlog::spdlog_ex& ex)\
    {\
        std::cout << "Log Failed: "  << ex.what() << std::endl;\
    }

#define FARLOR_LOG_STARTUP_SIMPLE_FILE(X) \
    try\
    {\
        std::ofstream ofs;\
        ofs.open(X, std::ofstream::out | std::ofstream::trunc);\
        ofs.close();\
        spdlog::basic_logger_mt("console", X);\
    }\
    catch (const spdlog::spdlog_ex& ex)\
    {\
        std::cout << "Log Failed: "  << ex.what() << std::endl;\
    }

#define FARLOR_LOG_INFO(X, ...) \
    try\
    {\
        spdlog::get("console")->info(X, __VA_ARGS__);\
        spdlog::get("console")->flush();\
    }\
    catch (const spdlog::spdlog_ex& ex)\
    {\
        std::cout << "Log Failed: "  << ex.what() << std::endl;\
    }

#define FARLOR_LOG_WARNING(X, ...) \
    try\
    {\
        spdlog::get("console")->warn(X, __VA_ARGS__);\
        spdlog::get("console")->flush();\
    }\
    catch (const spdlog::spdlog_ex& ex)\
    {\
        std::cout << "Log Failed: "  << ex.what() << std::endl;\
    }

#define FARLOR_LOG_ERROR(X, ...) \
    try\
    {\
        spdlog::get("console")->error(X, __VA_ARGS__);\
        spdlog::get("console")->flush();\
    }\
    catch (const spdlog::spdlog_ex& ex)\
    {\
        std::cout << "Log Failed: "  << ex.what() << std::endl;\
    }

#define FARLOR_LOG_CRITICAL(X, ...) \
    try\
    {\
        spdlog::get("console")->critical(X, __VA_ARGS__);\
        spdlog::get("console")->flush();\
    }\
    catch (const spdlog::spdlog_ex& ex)\
    {\
        std::cout << "Log Failed: "  << ex.what() << std::endl;\
    }

#else

#define FARLOR_LOG_STARTUP_STDOUT()
#define FARLOR_LOG_STARTUP_SIMPLE_FILE(X)

#define FARLOR_LOG_INFO(...)
#define FARLOR_LOG_WARNING(...)
#define FARLOR_LOG_ERROR(...)
#define FARLOR_LOG_CRITICAL(...)

#endif
