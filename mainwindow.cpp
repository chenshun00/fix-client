#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

#include <quickfix/SessionSettings.h>
#include <quickfix/FileStore.h>
#include <quickfix/SessionID.h>

#include "ClientApplication.h"
#include "fixwidget.h"
#include "FixClientLogFactory.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    this->setWindowTitle("测试用的哦");
    ui->setupUi(this);
    connect(ui->LoadFile, SIGNAL(clicked()), this, SLOT(print()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::print() {
    QFileDialog dialog;
    dialog.setWindowTitle("加载文件");
    dialog.setFileMode(QFileDialog::ExistingFile);
    auto result = dialog.exec();
    if (result == QDialog::Accepted) {
        QString file = dialog.selectedFiles().at(0);
        auto name = file.toStdString();

        try {
            auto settings = std::make_unique<FIX::SessionSettings>(name);

            std::set<FIX::SessionID> session_ids = settings->getSessions();
            if (session_ids.empty()) {
                throw FIX::ConfigError("没有session配置");
            }
            auto fileStoreFactory = std::make_unique<FIX::FileStoreFactory>(*settings);
            auto fixClientLogFactory = std::make_unique<FixClientLogFactory>(*settings);
            auto client = std::make_unique<ClientApplication>(settings.get());

            auto fix = new FixWidget(
                    std::move(settings),
                    std::move(fileStoreFactory),
                    std::move(fixClientLogFactory),
                    std::move(client)
            );

            this->hide();
            fix->show();

        } catch (FIX::ConfigError &e) {
            //deleting null pointer has no effect
            auto detail = e.detail;
            QMessageBox::warning(this, "ERROR", QString::fromStdString(detail));
        } catch (...) {
            //deleting null pointer has no effect
            QMessageBox::warning(this, "ERROR", "文件错误");
        }
    }
}

void MainWindow::on_LoadFile_clicked() {


}

