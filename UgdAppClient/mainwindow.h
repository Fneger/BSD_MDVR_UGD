#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include "st_packet_tool.h"

using namespace BdUgdServer;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QStandardItemModel;
class QItemSelectionModel;
class QLabel;
class CProductVersionInfo;
class QProgressBar;
class CTcpClient;
class CTcpClientTest;
class QPushButton;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow *Instance();
    //刷新产品信息
    void refreshProductInfos();
    CTcpClient *tcpClient() { return m_tcpClient.data(); }
    bool uploadFile(const QString &localFullPath, const QString &remotePath, const QString &remoteFileName);
    bool downloadFile(const QString &localPath,const QString &remoteFullPath);

private slots:
    void on_ShowTips(QString tips);
    void on_ShowTipsProgressBar(int totalValue,int currValue);
    void on_StopTransmissionBtnClicked();

    void on_ProductList_clicked(const QModelIndex &index);

    void on_action_ClientTest_triggered(bool checked);

    void on_action_SaveProduct_triggered();

    void on_action_AddProduct_triggered();

    void on_action_DeleteProduct_triggered();

    void on_action_RefreshProductInfo_triggered();

    void on_action_Administrator_triggered();

    void on_action_Events_triggered();

private:
    bool saveProductInfos();
    virtual void timerEvent(QTimerEvent * evt);
    CProductVersionInfo  *addProduct(const QString productName);
    void removeProduct(const QString productName);
    void setActionEnable(bool bEnable);

private:
    Ui::MainWindow *ui;
    static MainWindow *S_pThis;
    QStandardItemModel *m_deviceItemModel;
    QItemSelectionModel *m_itemSelectModel;
    st_packet_tool m_packetTools;
    QLabel *m_tipsItem;
    QLabel *m_tipsTransferredBytes;
    QPushButton *m_stopTransmissionBtn;
    QProgressBar *m_tipsProgressBar;
    QList<CProductVersionInfo*> m_productList;
    QScopedPointer<CTcpClient> m_tcpClient;
    QScopedPointer<CTcpClientTest> m_tcpCilentTests;
    bool m_bCancelUpload;
    bool m_bCancelDownload;
    int m_nTimer;
};
#endif // MAINWINDOW_H
