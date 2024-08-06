#include "Applicationbridge.h"


void ApplicationBridge::mySignal(const FIX::SessionID & s){

}

void ApplicationBridge::emitMySignal(const FIX::SessionID & s){
    emit mySignal(s);
}
