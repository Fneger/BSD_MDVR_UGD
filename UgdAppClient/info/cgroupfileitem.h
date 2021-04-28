#ifndef CGROUPFILEITEM_H
#define CGROUPFILEITEM_H

#include <QFrame>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CGroupFileItem;
}
class QListWidgetItem;
class CGroupFileItem : public QFrame
{
    Q_OBJECT

public:
    explicit CGroupFileItem(QListWidgetItem *item,GROUP_FILE_INFO_S &info,QWidget *parent = nullptr);
    ~CGroupFileItem();
    GROUP_FILE_INFO_S info();
    void setInfo(const GROUP_FILE_INFO_S &info);
    void setDefaultVersion(bool isDefault);
    bool defaultVersion()const { return m_info.isDefault; }

private slots:
    void on_DetailBtn_clicked();

private:
    Ui::CGroupFileItem *ui;
    QListWidgetItem *m_item;
    GROUP_FILE_INFO_S m_info;
};

#endif // CGROUPFILEITEM_H
