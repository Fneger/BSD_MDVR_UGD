#ifndef CGENERALUSERADDPRODUCTDLG_H
#define CGENERALUSERADDPRODUCTDLG_H

#include <QDialog>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CGeneralUserAddProductDlg;
}

class CGeneralUserAddProductDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CGeneralUserAddProductDlg(const QMap<QString,PRODUCT_INFO_S> &products,const USER_INFO_S &user, QWidget *parent = nullptr);
    ~CGeneralUserAddProductDlg();
    USER_INFO_S user();
    static bool addGeneralUserProducts(const QMap<QString, PRODUCT_INFO_S> &products, USER_INFO_S &user, QWidget *parent = nullptr);

private slots:
    void on_SelectedAllCheckBox_clicked(bool checked);

    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

    void on_ProductListWidget_clicked(const QModelIndex &index);

private:
    Ui::CGeneralUserAddProductDlg *ui;
    QMap<QString,PRODUCT_INFO_S> m_products;
    USER_INFO_S m_user;
};

#endif // CGENERALUSERADDPRODUCTDLG_H
