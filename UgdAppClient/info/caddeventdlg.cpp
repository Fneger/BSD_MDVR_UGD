#include "caddeventdlg.h"
#include "ui_caddeventdlg.h"
#include <QMessageBox>
#include <QFileInfo>

CAddEventDlg::CAddEventDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAddEventDlg)
{
    ui->setupUi(this);
    ui->EventTypeCombx->removeItem(1);
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
    m_info.id = ui->IdLineEdit->text();
    m_info.eventPara1 = ui->ParaLineEdit->text();
    m_info.status = EventStatusWaitting_E;
    if(m_info.id.length() <= 0 || m_info.eventPara1.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Event"),tr("The event id and parameter cannot be empty!"));
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
    else if(m_info.eventType == EventUpgradeDevice_E)
    {
        if(!m_info.eventPara1.contains("DVR") || !m_info.eventPara1.contains(".crc"))
        {
            QMessageBox::warning(this,tr("Add Event"),tr("The specify version format error!"));
            return;
        }
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
        ui->IdTypeCombx->setEnabled(true);
        ui->ParaNameLabel->setText(tr("Device File Path:"));
        break;
    default:
        ui->IdTypeCombx->setCurrentIndex(0);
        ui->IdTypeCombx->setEnabled(false);
        ui->ParaNameLabel->setText(tr("Specify Version"));
        break;
    }
}
