#include "fixwidget.h"
#include "ui_fixwidget.h"
#include <QMessageBox>
#include <QUuid>
#include <utility>

#include <QMenu>

#include "CancelDialog.h"
#include "AmendDialog.h"

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

    this->init();

    //自适应大小
    ui->Report->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->Report->horizontalHeader()->setStretchLastSection(true);
    ui->Report->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Report->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //禁止编辑

    this->setupCustomFeatures(); // 自定义设置
    m_initiator.start();

    connect(ui->Order, SIGNAL(clicked()), this, SLOT(order()));
    connect(m_client.get(), &ApplicationBridge::logon, this, &FixWidget::logon);
    connect(m_client.get(), &ApplicationBridge::logout, this, &FixWidget::logout);
    connect(m_client.get(), &ApplicationBridge::placeOrder, this, &FixWidget::receiveOrder);
    connect(m_client.get(), &ApplicationBridge::orderChanged, this, &FixWidget::orderChanged);
}

void FixWidget::setupCustomFeatures() {
    connect(ui->OrderTable, &QTableWidget::customContextMenuRequested, this, &FixWidget::showContextMenu);
    connect(ui->OrderTable, &QTableWidget::cellDoubleClicked, this, &FixWidget::showReport);
}

void FixWidget::init() {
    // 创建随机数引擎
    std::random_device rd;  // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用Mersenne Twister算法生成随机数
    std::uniform_int_distribution<> dis(10000, 99999); // 分布范围从10000到99999

    // 生成一个随机的5位数
    int startNumber = dis(gen);
    this->id = startNumber;
}

void FixWidget::showReport(int row, int column) {
    auto qsOrderId = this->getRowOrderId(row);
    String orderId = qsOrderId.toStdString();
    if (orderId.empty()) {
        spdlog::error("orderId is empty, {}", row);
        ui->Report->clearContents();
        ui->Report->setRowCount(0);
        return;
    }

    std::vector<ClientExecutionReport> reports;
    m_client->getReportList(orderId, reports);

    ui->Report->clearContents();
    auto rowCount = ui->Report->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        ui->Report->removeRow(i);
    }

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
    // 获取鼠标点击位置的行索引
    QModelIndex index = ui->OrderTable->indexAt(pos);
    if (index.isValid()) {
        currentRowIndex = index.row();
        spdlog::info("click row :{}", currentRowIndex);
    }
    QMenu menu(this);
    QAction *amend = menu.addAction("改单");
    QAction *cancel = menu.addAction("撤单");
    connect(amend, &QAction::triggered, this, &FixWidget::handleAmend);
    connect(cancel, &QAction::triggered, this, &FixWidget::handleCancel);
    menu.exec(ui->OrderTable->mapToGlobal(pos));
}

QString FixWidget::getRowOrderId(int rowIndex) {
    auto item = ui->OrderTable->item(rowIndex, 0);
    if (item) {
        return item->text();
    }
    return "";
}

QTableWidgetItem *FixWidget::getShowValues(char ordStatus, double ordQty, double cumQty, QTableWidgetItem *item) {
    if (item == nullptr) {
        item = new QTableWidgetItem();
    }

    switch (ordStatus) {
        case FIX::OrdStatus_PENDING_NEW:
            item->setText("待报");
            item->setBackground(Qt::darkYellow);
            return item;
        case FIX::OrdStatus_PENDING_CANCEL:
            item->setBackground(Qt::darkYellow);
            item->setText("待撤");
            return item;
        case FIX::OrdStatus_PENDING_REPLACE:
            item->setBackground(Qt::darkYellow);
            item->setText("待改");
            return item;
        case FIX::OrdStatus_NEW:
            item->setText("已报");
            item->setBackground(Qt::cyan);
            return item;
        case FIX::OrdStatus_PARTIALLY_FILLED:
            item->setText("部成");
            item->setBackground(Qt::green);
            return item;
        case FIX::OrdStatus_FILLED:
            item->setBackground(Qt::green);
            item->setText("全成");
            return item;
        case FIX::OrdStatus_CANCELED:
            item->setBackground(Qt::lightGray);
            item->setText("已撤");
            return item;
        case FIX::OrdStatus_EXPIRED:
            item->setBackground(Qt::lightGray);
            item->setText("过期");
            return item;
        case FIX::OrdStatus_REPLACED:
            if (cumQty == 0) {
                item->setBackground(Qt::cyan);
                item->setText("已报");
                return item;
            }
            item->setBackground(Qt::green);
            item->setText("部成");
            return item;
        case FIX::OrdStatus_REJECTED:
            item->setBackground(Qt::red);
            item->setText("拒绝");
            return item;
        default:
            item->setBackground(Qt::gray);
            item->setText("状态未知");
            return item;
    }
}

void FixWidget::handleAmend() {
    auto orderId = this->getRowOrderId(this->currentRowIndex).toStdString();
    auto order = this->m_client->getOrder(orderId);
    if (!order) {
        spdlog::info("this->currentRowIndex:{}, orderId: {}", this->currentRowIndex, orderId);
        QMessageBox::warning(this, "Note", "订单不存在");
        return;
    }
    if (!order->update) {
        QMessageBox::warning(this, "Note", "当前状态不能改单");
        return;
    }
    AmendContext amend{order->orderId, order->price, order->ordQty};
    AmendDialog dialog(amend, this);
    int result = dialog.exec();
    if (result == QDialog::Accepted) {

        //点击了确认按钮
        Entrust entrust(orderId, this->getStartOrderId(),
                        order->inMarket, FIX::MsgType_OrderCancelReplaceRequest,
                        order->ordQty - order->cumQty, order->price);
        auto res = this->m_client->send(*order, entrust);
        if (!res) {
            QMessageBox::warning(this, "ERROR", "撤单失败");
        }
    } else if (result == QDialog::Rejected) {
        // 点击了取消按钮或其他方式关闭了对话框
    }
}


void FixWidget::handleCancel() {
    auto orderId = this->getRowOrderId(this->currentRowIndex).toStdString();
    auto order = this->m_client->getOrder(orderId);
    if (!order) {
        spdlog::info("orderId: {}", orderId);
        QMessageBox::warning(this, "Note", "订单不存在");
        return;
    }
    if (!order->update) {
        QMessageBox::warning(this, "Note", "当前状态不能撤单");
        return;
    }
    CancelDialog dialog(this); // 将this作为父窗口
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        //点击了确认按钮
        Entrust entrust(orderId, this->getStartOrderId(),
                        order->inMarket, FIX::MsgType_OrderCancelRequest,
                        order->ordQty - order->cumQty, order->price);
        auto res = this->m_client->send(*order, entrust);
        if (!res) {
            QMessageBox::warning(this, "ERROR", "撤单失败");
        }
    } else if (result == QDialog::Rejected) {
        // 点击了取消按钮或其他方式关闭了对话框
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
        spdlog::info("FixWidget::logout success, {}", s.toString());
        ui->Session_ID->removeItem(ui->Session_ID->findText(QString::fromStdString(str)));
    }
}

void FixWidget::receiveOrder(const Order &order) {
    spdlog::info("FixWidget::receiveOrder, order:{}", order.orderId);
    int rowPosition = 0;
    ui->OrderTable->insertRow(rowPosition);
    ui->OrderTable->setItem(rowPosition, 0, new QTableWidgetItem(QString::fromStdString(order.orderId)));
    ui->OrderTable->setItem(rowPosition, 1, new QTableWidgetItem(QString::fromStdString(order.symbol)));
    ui->OrderTable->setItem(rowPosition, 2, new QTableWidgetItem(QString(QChar(order.ordType))));
    ui->OrderTable->setItem(rowPosition, 3, new QTableWidgetItem(QString::number(order.price)));
    ui->OrderTable->setItem(rowPosition, 4, FixWidget::getShowValues(order.ordStatus, order.ordQty, order.cumQty));

    if (order.side == FIX::Side_BUY) {
        if (order.positionEffect == FIX::PositionEffect_OPEN) {
            ui->OrderTable->setItem(rowPosition, 5, new QTableWidgetItem("买入开仓"));
        } else {
            ui->OrderTable->setItem(rowPosition, 5, new QTableWidgetItem("买入平仓"));
        }
    } else if (order.side == FIX::Side_SELL) {
        if (order.positionEffect == FIX::PositionEffect_OPEN) {
            ui->OrderTable->setItem(rowPosition, 5, new QTableWidgetItem("卖出开仓"));
        } else {
            ui->OrderTable->setItem(rowPosition, 5, new QTableWidgetItem("卖出平仓"));
        }
    } else if (order.side == FIX::Side_SELL_SHORT) {
        ui->OrderTable->setItem(rowPosition, 5, new QTableWidgetItem("沽空"));
    }
    ui->OrderTable->setItem(rowPosition, 6, new QTableWidgetItem(QString::number(order.ordQty)));
    ui->OrderTable->setItem(rowPosition, 7, new QTableWidgetItem(QString::number(order.cumQty)));
    ui->OrderTable->setItem(rowPosition, 8, new QTableWidgetItem(QString::number(order.leavesQty)));
    spdlog::info("FixWidget::receiveOrder end, order:{}", order.orderId);
}

void FixWidget::orderChanged(const Order &order) {
    spdlog::info("receive orderChanged :{}, ordStatus:{}", order.orderId, order.ordStatus);
    this->updateOrderTable();
}

void FixWidget::updateOrderTable() {
    auto rowCount = ui->OrderTable->rowCount();
    if (rowCount <= 0) {
        spdlog::info("no orders");
        return;
    }
    for (int i = 0; i < rowCount; ++i) {
        auto orderId = this->getRowOrderId(i).toStdString();
        auto order = m_client->getOrder(orderId);
        if (order == nullptr) {
            spdlog::error("order is not exist, row:{}, orderId:{}", i, orderId);
            continue;
        }

        int rowPosition = i;
        {
            auto price = ui->OrderTable->item(rowPosition, 3);
            price->setText(QString::number(order->price));
        }

        {
            auto ordStatus = ui->OrderTable->item(rowPosition, 4);
            ui->OrderTable->setItem(rowPosition, 4,
                                    FixWidget::getShowValues(order->ordStatus, order->ordQty, order->cumQty,
                                                             ordStatus));
        }

        {
            auto ordQty = ui->OrderTable->item(rowPosition, 6);
            ordQty->setText(QString::number(order->ordQty));
        }

        {
            auto cumQty = ui->OrderTable->item(rowPosition, 7);
            cumQty->setText(QString::number(order->cumQty));
        }

        {
            auto leavesQty = ui->OrderTable->item(rowPosition, 8);
            leavesQty->setText(QString::number(order->leavesQty));
        }
    }
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
    order.sendCompId = fix_session_id.getSenderCompID().getString();
    order.targetCompId = fix_session_id.getTargetCompID().getString();
    order.beginString = fix_session_id.getBeginString().getString();

    if (!exchange.isEmpty()) {
        order.exchange = exchange.toStdString();
    }
    if (!account.isEmpty()) {
        order.account = account.toStdString();
    }
    order.side = side == "Buy" ? FIX::Side_BUY : side == "Sell" ? FIX::Side_SELL : FIX::Side_SELL_SHORT;
    order.positionEffect = position_effect == "Open" ? FIX::PositionEffect_OPEN : FIX::PositionEffect_CLOSE;

    order.ordQty = quantity.toDouble();
    order.securityType = type.toStdString();
    order.ordType = ord_Type == "Limit" ? FIX::OrdType_LIMIT : FIX::OrdType_MARKET;
    if (order.ordType == FIX::OrdType_LIMIT) {
        order.price = price.toDouble();
    } else {
        order.price = 0;
    }
    order.symbol = symbol.toStdString();
    order.orderId = this->getStartOrderId();

    Entrust entrust(order.orderId, order.orderId, "", "D", order.ordQty, order.price);
    order.onRoad = entrust.m_clOrdId;
    order.cumQty = 0;
    order.leavesQty = order.ordQty;

    auto pbtn = qobject_cast<QRadioButton *>(ui->Trading_Session->checkedButton());
    QString name = pbtn->objectName();

    if (QString::compare(name, "Trading_Session_0") == 0) {
        order.tradingSessionId = "0";
    } else if (QString::compare(name, "Trading_Session_1") == 0) {
        order.tradingSessionId = "1";
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

