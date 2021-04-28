#include "caddversindlg.h"
#include "ui_caddversindlg.h"
#include <QFileDialog>
#include "ccommon.h"
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>

CAddVersinDlg::CAddVersinDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAddVersinDlg)
{
    ui->setupUi(this);
}

CAddVersinDlg::~CAddVersinDlg()
{
    delete ui;
}

void CAddVersinDlg::on_CancelBtn_clicked()
{
    emit reject();
    close();
}

void CAddVersinDlg::on_OkBtn_clicked()
{
    m_versionInfo.name = ui->VersionNameLineEdit->text();
    m_versionInfo.message = ui->VersionMessageTextEdit->toPlainText();
    m_versionInfo.date = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    if(m_versionInfo.name.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Version"),tr("The version name cannot be empty!"));
        return;
    }
    if(m_versionInfo.fileName.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Version"),tr("The corresponding version upgrade file path cannot be empty!"));
        return;
    }
    emit accept();
    close();
}

void CAddVersinDlg::on_SelectedFilePath_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select Upload File"),Settings.value("UiSettings/sOpenFileDir",".").toString());
    if(fileName.length() > 0)
    {
        QFileInfo fileInfo(fileName);
        Settings.setValue("UiSettings/sOpenFileDir",fileInfo.path());
        ui->FullPathLineEdit->setText(fileName);
        ui->VersionNameLineEdit->setText(fileInfo.fileName());
        m_versionInfo.fileName = fileName;
    }
}

VERSION_INFO_S CAddVersinDlg::versionInfo()
{
    m_versionInfo.name = ui->VersionNameLineEdit->text();
    m_versionInfo.message = ui->VersionMessageTextEdit->toPlainText();
    m_versionInfo.isDefault = false;
    return m_versionInfo;
}

bool CAddVersinDlg::addNewVersion(VERSION_INFO_S &versionInfo, QWidget *parent)
{
    bool res = false;
    CAddVersinDlg *pDlg = new CAddVersinDlg(parent);
    if(pDlg->exec() == QDialog::Accepted)
    {
        versionInfo = pDlg->versionInfo();
        res = true;
    }
    pDlg->deleteLater();
    return res;
}


