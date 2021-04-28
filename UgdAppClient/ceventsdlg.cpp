#include "ceventsdlg.h"
#include "ui_ceventsdlg.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include "ctcpclient.h"
#include "ccommon.h"
#include "caddeventdlg.h"
#include "ceventitem.h"
#include "mainwindow.h"

CEventsDlg::CEventsDlg(CTcpClient *pTcpClient, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CEventsDlg),
    m_tcpClient(pTcpClient)
{
    ui->setupUi(this);
    refreshEvents();
    m_timer.reset(new QTimer(this));
    connect(m_timer.data(),&QTimer::timeout,this,&CEventsDlg::slot_timerTimeout);
    m_timer->setInterval(5000);
    m_timer->start();
    setWindowTitle(tr("Events View"));
}

CEventsDlg::~CEventsDlg()
{
    delete ui;
}

void CEventsDlg::refreshEvents()
{
    m_events.clear();
    CCommon *pCommon = CCommon::Instance();
    pCommon->RemoveAllItems(ui->EventListWidget);
    m_events = pCommon->QueryEvents();
    foreach (EVENT_INFO_S event, m_events) {
        QListWidgetItem *item = new QListWidgetItem(CCommon::Instance()->GetEventName(event.eventType) + event.id);
        ui->EventListWidget->addItem(item);
        ui->EventListWidget->setItemWidget(item,new CEventItem(item,event,this));
        ui->EventListWidget->sortItems();
    }
}

void CEventsDlg::setButtonsEnable(bool bEnable)
{
    if(bEnable)
        m_timer->start();
    else
        m_timer->stop();
    ui->AddEventBtn->setEnabled(bEnable);
    ui->RemoveEventBtn->setEnabled(bEnable);
    ui->DownloadFileBtn->setEnabled(bEnable);
    ui->ResetEventBtn->setEnabled(bEnable);
}

void CEventsDlg::slot_timerTimeout()
{
    refreshEvents();
}

void CEventsDlg::on_AddEventBtn_clicked()
{
    m_timer->stop();
    EVENT_INFO_S body;
    if(CAddEventDlg::addNewEvent(body,this))
    {
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestAddEvent_E,&body,ackDoc))
        {
            QJsonObject ackObj = ackDoc.object();
            if(ackObj.contains("data"))
            {
                QJsonObject dataObj = ackObj["data"].toObject();
                if(dataObj.contains("EventSn"))
                    body.eventSn = dataObj["EventSn"].toString().toULongLong();
            }
            QListWidgetItem *item = new QListWidgetItem(CCommon::Instance()->GetEventName(body.eventType) + body.id);
            ui->EventListWidget->addItem(item);
            ui->EventListWidget->setItemWidget(item,new CEventItem(item,body,this));
            ui->EventListWidget->sortItems();
            m_events[body.eventSn] = body;
        }
    }
    m_timer->start();
}

void CEventsDlg::on_RemoveEventBtn_clicked()
{
    m_timer->stop();
    int row = ui->EventListWidget->currentRow();
    if(row >= 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Remove event.");
        msgBox.setInformativeText("The operation cannot be restored,Do you want to remove the current event?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if(ret != QMessageBox::Cancel)
        {
            QListWidgetItem *item = ui->EventListWidget->currentItem();
            CEventItem *pEvent = (CEventItem*)ui->EventListWidget->itemWidget(item);
            EVENT_INFO_S body = pEvent->info();
            if(body.status == EventStatusExecuting_E)
            {
                QMessageBox::warning(this,tr("Remove Event"),tr("The event is being executed and cannot be removed!"));
                return;
            }
            QJsonDocument ackDoc;
            if(m_tcpClient->proxyCall(BdRequestRemoveEvent_E,&body,ackDoc))
            {
                CCommon::Instance()->RemoveItem(ui->EventListWidget,row);
                m_events.remove(body.eventSn);
            }
        }
    }
    else
        hintSelectEvent();
    m_timer->start();
}

void CEventsDlg::on_EventListWidget_clicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->EventListWidget->item(index.row());
    CEventItem *pEvent = (CEventItem*)ui->EventListWidget->itemWidget(item);
    if(pEvent->info().eventType == EventGetDeviceFiles_E)
        ui->DownloadFileBtn->show();
    else
        ui->DownloadFileBtn->hide();
}

void CEventsDlg::on_DownloadFileBtn_clicked()
{
    m_timer->stop();
    int row = ui->EventListWidget->currentRow();
    if(row >= 0)
    {
        QListWidgetItem *item = ui->EventListWidget->item(row);
        CEventItem *pEvent = (CEventItem*)ui->EventListWidget->itemWidget(item);
        EVENT_INFO_S info = pEvent->info();
        if(info.status == EventStatusWaitting_E)
        {
            QMessageBox::warning(this,tr("Event Operation"),tr("The event is still waiting, please try again after successful execution!"));
        }
        else if(info.status == EventStatusExecuting_E)
        {
            QMessageBox::warning(this,tr("Event Operation"),tr("The event is being executed, please try again after successful execution!"));
        }
        else if(info.status == EventStatusFailed_E)
        {
            QMessageBox::warning(this,tr("Event Operation"),tr("Event execution failed, unable to download!"));
        }
        else
        {
            QString dirName = QFileDialog::getExistingDirectory(this,tr("Select Save Path"),Settings.value("UiSettings/sExistingDir",".").toString());
            if(dirName.length() > 0)
            {
                Settings.setValue("UiSettings/sExistingDir",dirName);
                setButtonsEnable(false);
                MainWindow::Instance()->downloadFile(dirName,info.ackPara1);
                setButtonsEnable(true);
            }
        }

    }
    else
        hintSelectEvent();
    m_timer->start();
}

void CEventsDlg::on_ResetEventBtn_clicked()
{
    m_timer->stop();
    int row = ui->EventListWidget->currentRow();
    if(row >= 0)
    {
        QListWidgetItem *item = ui->EventListWidget->item(row);
        CEventItem *pEvent = (CEventItem*)ui->EventListWidget->itemWidget(item);
        EVENT_INFO_S info = pEvent->info();
        if(info.status == EventStatusExecuting_E)
        {
            QMessageBox::warning(this,tr("Reset Event"),tr("The event is being executed and cannot be reset!"));
            return;
        }

        BD_REQUEST_SET_EVENT_STATUS_S body;
        body.eventSn = info.eventSn;
        body.status = EventStatusWaitting_E;
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestSetEventStatus_E,&body,ackDoc))
        {
            info.status = EventStatusWaitting_E;
            info.ackPara1.clear();
            info.ackPara2.clear();
            pEvent->setInfo(info);
            m_events[info.eventSn] = info;
        }
    }
    else
        hintSelectEvent();
    m_timer->start();
}

void CEventsDlg::hintSelectEvent()
{
    QMessageBox::warning(this,tr("Event Operation"),tr("Please select the corresponding event!"));
}
