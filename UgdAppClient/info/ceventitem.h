#ifndef CEVENTITEM_H
#define CEVENTITEM_H

#include <QFrame>
#include "st_message.h"

using namespace BdUgdServer;

namespace Ui {
class CEventItem;
}
class QListWidgetItem;
class CEventItem : public QFrame
{
    Q_OBJECT

public:
    explicit CEventItem(QListWidgetItem *item,const EVENT_INFO_S &info, QWidget *parent = nullptr);
    ~CEventItem();
    EVENT_INFO_S info()const { return m_info; }
    void setInfo(const EVENT_INFO_S &info);

private:
    Ui::CEventItem *ui;
    QListWidgetItem *m_item;
    EVENT_INFO_S m_info;
};

#endif // CEVENTITEM_H
