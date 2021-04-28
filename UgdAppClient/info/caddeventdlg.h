#ifndef CADDEVENTDLG_H
#define CADDEVENTDLG_H

#include <QDialog>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CAddEventDlg;
}

class CAddEventDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CAddEventDlg(QWidget *parent = nullptr);
    ~CAddEventDlg();
    EVENT_INFO_S info()const { return m_info; }
    static bool addNewEvent(EVENT_INFO_S &info,QWidget *parent = nullptr);

private slots:
    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

    void on_EventTypeCombx_currentIndexChanged(int index);

private:
    Ui::CAddEventDlg *ui;
    EVENT_INFO_S m_info;
};

#endif // CADDEVENTDLG_H
