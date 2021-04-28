#ifndef CADDPRODUCTDLG_H
#define CADDPRODUCTDLG_H

#include <QDialog>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CAddProductDlg;
}

class CAddProductDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CAddProductDlg(QWidget *parent = nullptr);
    ~CAddProductDlg();
    PRODUCT_INFO_S info();
    static bool addNewProduct(PRODUCT_INFO_S &info,QWidget *parent = nullptr);

private slots:
    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

private:
    Ui::CAddProductDlg *ui;
    PRODUCT_INFO_S m_info;

};

#endif // CADDPRODUCTDLG_H
