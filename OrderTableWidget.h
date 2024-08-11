//
// Created by chenshun on 2024/8/10.
//

#ifndef FIX_CLIENT_ORDERTABLEWIDGET_H
#define FIX_CLIENT_ORDERTABLEWIDGET_H

#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

class OrderTableWidget : public QTableWidget {
Q_OBJECT

public:
    explicit OrderTableWidget(QWidget *parent = nullptr) : QTableWidget(0, 8, parent) {
        // 设置水平表头的标签
        QStringList headers;
        headers << "OrderId" << "Symbol" << "OrdType" << "Price" << "OrdStatus" << "Side" << "Quantity" << "CumQty"
                << "LeavesQty";
        setHorizontalHeaderLabels(headers);
//        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//        setSizePolicy(sizePolicy);
        // 其他初始化代码...
        setGeometry(12, 231, 1000, 717);
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override {
        // 获取鼠标点击的位置
        QPoint pos = event->pos();
        // 转换为项索引
        QModelIndex index = indexAt(pos);
        if (index.isValid()) {
            // 获取点击的单元格的项
            QTableWidgetItem *item = itemAt(pos);
            if (item) {
                // 创建菜单
                QMenu menu(this);
                // 添加动作
                QAction *modifyAction = menu.addAction("改单");
                QAction *cancelAction = menu.addAction("撤单");

                // 连接信号
                connect(modifyAction, &QAction::triggered, this, &OrderTableWidget::onModifyItem);
                connect(cancelAction, &QAction::triggered, this, &OrderTableWidget::onCancelItem);
                // 显示菜单
                menu.exec(event->globalPos());
            }
        }
        QTableWidget::contextMenuEvent(event);
    }

public slots:

    void onModifyItem() {
        // 实现“选择为改单”的逻辑
        qDebug() << "选择为改单";
    }

    void onCancelItem() {
        // 实现“扯蛋”的逻辑
        qDebug() << "扯蛋";
    }
};


#endif //FIX_CLIENT_ORDERTABLEWIDGET_H
