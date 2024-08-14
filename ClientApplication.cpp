#include "ClientApplication.h"
#include "SessionHolder.h"

#include <QCoreApplication>
#include <utility>
#include <quickfix/FixValues.h>
#include <quickfix/fix42/NewOrderSingle.h>

#include <spdlog/spdlog.h>

ClientApplication::ClientApplication(FIX::SessionSettings s, QObject *parent) : ApplicationBridge(parent),
                                                                                FIX::Application(),
                                                                                FIX42::MessageCracker(),
                                                                                m_settings(std::move(s)) {
    spdlog::info("ClientApplication instantiate");
}

void ClientApplication::onCreate(const FIX::SessionID &sessionId) {
    spdlog::info("ClientApplication::onCreate, sessionId: {}", sessionId.toString());
}

void ClientApplication::onLogon(const FIX::SessionID &sessionId) {
    FIX::Session *session = FIX::Session::lookupSession(sessionId);
    if (!session) {
        spdlog::critical("session is not exist, sessionId:{}", sessionId.toString());
        return;
    }
    SessionHolder::Instance().insert(sessionId, session);
    spdlog::info("emitSignal, sessionId:{}", sessionId.toString());
    this->emitLogon(sessionId);
}

void ClientApplication::onLogout(const FIX::SessionID &s) {
    spdlog::info("start ClientApplication::onLogout, sessionId:{}, emit logout signal", s.toString());
    SessionHolder::Instance().unRegister(s);
    this->logout(s);
    spdlog::info("end ClientApplication::onLogout, sessionId:{}, emit logout signal", s.toString());
}

void ClientApplication::toAdmin(FIX::Message &m, const FIX::SessionID &s) {

    spdlog::info("size = {}", this->m_settings.size());
    if (m.getHeader().isSetField(FIX::FIELD::MsgType) &&
        m.getHeader().getField(FIX::FIELD::MsgType) == FIX::MsgType_Logon && this->m_settings.has(s)) {
        auto dataDic = this->m_settings.get(s);
        if (dataDic.has("RawData")) {
            String rawData = dataDic.getString("RawData");
            m.setField(FIX::RawData(rawData));
            m.setField(FIX::RawDataLength(rawData.size()));
        }
        if (dataDic.has("Password")) {
            String password = dataDic.getString("Password");
            m.setField(FIX::Password(password));
        }

        if (dataDic.has("ResetSeqNumFlag") && dataDic.getString("ResetSeqNumFlag") == "Y") {}
        {
            auto session = FIX::Session::lookupSession(s);
            auto store = session->getStore();
            if (store) {
                spdlog::info("nextTargetSeqNum:{}, nextSenderSeqNum:{}", store->getNextTargetMsgSeqNum(),
                             store->getNextSenderMsgSeqNum());
                if (store->getNextSenderMsgSeqNum() == 1 && store->getNextTargetMsgSeqNum() == 1) {
                    m.setField(FIX::ResetSeqNumFlag(true));
                }
            }
        }
    }
}

void ClientApplication::toApp(FIX::Message &m, const FIX::SessionID &s) {
    auto seq = m.getHeader().getFieldPtr(FIX::FIELD::MsgSeqNum);
    auto tag = seq->getString();


    const FIX::SenderCompID &sendCompId = s.getSenderCompID();

    auto tag_pair = std::make_pair(tag, m);

    auto item = this->maps.find(sendCompId.getString());
    if (item == this->maps.end()) {


        auto tag_map = std::map<std::string, FIX::Message>();

        tag_map.insert(tag_pair);

        auto pair = std::make_pair(sendCompId.getString(), tag_map);
        this->maps.insert(pair);
    } else {
        auto second = item->second;
        second.insert(tag_pair);
    }
}

void ClientApplication::fromAdmin(const FIX::Message &m, const FIX::SessionID &s) {
    crack(m, s);
}

void ClientApplication::fromApp(const FIX::Message &m, const FIX::SessionID &s) {
    crack(m, s);
}

bool ClientApplication::send(Order &order, Entrust &entrust) {

    String msg_type = entrust.m_msgType;

    FIX::Message message;
    message.getHeader().setField(FIX::BeginString(order.beginString));
    message.getHeader().setField(FIX::SenderCompID(order.sendCompId));
    message.getHeader().setField(FIX::TargetCompID(order.targetCompId));
    message.getHeader().setField(FIX::MsgType(msg_type));

    message.setField(FIX::TransactTime(FIX::UtcTimeStamp::nowUtc(), 6));

    if (msg_type == FIX::MsgType_NewOrderSingle) {
        order.ordStatus = FIX::OrdStatus_PENDING_NEW;
        message.setField(FIX::HandlInst(FIX::HandlInst_AUTOMATED_EXECUTION_NO_INTERVENTION));
        message.setField(FIX::OrdType(order.ordType));
        if (order.ordType == FIX::OrdType_LIMIT) {
            message.setField(FIX::Price(entrust.m_price));
        }
        message.setField(FIX::OrderQty(entrust.m_orderQty));
        message.setField(FIX::SecurityType(order.securityType));
        message.setField(FIX::Symbol(ClientApplication::trim(order.symbol)));
        message.setField(FIX::ClOrdID(entrust.m_clOrdId));
        message.setField(FIX::Side(order.side));
        if (order.exchange.length() > 0) {
            message.setField(FIX::ExDestination(order.exchange));
        }
        if (order.account.length() > 0) {
            message.setField(FIX::Account(order.account));
        }
        message.setField(FIX::PositionEffect(order.positionEffect));

        FIX42::NewOrderSingle::NoTradingSessions sessions;
        FIX::TradingSessionID session_id(order.tradingSessionId);
        sessions.set(session_id);

        message.addGroup(sessions);

    }
    if (msg_type == FIX::MsgType_OrderCancelReplaceRequest) {
        order.ordStatus = FIX::OrdStatus_PENDING_REPLACE;
        if (!order.account.empty()) {
            message.setField(FIX::Account(order.account));
        }
        message.setField(FIX::ClOrdID(entrust.m_clOrdId));
        message.setField(FIX::OrigClOrdID(entrust.m_origClOrdId));
        message.setField(FIX::Symbol(ClientApplication::trim(order.symbol)));
        message.setField(FIX::Side(order.side));
        message.setField(FIX::OrderQty(entrust.m_orderQty));
        message.setField(FIX::OrdType(order.ordType));
        if (order.ordType == FIX::OrdType_LIMIT) {
            message.setField(FIX::Price(entrust.m_price));
        }
    }
    if (msg_type == FIX::MsgType_OrderCancelRequest) {
        order.ordStatus = FIX::OrdStatus_PENDING_CANCEL;
        if (!order.account.empty()) {
            message.setField(FIX::Account(order.account));
        }
        message.setField(FIX::ClOrdID(entrust.m_clOrdId));
        message.setField(FIX::OrigClOrdID(entrust.m_origClOrdId));
        message.setField(FIX::Symbol(ClientApplication::trim(order.symbol)));
        message.setField(FIX::Side(order.side));
        message.setField(FIX::OrderQty(entrust.m_orderQty));
    }

    bool res = FIX::Session::sendToTarget(message);
    if (res) {
        order.update = FORBID_UPDATE;
        this->entrust_map.insert(std::make_pair(entrust.m_clOrdId, entrust));
        this->order_map.insert(std::make_pair(order.orderId, order));

        this->emitPlaceOrder(order);
    }
    return res;
}

void ClientApplication::onMessage(const FIX42::ExecutionReport &m, const FIX::SessionID &s) {
    ClientExecutionReport report = ClientExecutionReport();

    report.cl_ord_id = ClientApplication::getValue(m, FIX::FIELD::ClOrdID);

    auto entrust = this->getEntrust(report.cl_ord_id);
    if (!entrust) {
        return;
    }
    auto order = this->getOrder(entrust->m_orderId);
    if (!order) {
        return;
    }
    report.order_id = order->orderId;
    report.msg_seq_num = m.getHeader().getField(FIX::FIELD::MsgSeqNum);
    report.symbol = order->symbol;
    report.side = order->side == FIX::Side_BUY ? "Buy" : (order->side == FIX::Side_SELL) ? "Sell" : "Sell_Short";
    report.cl_ord_id = entrust->m_clOrdId;
    report.orig_cl_ord_id = entrust->m_origClOrdId;
    report.exec_id = ClientApplication::getValue(m, FIX::FIELD::ExecID);
    report.text = ClientApplication::getValue(m, FIX::FIELD::Text);
    report.exec_type = ClientApplication::getValue(m, FIX::FIELD::ExecType);
    report.ord_status = ClientApplication::getValue(m, FIX::FIELD::OrdStatus);
    report.last_px = std::stod(ClientApplication::getValue(m, FIX::FIELD::LastPx));
    report.last_share = std::stod(ClientApplication::getValue(m, FIX::FIELD::LastShares));
    report.cum_qty = std::stod(ClientApplication::getValue(m, FIX::FIELD::CumQty));
    report.leaves_qty = std::stod(ClientApplication::getValue(m, FIX::FIELD::LeavesQty));



    //订单进入终止状态
    if (ClientApplication::isFinalState(report.exec_type[0])) {
        order->update = FORBID_UPDATE;
    } else {
        order->update = CAN_UPDATE;
    }
    order->ordStatus = report.ord_status[0];
    if (FIX::ExecType_NEW == report.exec_type[0]) {
        order->inMarket = order->onRoad;
        order->onRoad = "";
    }
    //改单确认
    if (FIX::ExecType_REPLACED == report.exec_type[0]) {
        order->inMarket = order->onRoad;
        order->onRoad = "";
    }
    auto item = this->reportMap.find(report.order_id);
    if (item == this->reportMap.end()) {
        ReportList list;
        list.push_back(report);
        this->reportMap.insert(std::make_pair(report.order_id, list));
    } else {
        item->second.push_back(report);
    }
}

void ClientApplication::onMessage(const FIX42::OrderCancelReject &m, const FIX::SessionID &s) {
    auto report = ClientExecutionReport();
    //改单和撤单拒绝
    report.cl_ord_id = ClientApplication::getValue(m, FIX::FIELD::ClOrdID);

    auto entrust = this->getEntrust(report.cl_ord_id);
    auto order = this->getOrder(entrust->m_orderId);

    order->update = CAN_UPDATE;
    order->onRoad = "";

    report.order_id = order->orderId;
    report.msg_seq_num = m.getHeader().getField(FIX::FIELD::MsgSeqNum);
    report.text = ClientApplication::getValue(m, FIX::FIELD::Text);

    report.symbol = order->symbol;
    report.side = order->side == FIX::Side_BUY ? "Buy" : (order->side == FIX::Side_SELL) ? "Sell" : "Sell_Short";
    report.cl_ord_id = entrust->m_clOrdId;
    report.orig_cl_ord_id = entrust->m_origClOrdId;

    auto item = this->reportMap.find(report.order_id);
    if (item == this->reportMap.end()) {
        ReportList list;
        list.push_back(report);
        this->reportMap.insert(std::make_pair(report.order_id, list));
    } else {
        item->second.push_back(report);
    }
}

void ClientApplication::onMessage(const FIX42::Reject &m, const FIX::SessionID &s) {
    std::string ref = m.getFieldPtr(FIX::FIELD::RefSeqNum)->getString();
    const FIX::SenderCompID &sendCompId = s.getSenderCompID();

    auto item = this->maps.find(sendCompId.getString());
    if (item == this->maps.end()) {
        spdlog::info("1:clordid not exist and corresponding message is not exist");
        return;
    }
    auto tag_message = item->second;

    auto message_pair = tag_message.find(ref);
    if (message_pair == tag_message.end()) {
        spdlog::info("2:clordid not exist and corresponding message is not exist");
        return;
    }

    auto message = message_pair->second;

    String clOrdId = ClientApplication::getValue(message, FIX::FIELD::ClOrdID);
    auto entrust = this->getEntrust(clOrdId);
    auto order = this->getOrder(entrust->m_orderId);

    order->update = FORBID_UPDATE;
    order->ordStatus = FIX::OrdStatus_REJECTED;
    auto report = ClientExecutionReport();
    //report.orderId =
    report.order_id = entrust->m_orderId;
    report.cl_ord_id = entrust->m_clOrdId;
    report.text = ClientApplication::getValue(m, FIX::FIELD::Text);
    report.msg_seq_num = m.getHeader().getField(FIX::FIELD::MsgSeqNum);
    report.symbol = order->symbol;
    report.side = order->side == FIX::Side_BUY ? "Buy" : (order->side == FIX::Side_SELL) ? "Sell" : "Sell_Short";
    report.ord_status = "8";

    auto reportItem = this->reportMap.find(report.order_id);
    if (reportItem == this->reportMap.end()) {
        ReportList list;
        list.push_back(report);
        this->reportMap.insert(std::make_pair(report.order_id, list));
    } else {
        reportItem->second.push_back(report);
    }
}

void ClientApplication::onMessage(const FIX42::BusinessMessageReject &m, const FIX::SessionID &s) {
    std::string ref = m.getFieldPtr(FIX::FIELD::RefSeqNum)->getString();
    const FIX::SenderCompID &sendCompId = s.getSenderCompID();

    auto item = this->maps.find(sendCompId.getString());
    if (item == this->maps.end()) {
        spdlog::info("BusinessMessageReject, 1:clordid not exist and corresponding message is not exist");
        return;
    }
    auto tag_message = item->second;

    auto message_pair = tag_message.find(ref);
    if (message_pair == tag_message.end()) {
        spdlog::info("BusinessMessageReject, 2:clordid not exist and corresponding message is not exist");
        return;
    }

    auto message = message_pair->second;

    String clOrdId = ClientApplication::getValue(message, FIX::FIELD::ClOrdID);
    auto entrust = this->getEntrust(clOrdId);
    auto order = this->getOrder(entrust->m_orderId);
    if (entrust->m_msgType == FIX::MsgType_NewOrderSingle) {
        order->update = FORBID_UPDATE;
        order->ordStatus = FIX::OrdStatus_REJECTED;
    } else {
        order->update = CAN_UPDATE;
    }

    auto report = ClientExecutionReport();
    //report.orderId =
    report.order_id = entrust->m_orderId;
    report.cl_ord_id = entrust->m_clOrdId;
    report.text = ClientApplication::getValue(m, FIX::FIELD::Text);
    report.msg_seq_num = m.getHeader().getField(FIX::FIELD::MsgSeqNum);
    report.symbol = order->symbol;
    report.side = order->side == FIX::Side_BUY ? "Buy" : (order->side == FIX::Side_SELL) ? "Sell" : "Sell_Short";
    report.ord_status = "8";

    auto reportItem = this->reportMap.find(report.order_id);
    if (reportItem == this->reportMap.end()) {
        ReportList list;
        list.push_back(report);
        this->reportMap.insert(std::make_pair(report.order_id, list));
    } else {
        reportItem->second.push_back(report);
    }
}

String ClientApplication::getValue(const FIX::Message &m, int tag) {
    try {
        return m.getField(tag);
    }
    catch (FIX::FieldNotFound &f) {
        return "0";
    }
}

String ClientApplication::trim(String &value) {
    auto str_pos = value.find_first_not_of("\t\v\f");
    auto end_pos = value.find_last_not_of("\t\v\f");
    return String(value, str_pos, end_pos + 1);
}
