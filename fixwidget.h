#ifndef FIXWIDGET_H
#define FIXWIDGET_H

#include "quickfix/ThreadedSocketInitiator.h"
#include <quickfix/MessageStore.h>
#include "quickfix/FileStore.h"
#include "quickfix/FileLog.h"

#include "map"

#include "ClientApplication.h"

#include <QWidget>

namespace Ui {
class FixWidget;
}

class FixWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FixWidget(FIX::SessionSettings,FIX::FileStoreFactory,FIX::FileLogFactory,ClientApplication* c,QWidget *parent = nullptr);
    ~FixWidget();

public slots:
    void order();
    void receiveMySignal(const FIX::SessionID &); // 接收信号的槽

private:

    FIX::SessionSettings  sessionSettings;
    FIX::FileStoreFactory file_store_factory;
    FIX::FileLogFactory file_log;
    ClientApplication* client;
    FIX::ThreadedSocketInitiator m_initiator;
    Ui::FixWidget *ui;

    String getId();
    bool check(const QString& ,const QString&);
};

#endif // FIXWIDGET_H
