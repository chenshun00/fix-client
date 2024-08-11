#include "fixwidget.h"
#include "ui_fixwidget.h"
#include <QMessageBox>
#include <QUuid>
#include <utility>

#include "cancelDialog.h"

FixWidget::FixWidget(std::unique_ptr<FIX::SessionSettings> sessionSettings,
                     std::unique_ptr<FIX::FileStoreFactory> fileStoreFactory,
                     std::unique_ptr<FixClientLogFactory> fixClientLogFactory,
                     std::unique_ptr<ClientApplication> client,
                     QWidget *parent)
        : QWidget(parent),
          m_sessionSettings(std::move(sessionSettings)),
          m_fileStoreFactory(std::move(fileStoreFactory)),
          m_fixClientLogFactory(std::move(fixClientLogFactory)),
          m_client(std::move(client)),
          m_initiator(*m_client, *m_fileStoreFactory, *m_sessionSettings, *m_fixClientLogFactory),
          ui(new Ui::FixWidget) {
    ui->setupUi(this);
    //自适应大小
    ui->OrderTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->OrderTable->horizontalHeader()->setStretchLastSection(true);
    ui->OrderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //禁止编辑
    ui->OrderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //自适应大小
    ui->Report->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->Report->horizontalHeader()->setStretchLastSection(true);
    ui->Report->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //禁止编辑

    this->setupCustomFeatures(); // 自定义设置
    m_initiator.start();

    connect(ui->Order, SIGNAL(clicked()), this, SLOT(order()));
    connect(m_client.get(), &ApplicationBridge::mySignal, this, &FixWidget::logon);
    connect(m_client.get(), &ApplicationBridge::placeOrder, this, &FixWidget::receiveOrder);
}

void FixWidget::setupCustomFeatures() {
    connect(ui->OrderTable, &QTableWidget::customContextMenuRequested, this, &FixWidget::showContextMenu);
    connect(ui->OrderTable, &QTableWidget::cellDoubleClicked, this, &FixWidget::showReport);
}

void FixWidget::showReport(int row, int column) {
    auto qsOrderId = this->getRowOrderId(row);
    String orderId = qsOrderId.toStdString();
    ui->Report->clearContents();

    int rowCount = ui->Report->rowCount();
    for (int i = 0; i < rowCount; i++) {
        ui->Report->removeRow(i);
    }

    if (orderId.empty()) {
        return;
    }

    std::vector<ClientExecutionReport> reports;
    m_client->getReportList(orderId, reports);

    int rowPosition = 0;
    for (const auto &item : reports) {
        ui->Report->insertRow(rowPosition);

        ui->Report->setItem(rowPosition, 0, new QTableWidgetItem(QString::fromStdString(item.msg_seq_num)));
        ui->Report->setItem(rowPosition, 1, new QTableWidgetItem(QString::fromStdString(item.cl_ord_id)));
        ui->Report->setItem(rowPosition, 2, new QTableWidgetItem(QString::fromStdString(item.orig_cl_ord_id)));
        ui->Report->setItem(rowPosition, 3, new QTableWidgetItem(QString::fromStdString(item.symbol)));
        ui->Report->setItem(rowPosition, 4, new QTableWidgetItem(QString::fromStdString(item.side)));
        ui->Report->setItem(rowPosition, 5, new QTableWidgetItem(QString::number(item.last_px)));
        ui->Report->setItem(rowPosition, 6, new QTableWidgetItem(QString::number(item.last_share)));
        ui->Report->setItem(rowPosition, 7, new QTableWidgetItem(QString::fromStdString(item.exec_type)));
        ui->Report->setItem(rowPosition, 8, new QTableWidgetItem(QString::fromStdString(item.ord_status)));
        ui->Report->setItem(rowPosition, 9, new QTableWidgetItem(QString::number(item.cum_qty)));
        ui->Report->setItem(rowPosition, 10, new QTableWidgetItem(QString::number(item.leaves_qty)));
        ui->Report->setItem(rowPosition, 11, new QTableWidgetItem(QString::fromStdString(item.text)));
        ui->Report->setItem(rowPosition, 12, new QTableWidgetItem(QString::fromStdString(item.exec_id)));

        rowPosition++;
    }
}

void FixWidget::showContextMenu(const QPoint &pos) {
    QMenu menu(this);
    QAction *amend = menu.addAction("改单");
    QAction *cancel = menu.addAction("撤单");
    connect(amend, &QAction::triggered, this, &FixWidget::handleAmend);
    connect(cancel, &QAction::triggered, this, &FixWidget::handleCancel);
    menu.exec(ui->OrderTable->mapToGlobal(pos));

    // 获取鼠标点击位置的行索引
    QModelIndex index = ui->OrderTable->indexAt(pos);
    if (index.isValid()) {
        currentRowIndex = index.row();
    }
    menu.exec(ui->OrderTable->mapToGlobal(pos));
}

QString FixWidget::getRowOrderId(int rowIndex) {
    auto item = ui->OrderTable->item(rowIndex, 0);
    if (item) {
        return item->text();
    }
    return "";
}

void FixWidget::handleAmend() {
    auto orderId = this->getRowOrderId(this->currentRowIndex);

}


void FixWidget::handleCancel() {
    auto orderId = this->getRowOrderId(this->currentRowIndex);
    CancelDialog dialog(this); // 将this作为父窗口
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        // 用户点击了确认按钮
        // 这里可以获取对话框中的数据并进行进一步处理
        // 如：QString data = dialog.getData();
        qDebug() << "hh";
    } else if (result == QDialog::Rejected) {
        // 用户点击了取消按钮或其他方式关闭了对话框
        qDebug() << "ww";
    }
}


void FixWidget::logon(const FIX::SessionID &s) {
    spdlog::info("FixWidget::logon, {}", s.toString());
    auto str = s.toString();
    if (ui->Session_ID->findText(QString::fromStdString(str)) >= 0) {
        return;
    }
    ui->Session_ID->addItem(QString::fromStdString(s.toString()));
}

void FixWidget::logout(const FIX::SessionID &s) {
    spdlog::info("FixWidget::logout, {}", s.toString());
    auto str = s.toString();
    if (ui->Session_ID->findText(QString::fromStdString(str)) >= 0) {
        ui->Session_ID->removeItem(ui->Session_ID->findText(QString::fromStdString(str)));
    }
}

void FixWidget::receiveOrder(const Order &order) {
    int rowPosition = 0;
    ui->OrderTable->insertRow(rowPosition);
    ui->OrderTable->setItem(rowPosition, 0, new QTableWidgetItem(QString::fromStdString(order.order_id)));
    ui->OrderTable->setItem(rowPosition, 1, new QTableWidgetItem(QString::fromStdString(order.symbol)));
    ui->OrderTable->setItem(rowPosition, 2, new QTableWidgetItem(QString(QChar(order.ord_type))));
    ui->OrderTable->setItem(rowPosition, 3, new QTableWidgetItem(QString::number(order.price)));
    ui->OrderTable->setItem(rowPosition, 4, new QTableWidgetItem(QString::fromStdString(order.ord_status)));
    ui->OrderTable->setItem(rowPosition, 5, new QTableWidgetItem(QString(QChar(order.side))));
    ui->OrderTable->setItem(rowPosition, 6, new QTableWidgetItem(QString::number(order.order_qty)));
    ui->OrderTable->setItem(rowPosition, 7, new QTableWidgetItem(QString::number(order.cum_qty)));
    ui->OrderTable->setItem(rowPosition, 8, new QTableWidgetItem(QString::number(order.leaves_qty)));

    //ui->OrderTable->currentItem()
}

void FixWidget::order() {
    const QString symbol = ui->Symbol->text();
    const QString price = ui->Price->text();
    const QString type = ui->Security_Type->currentText();
    const QString ord_Type = ui->Ord_Type->currentText();
    const QString exchange = ui->Exchange->currentText();
    const QString quantity = ui->Quantity->text();
    const QString account = ui->Account->text();

    const QString session_id = ui->Session_ID->currentText();
    if (!this->check(session_id, "sessionId必填")) {
        return;
    }
    //session反解析为sessionId
    FIX::SessionID fix_session_id;
    fix_session_id.fromString(session_id.toStdString());


    const QString side = ui->Side->currentText();
    const QString position_effect = ui->Position_Effect->currentText();


    if (!this->check(quantity, "委托数量必填")) {
        return;
    }
    if (!this->check(symbol, "symbol必填")) {
        return;
    }
    if (ord_Type == "Limit" && !this->check(price, "price不能为空")) {
        return;
    }

    Order order;
    order.send_comp_id = fix_session_id.getSenderCompID().getString();
    order.target_comp_id = fix_session_id.getTargetCompID().getString();
    order.begin_string = fix_session_id.getBeginString().getString();

    if (!exchange.isEmpty()) {
        order.exchange = exchange.toStdString();
    }
    if (!account.isEmpty()) {
        order.account = account.toStdString();
    }
    order.side = side == "Buy" ? FIX::Side_BUY : side == "Sell" ? FIX::Side_SELL : FIX::Side_SELL_SHORT;
    order.open_close = position_effect == "Open" ? FIX::PositionEffect_OPEN : FIX::PositionEffect_CLOSE;

    order.order_qty = quantity.toDouble();
    order.security_type = type.toStdString();
    order.ord_type = ord_Type == "Limit" ? FIX::OrdType_LIMIT : FIX::OrdType_MARKET;
    if (order.ord_type == FIX::OrdType_LIMIT) {
        order.price = price.toDouble();
    } else {
        order.price = 0;
    }
    order.symbol = symbol.toStdString();
    order.order_id = FixWidget::getId();

    Entrust entrust(order.order_id, order.order_id, "", "D", order.order_qty, order.price);
    order.in_market = entrust.m_cl_ord_id;
    order.on_load = "";
    order.cum_qty = 0;
    order.leaves_qty = order.order_qty;

    auto pbtn = qobject_cast<QRadioButton *>(ui->Trading_Session->checkedButton());
    QString name = pbtn->objectName();

    if (QString::compare(name, "Trading_Session_0") == 0) {
        order.trading_session_id = "0";
    } else if (QString::compare(name, "Trading_Session_1") == 0) {
        order.trading_session_id = "1";
    } else {
        QMessageBox::warning(this, "盘前盘后参数错误", "参数错误");
        return;
    }
    this->m_client->send(order, entrust);
}

FixWidget::~FixWidget() {
    m_initiator.stop();
    delete ui;
}

String FixWidget::getId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).replace("-", "").toStdString();
}

bool FixWidget::check(const QString &field, const QString &msg) {
    if (field.isEmpty()) {
        QMessageBox::warning(this, "参数错误", msg);
        return false;
    }
    return true;
}

