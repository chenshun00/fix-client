#pragma once
#ifndef FIXCLIENTLOGFACTORY_H
#define FIXCLIENTLOGFACTORY_H

#include <quickfix/Log.h>
#include <quickfix/Exceptions.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/FieldConvertors.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <string>
#include <utility>

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 4290 )
#endif

class FixClientLogFactory : public FIX::LogFactory {
public:
    explicit FixClientLogFactory(FIX::SessionSettings settings) : m_settings(std::move(settings)) {
        auto dict = m_settings.get();
        if (dict.has(FIX::FILE_LOG_PATH)) {
            this->m_path = dict.getString(FIX::FILE_LOG_PATH);
        } else {
            throw FIX::ConfigError("FileLogPath");
        }

//        auto logger = spdlog::rotating_logger_mt("global", this->m_path + "/Logs/RotatingFileLog.txt",
//                                                 1024 * 1024 * 1024, 5);
//        spdlog::set_default_logger(logger);
    };

    explicit FixClientLogFactory(std::string path) : m_path(std::move(path)) {};

    ~FixClientLogFactory() override = default;

public:
    FIX::Log *create() override;

    FIX::Log *create(const FIX::SessionID &) override;

    void destroy(FIX::Log *log) override;

private:
    std::string m_path;
    FIX::SessionSettings m_settings;
};


class FixClientLog : public FIX::Log {

public:
    explicit FixClientLog(const std::string &path);

    FixClientLog(const std::string &path, const FIX::SessionID &sessionID);

    ~FixClientLog() override = default;

    void clear() override {}

    void backup() override {}

    void onIncoming(const std::string &value) override {
        spdlog::info("onIncoming {} : {}", this->m_fullPrefix, FixClientLog::replaceControlChar(value));
    }

    void onOutgoing(const std::string &value) override {
        spdlog::info("onOutgoing {} : {}", this->m_fullPrefix, FixClientLog::replaceControlChar(value));
    }

    void onEvent(const std::string &value) override {
        spdlog::info("onEvent {} : {}", this->m_fullPrefix, FixClientLog::replaceControlChar(value));
    }

private:
    std::string generatePrefix(const FIX::SessionID &sessionID);

    void init(std::string path, const std::string &prefix);

    std::string m_path;
    std::string m_fileName;
    std::string m_fullPrefix;

    static std::string replaceControlChar(const std::string &input) {
        std::string output;
        for (char c : input) {
            if (c == '\001') {
                output += " | ";
            } else {
                output += c;
            }
        }
        return output;
    }
};

#endif
