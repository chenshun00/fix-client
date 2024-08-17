#ifndef CLIENTAPPLICATION_H
#define CLIENTAPPLICATION_H

#include <map>

#include <quickfix/Application.h>
#include <quickfix/fix42/MessageCracker.h>
#include <quickfix/FixValues.h>

#include <quickfix/fix42/Reject.h>
#include <quickfix/fix42/ExecutionReport.h>
#include <quickfix/fix42/BusinessMessageReject.h>
#include <quickfix/fix42/OrderCancelReject.h>

#include <quickfix/SessionSettings.h>

#include "Applicationbridge.h"
#include "Entrust.h"

class ClientApplication : public ApplicationBridge, public FIX::Application, public FIX42::MessageCracker {
public:
    explicit ClientApplication(FIX::SessionSettings, QObject *parent = nullptr);

    ~ClientApplication() override = default;
    // Application interface
public:
    void onCreate(const FIX::SessionID &) override;

    void onLogon(const FIX::SessionID &) override;

    void onLogout(const FIX::SessionID &) override;

    void toAdmin(FIX::Message &, const FIX::SessionID &) override;

    void toApp(FIX::Message &, const FIX::SessionID &) override;

    void fromAdmin(const FIX::Message &, const FIX::SessionID &) override;

    void fromApp(const FIX::Message &, const FIX::SessionID &) override;

    void onMessage(const FIX42::ExecutionReport &, const FIX::SessionID &) override;

    void onMessage(const FIX42::Reject &, const FIX::SessionID &) override;

    void onMessage(const FIX42::BusinessMessageReject &, const FIX::SessionID &) override;

    void onMessage(const FIX42::OrderCancelReject &, const FIX::SessionID &) override;

    //发送消息, 根据msgType发送下改撤消息
    bool send(Order &, Entrust &);

    static String trim(String &);

    static String getValue(const FIX::Message &m, int tag);

    const Entrust *getEntrust(const String &clOrdId) {
        auto item = this->entrust_map.find(clOrdId);
        if (item == this->entrust_map.end()) {
            return nullptr;
        }
        return &(item->second);
    }

    Order *getOrder(const String &orderId) {
        auto item = this->order_map.find(orderId);
        if (item == this->order_map.end()) {
            return nullptr;
        }
        return &(item->second);
    }

    void getReportList(String &orderId, std::vector<ClientExecutionReport> &res) {
        auto item = reportMap.find(orderId);
        if (item == reportMap.end()) {
            return;
        }
        auto reportList = item->second;
        res.insert(res.end(), reportList.begin(), reportList.end());
    }

protected:
    FIX::SessionSettings m_settings;

private:

    static bool isFinalState(char execType) {
        return FIX::ExecType_REJECTED == execType
               || FIX::ExecType_CANCELED == execType
               || FIX::ExecType_EXPIRED == execType
               || FIX::ExecType_DONE_FOR_DAY == execType
               || FIX::ExecType_FILL == execType;
    }

    typedef std::vector<ClientExecutionReport> ReportList;
    typedef std::map<std::string, std::map<std::string, FIX::Message>> MessageMap;
    typedef std::map<String/*clOrdId*/, Entrust> EntrustMap;
    typedef std::map<String/*orderId*/, Order> OrderMap;
    typedef std::map<String/*orderId*/, ReportList> ExecutionReportMap;

    MessageMap maps;
    EntrustMap entrust_map;
    OrderMap order_map;
    ExecutionReportMap reportMap;
};


#endif // CLIENTAPPLICATION_H
