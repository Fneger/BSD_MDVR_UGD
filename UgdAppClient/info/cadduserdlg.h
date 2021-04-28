#ifndef CADDUSERDLG_H
#define CADDUSERDLG_H

#include <QDialog>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CAddUserDlg;
}

class CAddUserDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CAddUserDlg(QWidget *parent = nullptr);
    ~CAddUserDlg();
    USER_INFO_S info();
    static bool addNewUser(USER_INFO_S &info,QWidget *parent = nullptr);

private slots:
    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

private:
    Ui::CAddUserDlg *ui;
    USER_INFO_S m_info;
};

#endif // CADDUSERDLG_H
