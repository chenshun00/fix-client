#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

#include <quickfix/SessionSettings.h>
#include <quickfix/MessageStore.h>
#include <quickfix/FileStore.h>
#include <quickfix/FileLog.h>
#include <quickfix/SessionID.h>
#include <quickfix/ThreadedSocketInitiator.h>

#include "ClientApplication.h"
#include "Context.h"
#include "fixwidget.h"
#include "FixClientLogFactory.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setWindowTitle("测试用的哦");
    ui->setupUi(this);
    connect(ui->LoadFile, SIGNAL(clicked()), this, SLOT(print()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::print()
{
    QFileDialog dialog;
    dialog.setWindowTitle("加载文件");
    dialog.setFileMode(QFileDialog::ExistingFile);
    auto result = dialog.exec();
    if (result == QDialog::Accepted){
        QString file = dialog.selectedFiles().at(0);
        auto name = file.toStdString();

        try {
            FIX::SessionSettings settings(name);

            std::set < FIX::SessionID > session_ids = settings.getSessions();
            if (!session_ids.size()){
                throw FIX::ConfigError("没有session配置");
            }

            FIX::FileStoreFactory file_store_factory(settings);
            FixClientLogFacotry file_log(settings);

            ClientApplication* client = new ClientApplication(&settings);

            FixWidget * fix = new FixWidget(settings,file_store_factory,file_log,client);

            this->hide();
            fix->show();

        } catch(FIX::ConfigError& e){
            auto detail = e.detail;
            QMessageBox::warning(this, "ERROR", QString::fromStdString(detail));
        } catch (...) {
            QMessageBox::warning(this, "ERROR", "文件错误");
        }
    }
}

void MainWindow::on_LoadFile_clicked()
{


}

