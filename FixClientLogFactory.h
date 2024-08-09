#pragma once
#ifndef FIXCLIENTLOGFACTORY_H
#define FIXCLIENTLOGFACTORY_H

#include <quickfix/Log.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/FieldConvertors.h>

#include <spdlog/spdlog.h>

#include <string>

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 4290 )
#endif

class FixClientLogFacotry : public FIX::LogFactory
{
public:
	FixClientLogFacotry(const FIX::SessionSettings& settings): m_settings(settings), m_globalLog(nullptr), m_globalLogCount(0) {};
	
	FixClientLogFacotry(const std::string& path): m_path(path), m_backupPath(path), m_globalLog(nullptr), m_globalLogCount(0) {};

	FixClientLogFacotry(const std::string& path, const std::string& backupPath) : m_path(path), m_backupPath(backupPath), m_globalLog(nullptr), m_globalLogCount(0) {};

	~FixClientLogFacotry() = default;
public:
	FIX::Log* create();
	FIX::Log* create(const FIX::SessionID&);
	void destroy(FIX::Log* log);

private:
	std::string m_path;
	std::string m_backupPath;
	FIX::SessionSettings m_settings;
	FIX::Log* m_globalLog;
	int32_t m_globalLogCount;
};


class FixClientLog : public FIX::Log {

public:
	FixClientLog(const std::string& path);
	FixClientLog(const std::string& path, const std::string& backupPath);
	FixClientLog(const std::string& path, const FIX::SessionID& sessionID);
	FixClientLog(const std::string& path, const std::string& backupPath, const FIX::SessionID& sessionID);
	virtual ~FixClientLog();

	void clear();
	void backup();

	void onIncoming(const std::string& value)
	{
		spdlog::info("onIncoming {},{}£¬ message:{}", FIX::UtcTimeStampConvertor::convert(FIX::UtcTimeStamp::now(), 9), this->m_fullPrefix, value);
	}
	void onOutgoing(const std::string& value)
	{
		spdlog::info("onOutgoing {},{}£¬ message:{}", FIX::UtcTimeStampConvertor::convert(FIX::UtcTimeStamp::now(), 9), this->m_fullPrefix, value);
	}
	void onEvent(const std::string& value)
	{
		spdlog::info("onEvent {},{}£¬ message:{}", FIX::UtcTimeStampConvertor::convert(FIX::UtcTimeStamp::now(), 9), this->m_fullPrefix, value);
	}

private:
	std::string generatePrefix(const FIX::SessionID& sessionID);
	void init(std::string path, std::string backupPath, const std::string& prefix);

	std::string m_messagesFileName;
	std::string m_eventFileName;
	std::string m_fullPrefix;
	std::string m_fullBackupPrefix;
};
#endif