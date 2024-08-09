#include "FixClientLogFactory.h"

FIX::Log* FixClientLogFacotry::create()
{
    return nullptr;
}

FIX::Log* FixClientLogFacotry::create(const FIX::SessionID&)
{
    return nullptr;
}

void FixClientLogFacotry::destroy(FIX::Log* log)
{
}

FixClientLog::FixClientLog(const std::string& path)
{
}

FixClientLog::FixClientLog(const std::string& path, const std::string& backupPath)
{
}

FixClientLog::FixClientLog(const std::string& path, const SessionID& sessionID)
{
}

FixClientLog::FixClientLog(const std::string& path, const std::string& backupPath, const SessionID& sessionID)
{
}

FixClientLog::~FixClientLog()
{
}

std::string FixClientLog::generatePrefix(const FIX::SessionID& sessionID)
{
    return std::string();
}

void FixClientLog::init(std::string path, std::string backupPath, const std::string& prefix)
{
}
