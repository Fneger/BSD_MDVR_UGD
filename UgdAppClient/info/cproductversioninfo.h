#ifndef CPRODUCTVERSIONINFO_H
#define CPRODUCTVERSIONINFO_H

#include <QWidget>
#include "st_message.h"

using namespace BdUgdServer;
namespace Ui {
class CProductVersionInfo;
}

class CTcpClient;
class QListWidget;
class CProductVersionInfo : public QWidget
{
    Q_OBJECT

public:
    explicit CProductVersionInfo(CTcpClient *pTcpClient,const QString &sName,QWidget *parent = nullptr);
    ~CProductVersionInfo();
    QString productName()const { return m_producuInfo.productName; }
    const PRODUCT_INFO_S &productInfo();
    void setProductInfo(const PRODUCT_INFO_S &info);

signals:
    void evt_ShowTipsProgressBar(int totalValue,int currValue);
    void evt_ShowTips(QString tips);

private slots:
    void on_SetCrcDefaultVersionBtn_clicked();

    void on_AddNewCrcVersionBtn_clicked();

    void on_DeleteSelectedCrcVersionBtn_clicked();

    void on_DownloadSelectedCrcVersionBtn_clicked();

    void on_AddMcuTypeBtn_clicked();

    void on_RemoveMcuTypeBtn_clicked();

    void on_SetMcuDefaultVersionBtn_clicked();

    void on_AddNewMcuVersionBtn_clicked();

    void on_DeleteSelectedMcuVersionBtn_clicked();

    void on_DownloadSelectedMcuVersionBtn_clicked();

    void on_McuTypeNameCmBox_currentIndexChanged(int index);

    void on_SetGroupFileDefaultBtn_clicked();

    void on_AddGroupFileBtn_clicked();

    void on_DeleteSelectedGroupFileBtn_clicked();

    void on_DownloadSelectedGroupFileBtn_clicked();

    void on_AddFileBtn_clicked();

    void on_RemoveFileBtn_clicked();

    void on_GroupFileListWidget_currentRowChanged(int currentRow);

private:
    void hintSelectVersion();
    bool addMcuType(const MCU_INFO_S &mcuInfo);
    bool removeMcuType(const MCU_INFO_S &mcuInfo);
    void setButtonsEnable(bool bEnable);
    bool addFileToGroup(const QString &fileName);
    bool removeFileFromGroup(int index);

private:
    Ui::CProductVersionInfo *ui;
    CTcpClient *m_tcpClient;
    PRODUCT_INFO_S m_producuInfo;
    bool m_bCancelUpload;
    bool m_bCancelDownload;
    bool m_bIsLoading;
};

#endif // CPRODUCTVERSIONINFO_H
