#ifndef CLIENTAPPLICATION_H
#define CLIENTAPPLICATION_H

#include <map>

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
    String getValue(FIX42::ExecutionReport& m, int tag);

    const Entrust* getEntrust(String cl_ord_id) 
    {
        auto item = this->entrust_map.find(cl_ord_id);
        if (item == this->entrust_map.end())
        {
            return nullptr;
        }
        return &(item->second);
    }

    const Order* getOrder(String order_id)
    {
        auto item = this->order_map.find(order_id);
        if (item == this->order_map.end())
        {
            return nullptr;
        }
        return &(item->second);
    }

private:
    typedef std::map<std::string,std::map<std::string, FIX::Message>> MessageMap;
    typedef std::map<String/*clOrdId*/, Entrust> EntrustMap;
    typedef std::map<String/*orderId*/, Order> OrderMap;
    typedef std::map<String/*orderId*/, ClientExecutionReport> ExecutionReportMap;
    MessageMap maps;
    EntrustMap entrust_map;
    OrderMap order_map;
    ExecutionReportMap reportMap;
};


#endif // CLIENTAPPLICATION_H
