#include "amenddialog.h"
#include "ui_amenddialog.h"

AmendDialog::AmendDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AmendDialog)
{
    ui->setupUi(this);
}

AmendDialog::~AmendDialog()
{
    delete ui;
}
