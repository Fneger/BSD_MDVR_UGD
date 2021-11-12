#include "caddeventdlg.h"
#include "ui_caddeventdlg.h"
#include <QMessageBox>
#include <QFileInfo>

CAddEventDlg::CAddEventDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAddEventDlg)
{
    ui->setupUi(this);
    //ui->Para2NameLabel->hide();
    //ui->Para2Edit->hide();
    on_EventTypeCombx_currentIndexChanged(0);
}

CAddEventDlg::~CAddEventDlg()
{
    delete ui;
}

void CAddEventDlg::on_CancelBtn_clicked()
{
    emit reject();
    close();
}

void CAddEventDlg::on_OkBtn_clicked()
{
    m_info.eventType = ui->EventTypeCombx->currentIndex();
    m_info.idType = ui->IdTypeCombx->currentIndex();
    m_info.id = ui->IdEdit->text();
    m_info.eventPara1 = ui->Para1Edit->text();
    //m_info.eventPara2 = ui->Para2Edit->text();
    m_info.status = EventStatusWaitting_E;
    if(m_info.id.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Event"),tr("The specified ID cannot be empty!"));
        return;
    }
    if(m_info.eventPara1.length() <= 0 && m_info.eventType == EventGetDeviceFiles_E)
    {
        QMessageBox::warning(this,tr("Add Event"),tr("Target path cannot be empty!"));
        return;
    }
    if(m_info.eventType == EventGetDeviceFiles_E)
    {
        if(m_info.eventPara1.mid(0,1) != "/")
        {
            QMessageBox::warning(this,tr("Add Event"),tr("The file path format error!"));
            return;
        }
    }
    else if(m_info.eventType == EventUpgradeDeviceCrc_E)
    {
        if(m_info.eventPara1.size() > 0 && (!m_info.eventPara1.contains("DVR") || !m_info.eventPara1.contains(".crc")))
        {
            QMessageBox::warning(this,tr("Add Event"),tr("The specify version format error!"));
            return;
        }
    }
    else if(m_info.eventType == EventUpgradeDeviceMcu_E)
    {
        if(m_info.eventPara1.size() > 0 && m_info.eventPara1.contains(".hex"))
        {
            QMessageBox::warning(this,tr("Add Event"),tr("The specify version format error!"));
            return;
        }
    }
    else if(m_info.eventType == EventUpgradeDevicePara_E)
    {
    }
    emit accept();
    close();
}

bool CAddEventDlg::addNewEvent(EVENT_INFO_S &info, QWidget *parent)
{
    bool res = false;
    CAddEventDlg dlg(parent);
    if(dlg.exec() == QDialog::Accepted)
    {
        info = dlg.info();
        res = true;
    }
    return res;
}

void CAddEventDlg::on_EventTypeCombx_currentIndexChanged(int index)
{
    switch (index) {
    case EventGetDeviceFiles_E:
        ui->Para1NameLabel->setText(tr("Device File Path:"));
        ui->TipsLabel->setText(tr("Tips: When obtaining the device file, you need to correctly specify the full path of the file in the device!"));
        break;
    default:
        ui->Para1NameLabel->setText(tr("Specify Version"));
        ui->TipsLabel->setText(tr("Tips: When the specified version name is not filled in, it will be upgraded according to the default version. When filling, it will be upgraded according to the specified version. Note that the specified version name must be legal and exist!"));
        break;
    }
}
