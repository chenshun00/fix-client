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

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 4290 )
#endif

class FixClientLogFacotry : public FIX::LogFactory
{
public:
	FixClientLogFacotry(const FIX::SessionSettings& settings): m_settings(settings) {
		auto dict = m_settings.get();
		if (dict.has(FIX::FILE_LOG_PATH))
		{
			this->m_path = dict.getString(FIX::FILE_LOG_PATH);
		}
		else
		{
			throw FIX::ConfigError("FileLogPath");
		}

		auto logger = spdlog::rotating_logger_mt("global", this->m_path+"/Logs/RotatingFileLog.txt", 1024 * 1024 * 1024, 5);
		spdlog::set_default_logger(logger);
	};
	
	FixClientLogFacotry(const std::string& path): m_path(path) {};

	~FixClientLogFacotry() = default;
public:
	FIX::Log* create();
	FIX::Log* create(const FIX::SessionID&);
	void destroy(FIX::Log* log);

private:
	std::string m_path;
	FIX::SessionSettings m_settings;
};


class FixClientLog : public FIX::Log {

public:
	FixClientLog(const std::string& path);
	FixClientLog(const std::string& path, const FIX::SessionID& sessionID);
	~FixClientLog() = default;

	void clear(){}
	void backup(){}

	void onIncoming(const std::string& value)
	{
		spdlog::info("onIncoming {},{} message:{}", FIX::UtcTimeStampConvertor::convert(FIX::UtcTimeStamp::now(), 9), this->m_fullPrefix, value);
	}
	void onOutgoing(const std::string& value)
	{
		spdlog::info("onOutgoing {},{} message:{}", FIX::UtcTimeStampConvertor::convert(FIX::UtcTimeStamp::now(), 9), this->m_fullPrefix, value);
	}
	void onEvent(const std::string& value)
	{
		spdlog::info("onEvent {},{} message:{}", FIX::UtcTimeStampConvertor::convert(FIX::UtcTimeStamp::now(), 9), this->m_fullPrefix, value);
	}

private:
	std::string generatePrefix(const FIX::SessionID& sessionID);
	void init(std::string path, const std::string& prefix);

	std::string m_path;
	std::string m_fileName;
	std::string m_fullPrefix;
};
#endif
