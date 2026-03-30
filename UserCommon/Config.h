#pragma once
#include <string>
#include <optional>
#include <fstream>
#include <cctype>
#include <limits>  
#include "Logger.h"

namespace UserCommon_211645361_000000000 {

    inline bool ieq(const std::string& a, const std::string& b){
        if (a.size()!=b.size()) return false;
        for (size_t i=0;i<a.size();++i)
            if (std::tolower((unsigned char)a[i])!=std::tolower((unsigned char)b[i])) return false;
        return true;
    }

    inline void trim(std::string& s){
        auto a = s.find_first_not_of(" \t\r\n");
        auto b = s.find_last_not_of(" \t\r\n");
        s = (a==std::string::npos) ? "" : s.substr(a, b-a+1);
    }

    inline bool parseBool(const std::string& v){
        return ieq(v,"1")||ieq(v,"true")||ieq(v,"on")||ieq(v,"yes");
    }

    inline std::optional<LogLevel> parseLevelOpt(const std::string& v){
        if (ieq(v,"debug"))    return LogLevel::Debug;
        if (ieq(v,"info"))     return LogLevel::Info;
        if (ieq(v,"warning"))  return LogLevel::Warning;
        if (ieq(v,"error"))    return LogLevel::Error;
        if (ieq(v,"critical")) return LogLevel::Critical;
        if (ieq(v,"key"))      return LogLevel::Key;
        if (ieq(v,"illegal"))  return LogLevel::Illegal;
        return std::nullopt; // unknown -> not set
    }

    inline std::optional<size_t> parseSizeTOpt(const std::string& v){
        if (v.empty()) return std::nullopt;
        size_t i = (v[0]=='+') ? 1 : 0;
        if (i==v.size()) return std::nullopt;
        for (; i<v.size(); ++i)
            if (!std::isdigit((unsigned char)v[i])) return std::nullopt;
        try {
            unsigned long long x = std::stoull(v);
            if (x > std::numeric_limits<size_t>::max()) return std::nullopt;
            return static_cast<size_t>(x);
        } catch (...) {
            return std::nullopt;
        }
    }

    inline std::optional<size_t> parsePercentOpt(const std::string& v){
        auto s = parseSizeTOpt(v);
        if (!s) return std::nullopt;
        if (*s > 100) return std::nullopt; 
        return s;
    }

    struct MiniCfg {
        std::optional<std::string> simLogFilePrefix;
        std::optional<LogLevel>    simLogLevel;
        std::optional<bool>        simLogPrintToConsole;
        std::optional<bool>        simLogIncludeTID;

        std::optional<std::string> gmLogFilePrefix;
        std::optional<LogLevel>    gmLogLevel;
        std::optional<bool>        gmLogPrintToConsole;
        std::optional<bool>        gmLogIncludeTID;

        std::optional<std::string> mapDir;
        std::optional<size_t> rowsMax;
        std::optional<size_t> colsMax;
        std::optional<size_t> stepsMax;
        std::optional<size_t> shellsMax;
        std::optional<size_t> wallPct;
        std::optional<size_t> minePct;
        std::optional<size_t> emptyPct;
        std::optional<size_t> p1Pct;
        std::optional<size_t> p2Pct;
    };

    inline MiniCfg loadCfg(const std::string& path){
        MiniCfg c;
        std::ifstream in(path);
        if (!in) return c; // file is optional

        std::string line;
        while (std::getline(in, line)) {
            // strip inline comments
            if (auto p = line.find('#'); p != std::string::npos) line.erase(p);
            trim(line);
            if (line.empty()) continue;

            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq+1);
            trim(key); trim(val);

            if      (key=="sim.log.file")      { c.simLogFilePrefix   = val; }
            else if (key=="sim.log.level")     { c.simLogLevel        = parseLevelOpt(val); }
            else if (key=="sim.log.console")   { c.simLogPrintToConsole = parseBool(val); }
            else if (key=="sim.log.TID")       { c.simLogIncludeTID   = parseBool(val); }
            else if (key=="gm.log.file")       { c.gmLogFilePrefix    = val; }
            else if (key=="gm.log.level")      { c.gmLogLevel         = parseLevelOpt(val); }
            else if (key=="gm.log.console")    { c.gmLogPrintToConsole= parseBool(val); }
            else if (key=="gm.log.TID")        { c.gmLogIncludeTID    = parseBool(val); }
            else if (key=="map.dir")           { c.mapDir    = val; }
            else if (key=="map.rows.max")      { c.rowsMax   = parseSizeTOpt(val); }
            else if (key=="map.cols.max")      { c.colsMax   = parseSizeTOpt(val); }
            else if (key=="map.steps.max")     { c.stepsMax  = parseSizeTOpt(val); }
            else if (key=="map.shells.max")    { c.shellsMax = parseSizeTOpt(val); }
            else if (key=="map.wall.pct")      { c.wallPct   = parsePercentOpt(val); }
            else if (key=="map.mine.pct")      { c.minePct   = parsePercentOpt(val); }
            else if (key=="map.empty.pct")     { c.emptyPct  = parsePercentOpt(val); }
            else if (key=="map.p1.pct")        { c.p1Pct     = parsePercentOpt(val); }
            else if (key=="map.p2.pct")        { c.p2Pct     = parsePercentOpt(val); }
        }
        return c;
    }
}
