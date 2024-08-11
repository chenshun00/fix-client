//
// Created by chenshun on 2024/8/11.
//

#ifndef FIX_CLIENT_CANCELDIALOG_H
#define FIX_CLIENT_CANCELDIALOG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class CancelDialog; }
QT_END_NAMESPACE

class CancelDialog : public QDialog {
Q_OBJECT

public:
    explicit CancelDialog(QWidget *parent = nullptr);

    ~CancelDialog() override;

private:
    Ui::CancelDialog *ui;
};


#endif //FIX_CLIENT_CANCELDIALOG_H
