#ifndef CEVENTSDLG_H
#define CEVENTSDLG_H

#include <QDialog>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CEventsDlg;
}

class CTcpClient;
class QTimer;
class CEventsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CEventsDlg(CTcpClient *pTcpClient, QWidget *parent = nullptr);
    ~CEventsDlg();

private slots:
    void slot_timerTimeout();
    void on_AddEventBtn_clicked();

    void on_RemoveEventBtn_clicked();
    void on_EventListWidget_clicked(const QModelIndex &index);

    void on_DownloadFileBtn_clicked();

    void on_ResetEventBtn_clicked();

private:
    void refreshEvents();
    void hintSelectEvent();
    void setButtonsEnable(bool bEnable);

private:
    Ui::CEventsDlg *ui;
    CTcpClient * m_tcpClient;
    QScopedPointer<QTimer> m_timer;
    QMap<quint64,EVENT_INFO_S> m_events;
};

#endif // CEVENTSDLG_H
