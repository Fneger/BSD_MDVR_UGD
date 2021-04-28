#ifndef CVERSIONINFO_H
#define CVERSIONINFO_H

#include <QFrame>
#include "st_message.h"


namespace Ui {
class CVersionInfo;
}
using namespace BdUgdServer;

class QListWidgetItem;
class CVersionInfo : public QFrame
{
    Q_OBJECT

public:
    explicit CVersionInfo(QListWidgetItem *item,const VERSION_INFO_S &versionInfo, QWidget *parent = nullptr);
    ~CVersionInfo();
    VERSION_INFO_S versionInfo() { return m_info; }
    void setDefaultVersion(bool isDefault);
    bool defaultVersion()const { return m_info.isDefault; }

private slots:
    void on_DetailBtn_clicked();

private:
    Ui::CVersionInfo *ui;
    QListWidgetItem *m_item;
    VERSION_INFO_S m_info;
};

#endif // CVERSIONINFO_H
