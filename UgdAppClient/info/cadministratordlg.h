#ifndef CADMINISTRATORDLG_H
#define CADMINISTRATORDLG_H

#include <QDialog>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CAdministratorDlg;
}

class CTcpClient;
class QStandardItemModel;
class QItemSelectionModel;
class QListWidgetItem;
class QMenu;
class CAdministratorDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CAdministratorDlg(CTcpClient *pTcpClient,QWidget *parent = nullptr);
    ~CAdministratorDlg();

private slots:
    void on_AddUserBtn_clicked();

    void on_DeleteUserBtn_clicked();

    void on_AddProductBtn_clicked();

    void on_DeleteProductBtn_clicked();

    void on_UserAddProductBtn_clicked();

    void on_UserDeleteProductBtn_clicked();

    void on_UserListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void loadProductList();
    void loadUserList();

private:
    Ui::CAdministratorDlg *ui;
    CTcpClient * m_tcpClient;
    QMap<QString,PRODUCT_INFO_S> m_products;
    QMap<QString,USER_INFO_S> m_users;
    bool m_bIsLoading;
};

#endif // CADMINISTRATORDLG_H
