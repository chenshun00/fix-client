#ifndef SESSIONHOLDER_H
#define SESSIONHOLDER_H

#include <quickfix/SessionID.h>
#include <quickfix/Session.h>
#include <map>

#include <spdlog/spdlog.h>

class SessionHolder {
public:
    static SessionHolder Instance();

    void insert(FIX::SessionID, FIX::Session *);

    bool unRegister(const FIX::SessionID &);

private:
    SessionHolder();

    typedef std::map<FIX::SessionID, FIX::Session *> SESSION_ID_MAP;
    static SESSION_ID_MAP sessionMap;
};

inline SessionHolder::SessionHolder() = default;

#endif // SESSIONHOLDER_H
