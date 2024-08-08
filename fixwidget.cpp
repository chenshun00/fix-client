#include "fixwidget.h"
#include "ui_fixwidget.h"
#include <QMessageBox>
#include <QUuid>


FixWidget::FixWidget(FIX::SessionSettings s,FIX::FileStoreFactory f,FIX::FileLogFactory fl,ClientApplication* c,QWidget *parent)
    : QWidget(parent),
    sessionSettings(s),
    file_store_factory(f),
    file_log(fl),
    client(c),
    m_initiator(*client,file_store_factory,sessionSettings,file_log),
    ui(new Ui::FixWidget)
{
    ui->setupUi(this);

    m_initiator.start();

    connect(ui->Order, SIGNAL(clicked()), this, SLOT(order()));
    connect(client, &ApplicationBridge::mySignal, this, &FixWidget::receiveMySignal);

}

void FixWidget::receiveMySignal(const FIX::SessionID & s){
    qInfo() << "FixWidget::receiveMySignal" << s.toString() << Qt::endl;
    ui->Session_ID->addItem(QString::fromStdString(s.toString()));
}

void FixWidget::order() {
    const QString symbol = ui->Symbol->text();
    const QString price = ui->Price->text();
    const QString type=   ui->Security_Type->currentText();
    const QString ord_Type=   ui->Ord_Type->currentText();
    const QString exchange=   ui->Exchange->currentText();
    const QString quantity = ui->Quantity->text();
    const QString account = ui->Account->text();

    const QString session_id = ui->Session_ID->currentText();
    if(!this->check(session_id, "sessionId必填")){
        return;
    }
    //session反解析为sessionId
    FIX::SessionID fix_session_id;
    fix_session_id.fromString(session_id.toStdString());


    const QString side =   ui->Side->currentText();
    const QString position_effect = ui->Position_Effect->currentText();


    if (!this->check(quantity, "委托数量必填")){
        return;
    }
    if (!this->check(symbol, "symbol必填")){
        return;
    }
    if(ord_Type == "Limit" && !this->check(price, "price不能为空")) {
        return;
    }

    Order order;
    order.send_comp_id = fix_session_id.getSenderCompID().getString();
    order.target_comp_id = fix_session_id.getTargetCompID().getString();
    order.begin_string = fix_session_id.getBeginString().getString();
    
    if (!exchange.isEmpty()){
        order.exchange = exchange.toStdString();
    }
    if(!account.isEmpty()){
        order.account = account.toStdString();
    }
    order.side = side =="Buy"?FIX::Side_BUY: side =="Sell" ?FIX::Side_SELL :FIX::Side_SELL_SHORT;
    order.open_close = position_effect == "Open" ? FIX::PositionEffect_OPEN : FIX::PositionEffect_CLOSE;

    order.order_qty = quantity.toDouble();
    order.security_type = type.toStdString();
    order.ord_type = ord_Type == "Limit" ? FIX::OrdType_LIMIT :FIX::OrdType_MARKET;
    if(order.ord_type == FIX::OrdType_LIMIT){
        order.price = price.toDouble();
    }else {
        order.price = 0;
    }
    order.symbol = symbol.toStdString();
    order.order_id = this->getId();

    Entrust entrust(order.order_id, order.order_id, "", "D", order.order_qty, order.price);
    order.in_market = entrust.m_cl_ord_id;
    order.on_load = "";

    QRadioButton* pbtn = qobject_cast<QRadioButton*>(ui->Trading_Session->checkedButton());
    QString name = pbtn->objectName();
    if (QString::compare(name, "Trading_Session_0") == 0){
        order.trading_session_id = '0';
    }else if (QString::compare(name, "Trading_Session_1") == 0) {
        order.trading_session_id = '1';
    }else {
        QMessageBox::warning(this, "盘前盘后参数错误", "参数错误");
        return;
    }

    qInfo() << "symbol:" << symbol << ", price:" << price << Qt::endl;
    this->client->send(order, entrust);
}

FixWidget::~FixWidget()
{
    qInfo() << "what fuck" << Qt::endl;
    m_initiator.stop();
    delete client;
    delete ui;
}

String FixWidget::getId(){
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}

bool FixWidget::check(const QString& field, const QString& msg){
    if (field.isEmpty()){
        QMessageBox::warning(this, "参数错误", msg);
        return false;
    }
    return true;
}

