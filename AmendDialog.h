#ifndef AMENDDIALOG_H
#define AMENDDIALOG_H

#include <QDialog>

#include "Entrust.h"

#include "ui_amenddialog.h"

namespace Ui {
    class AmendDialog;
}

class AmendDialog : public QDialog {
Q_OBJECT

public:
    explicit AmendDialog(const AmendContext &, QWidget *parent = nullptr);

    ~AmendDialog() override;

private:
    const AmendContext &m_AmendContext;
    Ui::AmendDialog *ui;
};

#endif // AMENDDIALOG_H
