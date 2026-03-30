#pragma once
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <filesystem>
#include <cstdio>

namespace UserCommon_211645361_000000000{
    enum class LogLevel { Debug, Info, Illegal, Key, Warning, Error, Critical, UNKNOWN };

    class Logger {
    public:
        explicit Logger(const std::string& filePath, bool printToConsole = false, bool includeTID = true, LogLevel minLevel = LogLevel::Debug): minLevel_(minLevel){
            // Ensure parent directory exists (no-throw via error_code)
            this->printToConsole_ = printToConsole;
            this->includeTID_ = includeTID;
            std::error_code ec;
            std::filesystem::path p(filePath);
            if (!p.parent_path().empty()) {
                std::filesystem::create_directories(p.parent_path(), ec);
            }

            // Open in append (creates file if missing)
            if(this->minLevel_ == LogLevel::UNKNOWN) return;
            out_.open(filePath, std::ios::out | std::ios::app);

            // If that still failed, "touch" then reopen
            if (!out_.is_open()) {
                std::ofstream touch(filePath, std::ios::out | std::ios::trunc);
                if (touch.is_open()) touch.close();
                out_.open(filePath, std::ios::out | std::ios::app);
            }

            if (!out_.is_open()) {
                std::ostringstream oss;
                oss << "[LOGGER] Failed to open log file: " << filePath << "\n";
                std::fwrite(oss.str().c_str(), 1, oss.str().size(), stderr);
            }
        }

        // Level helpers
        void debug   (const std::string& msg) { log(LogLevel::Debug,   msg); }
        void info    (const std::string& msg) { log(LogLevel::Info,    msg); }
        void illegal (const std::string& msg) { log(LogLevel::Illegal, msg); }
        void key     (const std::string& msg) { log(LogLevel::Key,     msg); }
        void warn    (const std::string& msg) { log(LogLevel::Warning, msg); }
        void error   (const std::string& msg) { log(LogLevel::Error,   msg); }
        void critical(const std::string& msg) { log(LogLevel::Critical,msg); }

        void log(LogLevel level, const std::string& message) {
            if(this->minLevel_ == LogLevel::UNKNOWN) return;
            const auto now = std::chrono::system_clock::now();
            const auto t   = std::chrono::system_clock::to_time_t(now);

            std::lock_guard<std::mutex> lock(m_);
            if (level < minLevel_) return;

            std::tm tm{};
            ::localtime_r(&t, &tm);

            std::ostringstream line;
            line << '[' << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
                 << '[' << levelToString(level) << "] ";
            if (this->includeTID_)
                line << "[thread:" << std::this_thread::get_id() << "] ";
            line << message << '\n';

            const std::string s = line.str();

            if (this->printToConsole_) {
                std::fwrite(s.c_str(), 1, s.size(), stdout); 
                std::fflush(stdout);
            }

            if (out_.is_open()) {
                out_ << s;
                out_.flush();
            }
        }

        void setMinLevel(LogLevel lvl) { std::lock_guard<std::mutex> lock(m_); minLevel_ = lvl; }

        static std::string makeDatedFilename(const std::string& dir, const std::string& base) {
            const auto now = std::chrono::system_clock::now();
            const auto t   = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
            ::localtime_r(&t, &tm);
            std::ostringstream oss;
            oss << dir << '/' << base << '_'
                << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".log";
            return oss.str();
        }

    private:
        static const char* levelToString(LogLevel lvl) {
            switch (lvl) {
                case LogLevel::Debug:    return "DEBUG";
                case LogLevel::Info:     return "INFO";
                case LogLevel::Illegal:  return "ILLEGAL";
                case LogLevel::Key:      return "KEY";
                case LogLevel::Warning:  return "WARNING";
                case LogLevel::Error:    return "ERROR";
                case LogLevel::Critical: return "CRITICAL";
                default:                 return "UNKNOWN";
            }
        }

        std::ofstream out_;
        std::mutex    m_;
        LogLevel      minLevel_;
        bool printToConsole_;
        bool includeTID_;
    };
}

