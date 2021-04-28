#include "caddgroupfiledlg.h"
#include "ui_caddgroupfiledlg.h"
#include <QDateTime>
#include <QMessageBox>

CAddGroupFileDlg::CAddGroupFileDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAddGroupFileDlg)
{
    ui->setupUi(this);
}

CAddGroupFileDlg::~CAddGroupFileDlg()
{
    delete ui;
}

GROUP_FILE_INFO_S CAddGroupFileDlg::info()
{
    m_info.name = ui->NameEdit->text();
    m_info.date = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    m_info.message = ui->MessageEdit->toPlainText();
    return m_info;

}

void CAddGroupFileDlg::on_CancelBtn_clicked()
{
    emit reject();
    close();
}

void CAddGroupFileDlg::on_OkBtn_clicked()
{
    m_info.name = ui->NameEdit->text();
    if(m_info.name.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Group File"),tr("The group file name cannot be empty!"));
        return;
    }
    emit accept();
    close();
}

bool CAddGroupFileDlg::addNewGroupFile(GROUP_FILE_INFO_S &info, QWidget *parent)
{
    bool res = false;
    CAddGroupFileDlg dlg(parent);
    if(dlg.exec() == QDialog::Accepted)
    {
        info = dlg.info();
        res = true;
    }
    return res;
}
