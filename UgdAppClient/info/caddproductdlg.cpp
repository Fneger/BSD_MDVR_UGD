#include "caddproductdlg.h"
#include "ui_caddproductdlg.h"
#include <QDateTime>
#include <QMessageBox>

CAddProductDlg::CAddProductDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAddProductDlg)
{
    ui->setupUi(this);
}

CAddProductDlg::~CAddProductDlg()
{
    delete ui;
}

PRODUCT_INFO_S CAddProductDlg::info()
{
    m_info.productTypeId = m_info.productName.toULongLong();
    m_info.date = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    m_info.productDesp = ui->MessageEdit->toPlainText();
    m_info.saveFilePath = QString("./version/%1/").arg(m_info.productName);
    return m_info;

}

void CAddProductDlg::on_CancelBtn_clicked()
{
    emit reject();
    close();
}

void CAddProductDlg::on_OkBtn_clicked()
{
    m_info.productName = ui->NameEdit->text();
    if(m_info.productName.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Product"),tr("The product name cannot be empty!"));
        return;
    }
    emit accept();
    close();
}

bool CAddProductDlg::addNewProduct(PRODUCT_INFO_S &info, QWidget *parent)
{
    bool res = false;
    CAddProductDlg dlg(parent);
    if(dlg.exec() == QDialog::Accepted)
    {
        info = dlg.info();
        res = true;
    }
    return res;
}
