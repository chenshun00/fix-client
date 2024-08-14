#ifndef ENTRUST_H
#define ENTRUST_H

#include <string>
#include <utility>

const int FORBID_UPDATE = 0;
const int CAN_UPDATE = 1;

typedef std::string String;

struct AmendContext {
    String orderId;
    double price{};
    double ordQty{};
} Amend;

class Entrust {
public:
    String m_orderId;
    String m_clOrdId;
    String m_origClOrdId;
    String m_msgType;
    double m_orderQty;
    double m_price;

public:
    Entrust(String order_id,
            String cl_ord_id,
            String orig_cl_ord_id,
            String msg_type,
            double order_qty,
            double price) : m_orderId(std::move(order_id)), m_clOrdId(std::move(cl_ord_id)),
                            m_origClOrdId(std::move(orig_cl_ord_id)),
                            m_msgType(std::move(msg_type)), m_orderQty(order_qty), m_price(price) {};

    ~Entrust() = default;
};

class Order {
public:
    int update{0};
    String beginString;
    String sendCompId;
    String targetCompId;
    String orderId;
    String inMarket;
    String onRoad;
    String symbol;
    double ordQty;
    char side;
    char ordType;
    double price;
    String tradingSessionId;
    String exchange;
    String account;
    String securityType;
    char positionEffect;
    double cumQty = 0;
    double leavesQty = 0;
    char ordStatus;
public:
    Order(/* args */) = default;

    ~Order() = default;
};

class ClientExecutionReport {
public:
    String order_id;
    String msg_seq_num;
    String symbol;
    String side;
    double last_px{};
    double last_share{};
    double cum_qty{};
    double leaves_qty{};
    String exec_type;
    String ord_status;
    String text;
    String cl_ord_id;
    String orig_cl_ord_id;
    String exec_id;
public:
    ClientExecutionReport(/* args */) = default;

    ~ClientExecutionReport() = default;
};

#endif
