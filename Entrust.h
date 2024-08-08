#ifndef ENTRUST_H
#define ENTRUST_H

#include <string>

typedef std::string String;

class Entrust
{
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
            double price) : m_order_id(order_id), m_cl_ord_id(cl_ord_id), m_orig_cl_ord_id(orig_cl_ord_id),
                            m_msg_type(msg_type), m_order_qty(order_qty), m_price(price) {};
    ~Entrust() = default;
};

class Order
{
public:
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
    char trading_session_id;
    String exchange;
    String account;
    String security_type;
    char open_close;
    double cum_qty=0;
    String ord_status;
public:
    Order(/* args */){}
    ~Order() = default;

    String calculator_leaves_qty(){
        return std::to_string(order_qty - cum_qty);
    }
};

class ClientExecutionReport
{
public:
    String order_id;
    int msg_seq_num;
    String symbol;
    String side;
    double last_px;
    double last_share;
    double leaves_qty;
    double cum_qty;
    String exec_type;
    String ord_status;
    String text;
    String cl_ord_id;
    String orig_cl_ord_id;
    String exec_id;
public:
    ClientExecutionReport(/* args */){}
    ~ClientExecutionReport() = default;
};

#endif
