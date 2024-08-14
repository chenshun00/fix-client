#ifndef FIXWIDGET_H
#define FIXWIDGET_H

#include "quickfix/ThreadedSocketInitiator.h"
#include <quickfix/MessageStore.h>
#include "quickfix/FileStore.h"
#include "quickfix/FileLog.h"

#include "spdlog/logger.h"

#include "map"

#include "ClientApplication.h"
#include "FixClientLogFactory.h"

#include <QWidget>

#include "memory"

namespace Ui {
    class FixWidget;
}

class FixWidget : public QWidget {
Q_OBJECT

public:
    explicit FixWidget(std::unique_ptr<FIX::SessionSettings>,
                       std::unique_ptr<FIX::FileStoreFactory>,
                       std::unique_ptr<FixClientLogFactory>,
                       std::unique_ptr<ClientApplication>,
                       QWidget *parent = nullptr);

    ~FixWidget() override;

public slots:

    void order();

    void logon(const FIX::SessionID &); // 接收信号的槽
    void logout(const FIX::SessionID &); // 接收信号的槽
    void receiveOrder(const Order &); // 接收信号的槽
    void showContextMenu(const QPoint &);

    void showReport(int row, int column);

private slots:

    void setupCustomFeatures(); // 自定义设置

private:

    std::unique_ptr<FIX::SessionSettings> m_sessionSettings;
    std::unique_ptr<FIX::FileStoreFactory> m_fileStoreFactory;
    std::unique_ptr<FixClientLogFactory> m_fixClientLogFactory;
    std::unique_ptr<ClientApplication> m_client;
    FIX::ThreadedSocketInitiator m_initiator;
    Ui::FixWidget *ui;

    static String getId();

    bool check(const QString &, const QString &);

    int currentRowIndex = -1;

    void handleAmend();

    void handleCancel();

    QString getRowOrderId(int rowIndex);

    static QString getShowValues(char ordStatus, double ordQty, double cumQty);
};

#endif // FIXWIDGET_H
