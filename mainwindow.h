#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "atomic"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

public slots:

    void print();

    void on_LoadFile_clicked();

private:
    Ui::MainWindow *ui;
    std::atomic<bool> running = false;
};

#endif // MAINWINDOW_H
