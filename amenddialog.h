#ifndef AMENDDIALOG_H
#define AMENDDIALOG_H

#include <QDialog>

namespace Ui {
class AmendDialog;
}

class AmendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AmendDialog(QWidget *parent = nullptr);
    ~AmendDialog();

private:
    Ui::AmendDialog *ui;
};

#endif // AMENDDIALOG_H
