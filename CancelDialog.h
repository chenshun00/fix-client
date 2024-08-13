//
// Created by chenshun on 2024/8/14.
//

#ifndef FIX_CLIENT_CANCELDIALOG_H
#define FIX_CLIENT_CANCELDIALOG_H

#include "ui_CancelDialog.h"

class CancelDialog : public QDialog {
Q_OBJECT

public:
    explicit CancelDialog(QWidget *parent = nullptr) :
            QDialog(parent),
            ui(new Ui::Cancel) {
        ui->setupUi(this);
    }

    ~CancelDialog() override {
        delete ui;
    }

private:
    Ui::Cancel *ui;
};


#endif //FIX_CLIENT_CANCELDIALOG_H
