#include "cgeneraluseraddproductdlg.h"
#include "ui_cgeneraluseraddproductdlg.h"
#include <QMessageBox>
#include <QDebug>
#include "cproductitem.h"
#include "ccommon.h"

CGeneralUserAddProductDlg::CGeneralUserAddProductDlg(const QMap<QString, PRODUCT_INFO_S> &products, const USER_INFO_S &user, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CGeneralUserAddProductDlg),
    m_products(products),
    m_user(user)
{
    ui->setupUi(this);
    QMap<QString,QString> findUser;
    foreach (QString str, user.products) {
        findUser[str] = str;
    }
    foreach (PRODUCT_INFO_S info, products) {
        if(findUser.contains(info.productName))
            continue;
        QListWidgetItem *item = new QListWidgetItem();
        item->setCheckState(Qt::Unchecked);
        ui->ProductListWidget->addItem(item);
        CProductItem *pInfo = new CProductItem(item,info,this);
        ui->ProductListWidget->setItemWidget(item,pInfo);
    }
}

CGeneralUserAddProductDlg::~CGeneralUserAddProductDlg()
{
    CCommon::Instance()->RemoveAllItems(ui->ProductListWidget);
    delete ui;
}

USER_INFO_S CGeneralUserAddProductDlg::user()
{
    return m_user;
}

void CGeneralUserAddProductDlg::on_SelectedAllCheckBox_clicked(bool checked)
{
    int count = ui->ProductListWidget->count();
    while (count--) {
        if(checked)
            ui->ProductListWidget->item(count)->setCheckState(Qt::Checked);
        else
            ui->ProductListWidget->item(count)->setCheckState(Qt::Unchecked);
    }
}

void CGeneralUserAddProductDlg::on_CancelBtn_clicked()
{
    emit reject();
    close();
}

void CGeneralUserAddProductDlg::on_OkBtn_clicked()
{
    QStringList strList;
    int count = ui->ProductListWidget->count();
    while (count--) {
        QListWidgetItem *item = ui->ProductListWidget->item(count);
        if(item->checkState() == Qt::Checked)
        {
            CProductItem *pInfo = (CProductItem*)ui->ProductListWidget->itemWidget(item);
            strList.push_front(pInfo->info().productName);
        }
    }
    if(strList.size() <= 0)
    {
        QMessageBox::warning(this,tr("Add Product"),tr("The added product cannot be empty!"));
        return;
    }
    m_user .products << strList;
    emit accept();
    close();
}

bool CGeneralUserAddProductDlg::addGeneralUserProducts(const QMap<QString,PRODUCT_INFO_S> &products,USER_INFO_S &user, QWidget *parent)
{
    bool res = false;
    CGeneralUserAddProductDlg pDlg(products,user,parent);
    if(pDlg.exec() == Accepted)
    {
        user = pDlg.user();
        res = true;
    }
    return res;
}

void CGeneralUserAddProductDlg::on_ProductListWidget_clicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->ProductListWidget->item(index.row());
    if(item->checkState() == Qt::Checked)
        item->setCheckState(Qt::Unchecked);
    else
        item->setCheckState(Qt::Checked);
}
