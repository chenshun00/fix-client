
#include "SessionHolder.h"

std::map<FIX::SessionID, FIX::Session*> SessionHolder::sessionMap;

SessionHolder SessionHolder::Instance()
{
    static SessionHolder s;
    return s;
}

void SessionHolder::insert(FIX::SessionID sessionId, FIX::Session* session)
{
    SessionHolder::sessionMap.insert(std::make_pair(sessionId, session));
}

bool SessionHolder::un_register(FIX::SessionID sessionId)
{
    auto item = SessionHolder::sessionMap.find(sessionId);
    if (item == SessionHolder::sessionMap.end()){
        return false;
    }
    SessionHolder::sessionMap.erase(item);
    return true;
}
