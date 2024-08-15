#include "AmendDialog.h"

AmendDialog::AmendDialog(const AmendContext &amendContext, QWidget *parent)
        : QDialog(parent), m_AmendContext(amendContext), ui(new Ui::AmendDialog) {
    ui->setupUi(this);
    ui->OrderNo->setText(QString::fromStdString(m_AmendContext.orderId));
    ui->OrdQty->setValue((int) m_AmendContext.ordQty);
    ui->Price->setValue(m_AmendContext.price);
}

AmendDialog::~AmendDialog() {
    delete ui;
}
