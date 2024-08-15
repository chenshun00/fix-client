#ifndef APPLICATIONBRIDGE_H
#define APPLICATIONBRIDGE_H

#include "quickfix/SessionID.h"

#include "QObject"

#include "Entrust.h"

class ApplicationBridge : public QObject {
Q_OBJECT

public:
    explicit ApplicationBridge(QObject *parent = nullptr) : QObject(parent) {}

    ~ApplicationBridge() override = default;

signals:

    void logon(const FIX::SessionID &); // 定义一个信号，带有一个QString参数
    void logout(const FIX::SessionID &);

    void placeOrder(const Order &);

    void orderChanged(const Order &);

public slots:

    void emitLogon(const FIX::SessionID &); // 一个槽，用于发送信号
    void emitLogout(const FIX::SessionID &);

    void emitPlaceOrder(const Order &);

    void emitOrderChanged(const Order &);
};


#endif // APPLICATIONBRIDGE_H
