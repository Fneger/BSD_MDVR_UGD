#include "cadministratordlg.h"
#include "ui_cadministratordlg.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QMenu>
#include "ctcpclient.h"
#include "cuseritem.h"
#include "cproductitem.h"
#include "caddproductdlg.h"
#include "cadduserdlg.h"
#include "ccommon.h"
#include "cgeneraluseraddproductdlg.h"

CAdministratorDlg::CAdministratorDlg(CTcpClient *pTcpClient, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAdministratorDlg),
    m_tcpClient(pTcpClient),
    m_bIsLoading(false)
{
    ui->setupUi(this);

    loadProductList();
    loadUserList();
}

CAdministratorDlg::~CAdministratorDlg()
{
    m_bIsLoading = true;
    CCommon::Instance()->RemoveAllItems(ui->ProductListWidget);
    CCommon::Instance()->RemoveAllItems(ui->UserListWidget);
    m_bIsLoading = false;
    delete ui;
}

void CAdministratorDlg::loadProductList()
{
    m_bIsLoading = true;
    m_products.clear();
    CCommon::Instance()->RemoveAllItems(ui->ProductListWidget);
    m_products = CCommon::Instance()->LoadProducts();
    foreach (PRODUCT_INFO_S info, m_products) {
        QListWidgetItem *item = new QListWidgetItem(info.productName);
        ui->ProductListWidget->addItem(item);
        CProductItem *pInfo = new CProductItem(item,info,this);
        ui->ProductListWidget->setItemWidget(item,pInfo);
    }
    m_bIsLoading = false;
}

void CAdministratorDlg::loadUserList()
{
    QJsonDocument ackDoc;
    m_users.clear();
    CCommon::Instance()->RemoveAllItems(ui->UserListWidget);

    if(m_tcpClient->proxyCall(BdRequestGetUserList_E,NULL,ackDoc))
    {
        QJsonObject ackObj = ackDoc.object();
        if(!ackObj.contains("data"))
            return;
        m_bIsLoading = true;
        QJsonObject usersObj = ackObj["data"].toObject();
        QStringList keys = usersObj.keys();
        foreach (QString key, keys) {
            QJsonObject userObj = usersObj[key].toObject();
            if(userObj.contains("Name") && userObj.contains("Password") && userObj.contains("Auth") && userObj.contains("Date")
                    && userObj.contains("Message") && userObj.contains("Products"))
            {
                USER_INFO_S user;
                user.name = userObj["Name"].toString();
                user.password = userObj["Password"].toString();
                user.auth = userObj["Auth"].toInt() >= 0 ? userObj["Auth"].toInt() : 0;
                user.date = userObj["Date"].toString();
                user.message = userObj["Message"].toString();
                user.products.clear();
                QJsonArray productArray = userObj["Products"].toArray();
                foreach (QJsonValue val, productArray) {
                    user.products << val.toString();
                }

                QListWidgetItem *item = new QListWidgetItem(user.name);
                ui->UserListWidget->addItem(item);
                ui->UserListWidget->setItemWidget(item,new CUserItem(item,user,this));
                m_users[user.name] = user;
            }
        }
        ui->UserListWidget->sortItems();
        m_bIsLoading = false;
    }
    if(ui->UserListWidget->count() > 0)
    {
        ui->UserListWidget->setCurrentRow(0);
    }
}

void CAdministratorDlg::on_AddUserBtn_clicked()
{
    USER_INFO_S body;
    if(CAddUserDlg::addNewUser(body,this))
    {
        foreach (USER_INFO_S info, m_users) {
            if(info.name == body.name)
            {
                QMessageBox::warning(this,tr("Add User"),tr("User already exists!"));
                return;
            }
        }
        QJsonDocument ackDoc;
        if(!m_tcpClient->proxyCall(BdRequestAddUser_E,&body,ackDoc))
        {
            QMessageBox::warning(this,tr("Add User"),tr("Failed to add user!"));
            return;
        }
        m_users[body.name] = body;
        QListWidgetItem *item = new QListWidgetItem(body.name);
        ui->UserListWidget->insertItem(0,item);
        ui->UserListWidget->setItemWidget(item,new CUserItem(item,body,this));
        ui->UserListWidget->setCurrentItem(item);
        ui->UserListWidget->sortItems();
    }
}

void CAdministratorDlg::on_DeleteUserBtn_clicked()
{
    if(ui->UserListWidget->currentRow() < 0)
        return;
    QMessageBox msgBox;
    msgBox.setText("Delete user.");
    msgBox.setInformativeText("After the user is deleted, it cannot be restored. Do you want to delete it?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel)
        return;

    int row = ui->UserListWidget->currentRow();
    QListWidgetItem *item = ui->UserListWidget->currentItem();
    CUserItem *pInfo = (CUserItem*)ui->UserListWidget->itemWidget(item);
    QJsonDocument ackJson;
    USER_INFO_S body = pInfo->info();
    if(m_tcpClient->proxyCall(BdRequestRemoveUser_E,&body,ackJson))
    {
        CCommon::Instance()->RemoveItem(ui->UserListWidget,row);
        m_users.remove(body.name);
    }
}

void CAdministratorDlg::on_AddProductBtn_clicked()
{
    PRODUCT_INFO_S body;
    if(CAddProductDlg::addNewProduct(body,this))
    {
        QJsonDocument ackDoc;
        if(!m_tcpClient->proxyCall(BdRequestAddProductInfo_E,&body,ackDoc))
        {
            QMessageBox::warning(this,tr("Add Product"),tr("Failed to add product information!"));
            return;
        }
        m_products[body.productName] = body;
        QListWidgetItem *item = new QListWidgetItem(body.productName);
        ui->ProductListWidget->addItem(item);
        ui->ProductListWidget->setItemWidget(item,new CProductItem(item,body,this));
        ui->ProductListWidget->sortItems();
        on_UserListWidget_currentItemChanged(ui->UserListWidget->currentItem(),ui->UserListWidget->currentItem());
    }
}

void CAdministratorDlg::on_DeleteProductBtn_clicked()
{
    if(ui->ProductListWidget->currentRow() < 0)
        return;
    QMessageBox msgBox;
    msgBox.setText("Delete product.");
    msgBox.setInformativeText("After the product is deleted, it cannot be restored. Do you want to delete it?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel)
        return;

    int row = ui->ProductListWidget->currentRow();
    QListWidgetItem *item = ui->ProductListWidget->currentItem();
    CProductItem *pInfo = (CProductItem*)ui->ProductListWidget->itemWidget(item);
    QJsonDocument ackJson;
    PRODUCT_INFO_S body = pInfo->info();
    if(m_tcpClient->proxyCall(BdRequestDeleteProductInfo_E,&body,ackJson))
    {
        CCommon::Instance()->RemoveItem(ui->ProductListWidget,row);
        foreach (USER_INFO_S userInfo, m_users) {
            QStringList products = userInfo.products;
            int index = 0;
            foreach (QString name, products) {
                if(name == body.productName)
                {
                    products.removeAt(index);
                    break;
                }
                index++;
            }
            userInfo.products = products;
            QJsonDocument ackDoc;
            if(m_tcpClient->proxyCall(BdRequestEditUser_E,&userInfo,ackDoc))
            {
                m_users[userInfo.name] = userInfo;
            }
        }
        on_UserListWidget_currentItemChanged(ui->UserListWidget->currentItem(),ui->UserListWidget->currentItem());
    }
}

void CAdministratorDlg::on_UserAddProductBtn_clicked()
{
    int row = ui->UserListWidget->currentRow();
    if(row >= 0 && row < m_users.size())
    {
        USER_INFO_S info = m_users[ui->UserListWidget->currentItem()->text()];
        if(CGeneralUserAddProductDlg::addGeneralUserProducts(m_products,info,this))
        {
            QJsonDocument ackDoc;
            if(m_tcpClient->proxyCall(BdRequestEditUser_E,&info,ackDoc))
            {
                m_users[info.name] = info;
                on_UserListWidget_currentItemChanged(ui->UserListWidget->currentItem(),ui->UserListWidget->currentItem());
            }
            else
                QMessageBox::warning(this,tr("Add Product"),tr("Failed to add product!"));
        }
    }
}

void CAdministratorDlg::on_UserDeleteProductBtn_clicked()
{
    int row = ui->UserListWidget->currentIndex().row();
    int index = ui->UserProductListWidget->currentRow();
    if(row >= 0 && row < m_users.size() && index >= 0)
    {
        USER_INFO_S info = m_users[ui->UserListWidget->currentItem()->text()];
        info.products.removeAt(index);
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestEditUser_E,&info,ackDoc))
        {
            m_users[info.name] = info;
            on_UserListWidget_currentItemChanged(ui->UserListWidget->currentItem(),ui->UserListWidget->currentItem());
        }
        else
            QMessageBox::warning(this,tr("Delete Product"),tr("Failed to delete product!"));
    }
}

void CAdministratorDlg::on_UserListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    int row = ui->UserListWidget->currentRow();
    if(m_bIsLoading && row < 0)
        return;
    CUserItem *pUser = (CUserItem*)ui->UserListWidget->itemWidget(current);
    if(pUser->info().name == "admin")
        ui->DeleteUserBtn->setEnabled(false);
    else
        ui->DeleteUserBtn->setEnabled(true);

    if(row < m_users.size())
    {
        CCommon::Instance()->RemoveAllItems(ui->UserProductListWidget);
        USER_INFO_S info = m_users[ui->UserListWidget->currentItem()->text()];
        if(info.auth == BdUserAuthAdmin_E)
        {
            ui->UserAddProductBtn->setEnabled(false);
            ui->UserDeleteProductBtn->setEnabled(false);
            foreach (PRODUCT_INFO_S product, m_products) {
                QListWidgetItem *item = new QListWidgetItem(product.productName);
                ui->UserProductListWidget->addItem(item);
                ui->UserProductListWidget->setItemWidget(item,new CProductItem(item,product,this));
            }
        }
        else
        {
            ui->UserAddProductBtn->setEnabled(true);
            ui->UserDeleteProductBtn->setEnabled(true);
            foreach (QString srcProduct, info.products) {
                foreach (PRODUCT_INFO_S dstProduct, m_products) {
                    if(srcProduct == dstProduct.productName)
                    {
                        QListWidgetItem *item = new QListWidgetItem(dstProduct.productName);
                        ui->UserProductListWidget->addItem(item);
                        ui->UserProductListWidget->setItemWidget(item,new CProductItem(item,dstProduct,this));
                        break;
                    }
                }
            }
        }
        ui->UserProductListWidget->sortItems();
    }
}
