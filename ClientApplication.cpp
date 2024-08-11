#include "ClientApplication.h"
#include "SessionHolder.h"

#include <QCoreApplication>
#include <quickfix/FixValues.h>
#include <quickfix/fix42/NewOrderSingle.h>

#include <spdlog/spdlog.h>

#include <regex>


ClientApplication::ClientApplication(FIX::SessionSettings *s, QObject *parent) : ApplicationBridge(parent),
                                                                                 FIX::Application(), m_settings(s) {
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
    this->emitMySignal(sessionId);
}

void ClientApplication::onLogout(const FIX::SessionID &s) {
    spdlog::info("start ClientApplication::onLogout, sessionId:{}, emit logout signal", s.toString());
    this->logout(s);
    spdlog::info("end ClientApplication::onLogout, sessionId:{}, emit logout signal", s.toString());
}

void ClientApplication::toAdmin(FIX::Message &m, const FIX::SessionID &s) {

    spdlog::info("size = {}", this->m_settings->size());
    if (m.getHeader().isSetField(FIX::FIELD::MsgType) &&
        m.getHeader().getField(FIX::FIELD::MsgType) == FIX::MsgType_Logon && this->m_settings->has(s)) {
        auto dataDic = this->m_settings->get(s);
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
    this->crack(m, s);
}

void ClientApplication::fromApp(const FIX::Message &m, const FIX::SessionID &s) {
    this->crack(m, s);
}

void ClientApplication::onMessage(FIX42::ExecutionReport &m, const FIX::SessionID &s) {
    ClientExecutionReport report = ClientExecutionReport();

    FIX::ClOrdID cl_ord_id;
    m.getField(cl_ord_id);
    report.cl_ord_id = cl_ord_id.getFixString();


    auto entrust = this->getEntrust(report.cl_ord_id);
    if (!entrust) {
        return;
    }
    auto order = this->getOrder(entrust->m_order_id);
    if (!order) {
        return;
    }
    FIX::MsgSeqNum num;
    m.getHeader().getField(num);
    report.order_id = order->order_id;
    report.msg_seq_num = num.getValue();
    report.symbol = order->symbol;
    report.side = order->side == FIX::Side_BUY ? "Buy" : (order->side == FIX::Side_SELL) ? "Sell" : "Sell_Short";
    report.cl_ord_id = entrust->m_cl_ord_id;
    report.orig_cl_ord_id = entrust->m_orig_cl_ord_id;
    report.exec_id = this->getValue(m, FIX::FIELD::ExecID);
    report.text = this->getValue(m, FIX::FIELD::Text);
    report.exec_type = this->getValue(m, FIX::FIELD::ExecType);
    report.ord_status = this->getValue(m, FIX::FIELD::OrdStatus);
    report.last_px = std::stod(this->getValue(m, FIX::FIELD::LastPx));
    report.last_share = std::stod(this->getValue(m, FIX::FIELD::LastShares));
    report.cum_qty = std::stod(this->getValue(m, FIX::FIELD::CumQty));
    report.leaves_qty = std::stod(this->getValue(m, FIX::FIELD::LeavesQty));

    this->reportMap.insert(std::make_pair(report.order_id, report));
}


String ClientApplication::getValue(FIX42::ExecutionReport &m, int tag) {
    try {
        return m.getField(tag);
    }
    catch (FIX::FieldNotFound &f) {
        return "0";
    }
}

void ClientApplication::onMessage(FIX42::OrderCancelReject &m, const FIX::SessionID &s) {

}

void ClientApplication::onMessage(FIX42::Reject &m, const FIX::SessionID &s) {
    std::string ref = m.getFieldPtr(FIX::FIELD::RefSeqNum)->getString();
    FIX::SenderCompID sendCompId = s.getSenderCompID();

    auto item = this->maps.find(sendCompId.getString());
    if (item == this->maps.end()) {
        qInfo() << "clordid not exist and corresponding message is not exist";
        return;
    }
    auto tag_message = item->second;

    auto message_pair = tag_message.find(ref);
    if (message_pair == tag_message.end()) {
        qInfo() << "2, clordid not exist and corresponding message is not exist";
        return;
    }

    auto message = message_pair->second;

    FIX::ClOrdID clOrdId;
    message.getField(clOrdId);

    auto id = clOrdId.getString();


}

void ClientApplication::onMessage(FIX42::BusinessMessageReject &m, const FIX::SessionID &s) {

}

bool ClientApplication::send(Order &order, Entrust &entrust) {

    String msg_type = entrust.m_msg_type;


    FIX::Message message;
    message.getHeader().setField(FIX::BeginString(order.begin_string));
    message.getHeader().setField(FIX::SenderCompID(order.send_comp_id));
    message.getHeader().setField(FIX::TargetCompID(order.target_comp_id));
    message.getHeader().setField(FIX::MsgType(msg_type));

    message.setField(FIX::TransactTime(FIX::UtcTimeStamp::nowUtc(), 6));

    if (msg_type == FIX::MsgType_NewOrderSingle) {
        message.setField(FIX::HandlInst(FIX::HandlInst_AUTOMATED_EXECUTION_NO_INTERVENTION));
        message.setField(FIX::OrdType(order.ord_type));
        if (order.ord_type == FIX::OrdType_LIMIT) {
            message.setField(FIX::Price(entrust.m_price));
        }
        message.setField(FIX::OrderQty(entrust.m_order_qty));
        message.setField(FIX::SecurityType(order.security_type));
        message.setField(FIX::Symbol(this->trim(order.symbol)));
        message.setField(FIX::ClOrdID(entrust.m_cl_ord_id));
        message.setField(FIX::Side(order.side));
        if (order.exchange.length() > 0) {
            message.setField(FIX::ExDestination(order.exchange));
        }
        if (order.account.length() > 0) {
            message.setField(FIX::Account(order.account));
        }
        message.setField(FIX::PositionEffect(order.open_close));
        
        FIX42::NewOrderSingle::NoTradingSessions sessions;
        FIX::TradingSessionID session_id(order.trading_session_id);
        sessions.set(session_id);

        message.addGroup(sessions);

    }
    if (msg_type == FIX::MsgType_OrderCancelReplaceRequest) {
        if (order.account.size() > 0) {
            message.setField(FIX::Account(order.account));
        }
        message.setField(FIX::ClOrdID(entrust.m_cl_ord_id));
        message.setField(FIX::OrigClOrdID(entrust.m_orig_cl_ord_id));
        message.setField(FIX::Symbol(this->trim(order.symbol)));
        message.setField(FIX::Side(order.side));
        message.setField(FIX::OrderQty(entrust.m_order_qty));
        message.setField(FIX::OrdType(order.ord_type));
        if (order.ord_type == FIX::OrdType_LIMIT) {
            message.setField(FIX::Price(entrust.m_price));
        }
    }
    if (msg_type == FIX::MsgType_OrderCancelRequest) {
        if (order.account.size() > 0) {
            message.setField(FIX::Account(order.account));
        }
        message.setField(FIX::ClOrdID(entrust.m_cl_ord_id));
        message.setField(FIX::OrigClOrdID(entrust.m_orig_cl_ord_id));
        message.setField(FIX::Symbol(this->trim(order.symbol)));
        message.setField(FIX::Side(order.side));
        message.setField(FIX::OrderQty(entrust.m_order_qty));
    }

    bool res = FIX::Session::sendToTarget(message);
    if (res) {
        this->entrust_map.insert(std::make_pair(entrust.m_cl_ord_id, entrust));
        this->order_map.insert(std::make_pair(order.order_id, order));

        this->emitPlaceOrder(order);
    }
    return res;
}

String ClientApplication::trim(String &value) {
    auto str_pos = value.find_first_not_of("\t\v\f");
    auto end_pos = value.find_last_not_of("\t\v\f");
    return String(value, str_pos, end_pos + 1);
}
