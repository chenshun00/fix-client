#include "Applicationbridge.h"


void ApplicationBridge::emitLogon(const FIX::SessionID &s) {
    emit logon(s);
}

void ApplicationBridge::emitLogout(const FIX::SessionID &s) {
    emit logout(s);
}

void ApplicationBridge::emitPlaceOrder(const Order &order) {
    emit placeOrder(order);
}