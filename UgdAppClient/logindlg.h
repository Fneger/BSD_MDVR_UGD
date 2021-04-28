#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>

namespace Ui {
class LoginDlg;
}
//登录过程
//1.连接服务器
//2.验证用户名和密码
class CTcpClient;
class LoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDlg(CTcpClient *pTcpClient,QWidget *parent = nullptr);
    ~LoginDlg();
    void closeEvent(QCloseEvent *);

private slots:
    void on_CancelBtn_clicked();

    void on_LoginBtn_clicked();

private:
    Ui::LoginDlg *ui;
    CTcpClient *m_tcpClient;
    bool m_bImmediatelyCancel; //立即取消
};

#endif // LOGINDLG_H
