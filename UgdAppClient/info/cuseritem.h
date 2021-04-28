#ifndef CUSERITEM_H
#define CUSERITEM_H

#include <QFrame>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CUserItem;
}

class QListWidgetItem;
class CUserItem : public QFrame
{
    Q_OBJECT

public:
    explicit CUserItem(QListWidgetItem *item,const USER_INFO_S &userInfo,QWidget *parent = nullptr);
    ~CUserItem();
    USER_INFO_S info();

private slots:
    void on_DetailBtn_clicked();

private:
    Ui::CUserItem *ui;
    QListWidgetItem *m_item;
    USER_INFO_S m_info;
};

#endif // CUSERITEM_H
