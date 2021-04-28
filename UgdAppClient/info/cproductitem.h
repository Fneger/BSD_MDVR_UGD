#ifndef CPRODUCTITEM_H
#define CPRODUCTITEM_H

#include <QFrame>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CProductItem;
}

class QListWidgetItem;
class CProductItem : public QFrame
{
    Q_OBJECT

public:
    explicit CProductItem(QListWidgetItem *item,PRODUCT_INFO_S &info,QWidget *parent = nullptr);
    ~CProductItem();
    PRODUCT_INFO_S info();

private slots:
    void on_DetailBtn_clicked();

private:
    Ui::CProductItem *ui;
    QListWidgetItem *m_item;
    PRODUCT_INFO_S m_info;
};

#endif // CPRODUCTITEM_H
