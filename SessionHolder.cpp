
#include "SessionHolder.h"

std::map<FIX::SessionID, FIX::Session *> SessionHolder::sessionMap;

SessionHolder SessionHolder::Instance() {
    static SessionHolder s;
    return s;
}

void SessionHolder::insert(FIX::SessionID sessionId, FIX::Session *session) {
    SessionHolder::sessionMap.insert(std::make_pair(sessionId, session));
}

bool SessionHolder::unRegister(const FIX::SessionID &sessionId) {
    auto item = SessionHolder::sessionMap.find(sessionId);
    if (item == SessionHolder::sessionMap.end()) {
        spdlog::info("find failure, {}", sessionId.toString());
        return false;
    }
    spdlog::info("erase success, {}", sessionId.toString());
    SessionHolder::sessionMap.erase(item);
    return true;
}
