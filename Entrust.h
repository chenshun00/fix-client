#ifndef ENTRUST_H
#define ENTRUST_H

#include <string>
#include <utility>

const int FORBID_UPDATE = 0;
const int CAN_UPDATE = 1;

typedef std::string String;

class Entrust {
public:
    String m_order_id;
    String m_cl_ord_id;
    String m_orig_cl_ord_id;
    String m_msg_type;
    double m_order_qty;
    double m_price;

public:
    Entrust(String order_id,
            String cl_ord_id,
            String orig_cl_ord_id,
            String msg_type,
            double order_qty,
            double price) : m_order_id(std::move(order_id)), m_cl_ord_id(std::move(cl_ord_id)),
                            m_orig_cl_ord_id(std::move(orig_cl_ord_id)),
                            m_msg_type(std::move(msg_type)), m_order_qty(order_qty), m_price(price) {};

    ~Entrust() = default;
};

class Order {
public:
    int update{0};
    String begin_string;
    String send_comp_id;
    String target_comp_id;
    String order_id;
    String in_market;
    String on_load;
    String symbol;
    double order_qty;
    char side;
    char ord_type;
    double price;
    String trading_session_id;
    String exchange;
    String account;
    String security_type;
    char open_close;
    double cum_qty = 0;
    double leaves_qty = 0;
    String ord_status;
public:
    Order(/* args */) {}

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
