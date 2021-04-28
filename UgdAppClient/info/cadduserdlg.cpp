#include "cadduserdlg.h"
#include "ui_cadduserdlg.h"
#include <QDateTime>
#include <QMessageBox>

CAddUserDlg::CAddUserDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAddUserDlg)
{
    ui->setupUi(this);
}

CAddUserDlg::~CAddUserDlg()
{
    delete ui;
}

USER_INFO_S CAddUserDlg::info()
{
    m_info.auth = ui->AuthComboBox->currentIndex();
    m_info.date = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    m_info.message = ui->MessageEdit->toPlainText();
    return m_info;
}

void CAddUserDlg::on_CancelBtn_clicked()
{
    emit reject();
    close();
}

void CAddUserDlg::on_OkBtn_clicked()
{
    m_info.name = ui->NameEdit->text();
    m_info.password = ui->PasswordEdit->text();
    if(m_info.name.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add User"),tr("The user name cannot be empty!"));
        return;
    }
    if(m_info.password.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add User"),tr("The password cannot be empty!"));
        return;
    }
    emit accept();
    close();
}

bool CAddUserDlg::addNewUser(USER_INFO_S &info,QWidget *parent)
{
    bool res = false;
    CAddUserDlg pDlg(parent);
    if(pDlg.exec() == Accepted)
    {
        info = pDlg.info();
        res = true;
    }
    return res;
}
