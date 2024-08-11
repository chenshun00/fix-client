//
// Created by chenshun on 2024/8/11.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CancelDialog.h" resolved

#include "canceldialog.h"
#include "ui_CancelDialog.h"


CancelDialog::CancelDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::CancelDialog) {
    ui->setupUi(this);
}

CancelDialog::~CancelDialog() {
    delete ui;
}

