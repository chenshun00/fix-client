#include "FixClientLogFactory.h"

FIX::Log* FixClientLogFacotry::create()
{
    return new FixClientLog(this->m_path);
}

FIX::Log* FixClientLogFacotry::create(const FIX::SessionID& sessionId)
{
    return new FixClientLog(this->m_path, sessionId);
}

void FixClientLogFacotry::destroy(FIX::Log* log)
{
    delete log;
}

FixClientLog::FixClientLog(const std::string& path):m_path(path)
{
    this->init(path, "Global");
}


FixClientLog::FixClientLog(const std::string& path, const FIX::SessionID& sessionID):m_path(path)
{
    this->init(path, sessionID.toString());
}

std::string FixClientLog::generatePrefix(const FIX::SessionID& sessionID)
{
    return sessionID.toString();
}

void FixClientLog::init(std::string path, const std::string& prefix)
{
    this->m_fullPrefix = prefix;
}
