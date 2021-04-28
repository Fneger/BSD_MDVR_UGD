#include "cmcuinfodlg.h"
#include "ui_cmcuinfodlg.h"
#include <QMessageBox>

CMcuInfoDlg::CMcuInfoDlg(const QString sProductName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CMcuInfoDlg),
    m_productName(sProductName)
{
    ui->setupUi(this);
}

CMcuInfoDlg::~CMcuInfoDlg()
{
    delete ui;
}

MCU_INFO_S CMcuInfoDlg::mcuInfo()
{
    MCU_INFO_S mcuInfo;
    mcuInfo.mcuTypeName = ui->McuTypeNameLineEdit->text();
    mcuInfo.mcuTypeId = mcuInfo.mcuTypeName.toULongLong();
    return mcuInfo;
}

void CMcuInfoDlg::on_CancelBtn_clicked()
{
    m_clickedType = QDialog::Rejected;
    close();
}

void CMcuInfoDlg::on_OkBtn_clicked()
{
    if(ui->McuTypeNameLineEdit->text().length() <= 0)
    {
        QMessageBox::warning(this,tr("File Info"),tr("File type name cannot be empty!"));
        return;
    }
    m_clickedType = QDialog::Accepted;
    close();
}

bool CMcuInfoDlg::getMcuInfo(const QString &sProductName,MCU_INFO_S &mcuInfo, QWidget *parent)
{
    bool res = false;
    CMcuInfoDlg *pMcuInfoDlg = new CMcuInfoDlg(sProductName,parent);
    pMcuInfoDlg->exec();
    if(pMcuInfoDlg->clickedType() == QDialog::Accepted)
    {
        mcuInfo = pMcuInfoDlg->mcuInfo();
        res = true;
    }
    pMcuInfoDlg->deleteLater();
    return res;
}
