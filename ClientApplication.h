#ifndef CLIENTAPPLICATION_H
#define CLIENTAPPLICATION_H

#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/FixValues.h>


#include <quickfix/fix42/Reject.h>
#include <quickfix/fix42/ExecutionReport.h>
#include <quickfix/fix42/BusinessMessageReject.h>
#include <quickfix/fix42/OrderCancelReject.h>

#include "Applicationbridge.h"
#include "Entrust.h"

class ClientApplication : public ApplicationBridge,public FIX::Application, FIX::MessageCracker
{
public:
    explicit ClientApplication(QObject *parent = nullptr);

    // Application interface
public:
    void onCreate(const FIX::SessionID &) override;
    void onLogon(const FIX::SessionID &) override;
    void onLogout(const FIX::SessionID &) override;
    void toAdmin(FIX::Message &, const FIX::SessionID &) override;
    void toApp(FIX::Message &, const FIX::SessionID &) override;
    void fromAdmin(const FIX::Message &, const FIX::SessionID &) override;
    void fromApp(const FIX::Message &, const FIX::SessionID &) override;



    // MessageCracker interface
public:
    void onMessage(FIX42::ExecutionReport &, const FIX::SessionID &) override;
    void onMessage(FIX42::Reject &, const FIX::SessionID &) override;
    void onMessage(FIX42::BusinessMessageReject &, const FIX::SessionID &) override;
    void onMessage(FIX42::OrderCancelReject &, const FIX::SessionID &) override;

    //发送消息, 根据msgType发送下改撤消息
    bool send(Order&, Entrust&);
    String trim(String&);

private:
    typedef std::map<std::string,std::map<std::string, FIX::Message>> MessageMap;
    MessageMap maps;
};


#endif // CLIENTAPPLICATION_H
