#include "Applicationbridge.h"


void ApplicationBridge::emitMySignal(const FIX::SessionID & s){
    emit mySignal(s);
}

void ApplicationBridge::emitLogout(const FIX::SessionID & s){
    emit logout(s);
}
