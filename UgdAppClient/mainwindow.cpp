#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cproductversioninfo.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QModelIndex>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "ctcpclient.h"
#include <QProgressBar>
#include <QItemSelectionModel>
#include "ctcpclienttest.h"
#include "cadministratordlg.h"
#include "ccommon.h"
#include "ceventsdlg.h"
#include <QFileInfo>
#include <QElapsedTimer>

MainWindow *MainWindow::S_pThis = nullptr;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_nTimer(0)
{
    S_pThis = this;
    ui->setupUi(this);
    m_deviceItemModel = new QStandardItemModel(this);
    m_itemSelectModel = new QItemSelectionModel(m_deviceItemModel);
    ui->ProductList->setModel(m_deviceItemModel);
    ui->ProductList->setSelectionModel(m_itemSelectModel);
    m_deviceItemModel->setColumnCount(1);
    m_tipsItem = new QLabel(this);
    ui->statusbar->addWidget(m_tipsItem);

    m_tcpClient.reset(new CTcpClient(this));
    CCommon::Instance()->SetTcpClient(m_tcpClient.data());
    connect(m_tcpClient.data(),&CTcpClient::evt_ShowTips,this,&MainWindow::on_ShowTips);

    m_tipsProgressBar = new QProgressBar(this);
    m_tipsProgressBar->setMaximumHeight(15);
    ui->statusbar->addWidget(m_tipsProgressBar);

    m_stopTransmissionBtn = new QPushButton(this);
    connect(m_stopTransmissionBtn,&QPushButton::clicked,this,&MainWindow::on_StopTransmissionBtnClicked);
    m_stopTransmissionBtn->setIcon(QIcon(":/images/stop_transmission.png"));
    //ui->statusbar->addWidget(m_stopTransmissionBtn);
    m_stopTransmissionBtn->hide();

    m_tipsTransferredBytes = new QLabel(this);
    ui->statusbar->addWidget(m_tipsTransferredBytes);

    m_tcpCilentTests.reset(new CTcpClientTest(this));

    ui->action_ClientTest->setVisible(false);
    ui->action_AddProduct->setVisible(false);
    ui->action_DeleteProduct->setVisible(false);
    m_nTimer = startTimer(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow *MainWindow::Instance()
{
    if(S_pThis == nullptr)
        S_pThis = new MainWindow();
    return S_pThis;
}
void MainWindow::refreshProductInfos()
{
    QString title = QString("%1(%2-%3)").arg(qApp->applicationName()).arg(Settings.value("ClientSettings/uuid","U000000000001").toString()).arg(Settings.value("ServerSettings/sUserName","admin").toString());
    this->setWindowTitle(title);
    on_action_RefreshProductInfo_triggered();
    if(m_tcpClient->authLevel() != BdUserAuthAdmin_E)
    {
        ui->action_Administrator->setVisible(false);
    }
}

bool MainWindow::saveProductInfos()
{
    return true;
}

void MainWindow::on_ShowTips(QString tips)
{
    m_tipsItem->setText(tips);
}

void MainWindow::on_ShowTipsProgressBar(int totalValue,int currValue)
{
    int scale = 1;
    if(totalValue > 1000000000)
        scale = 1000000;
    else if(totalValue > 1000000)
        scale = 1000;
    if(totalValue != m_tipsProgressBar->maximum())
        m_tipsProgressBar->setMaximum(totalValue);
    m_tipsProgressBar->setValue(currValue);
}

void MainWindow::on_StopTransmissionBtnClicked()
{
    m_bCancelUpload = true;
    m_bCancelDownload = true;
}

void MainWindow::timerEvent(QTimerEvent * evt)
{
    Q_UNUSED(evt)
    quint64 currSentBytes = CTcpClient::getSentBytes();
    quint64 currRecBytes = CTcpClient::getRecBytes();
    static quint64 s_lastSentBytes = currSentBytes;
    static quint64 s_lasrRecBytes = currRecBytes;
    QString rateStr = QString("Rec %1 KB/s,Sent %2 KB/s").arg((currRecBytes - s_lasrRecBytes)/1024).arg((currSentBytes - s_lastSentBytes)/1024);
    s_lastSentBytes = currSentBytes;
    s_lasrRecBytes = currRecBytes;
    m_tipsTransferredBytes->setText(rateStr);
}

CProductVersionInfo  *MainWindow::addProduct(const QString productName)
{
    CProductVersionInfo *pVersionInfo = new CProductVersionInfo(m_tcpClient.data(),productName,this);
    m_productList << pVersionInfo;
    connect(pVersionInfo,&CProductVersionInfo::evt_ShowTipsProgressBar,this,&MainWindow::on_ShowTipsProgressBar);
    connect(pVersionInfo,&CProductVersionInfo::evt_ShowTips,this,&MainWindow::on_ShowTips);
    ui->ProductInfoWidgets->addWidget(pVersionInfo);
    ui->ProductInfoWidgets->setCurrentWidget(pVersionInfo);
    m_deviceItemModel->appendRow(new QStandardItem(productName));
    QModelIndex index = m_deviceItemModel->index(ui->ProductInfoWidgets->currentIndex(),0);
    m_itemSelectModel->clearSelection();
    m_itemSelectModel->setCurrentIndex(index,QItemSelectionModel::Select);
    return pVersionInfo;
}

void MainWindow::removeProduct(const QString productName)
{
    int index = 0;
    CProductVersionInfo *pItem = nullptr;
    foreach (CProductVersionInfo *pVersionInfo, m_productList) {
        if(pVersionInfo->productName() == productName)
        {
            pItem = pVersionInfo;
            index = m_productList.indexOf(pVersionInfo);
        }
    }

    if(pItem != nullptr)
    {
        m_productList.removeAt(index);
        ui->ProductInfoWidgets->removeWidget(pItem);
        m_deviceItemModel->removeRow(index);
        disconnect(pItem,&CProductVersionInfo::evt_ShowTipsProgressBar,this,&MainWindow::on_ShowTipsProgressBar);
        disconnect(pItem,&CProductVersionInfo::evt_ShowTips,this,&MainWindow::on_ShowTips);
        pItem->deleteLater();
    }
}

void MainWindow::setActionEnable(bool bEnable)
{
    ui->action_SaveProduct->setEnabled(bEnable);
    ui->action_RefreshProductInfo->setEnabled(bEnable);
    ui->action_Events->setEnabled(bEnable);
    ui->action_Administrator->setEnabled(bEnable);
    ui->ProductList->setEnabled(bEnable);
    if(bEnable)
    {
        ui->statusbar->removeWidget(m_stopTransmissionBtn);
        m_stopTransmissionBtn->hide();
    }
    else
    {

        ui->statusbar->insertWidget(2,m_stopTransmissionBtn);
        m_stopTransmissionBtn->show();
    }
}

bool MainWindow::uploadFile(const QString &localFullPath,const QString &remotePath,const QString &remoteFileName)
{
    bool res = false;
    m_bCancelUpload = false;
    if(localFullPath.length() <= 0 || remotePath.length() <= 0)
    {
        QMessageBox::warning(this,tr("Upload File"),tr("The local file name or remote path cannot be empty!"));
        return res;
    }
    BD_REQUEST_UPLOAD_FILE_S body;
    QFileInfo fileInfo(localFullPath);
    if(!fileInfo.exists())
    {
        QMessageBox::warning(this,tr("Upload File"),fileInfo.fileName() + tr(",file does not exist!"));
        return res;
    }
    if(fileInfo.size() <= 0)
    {
        QMessageBox::warning(this,tr("Upload File"),tr("The file size cannot be 0!"));
        return res;
    }
    QString rPath = remotePath;
    if(rPath.mid(rPath.length() - 1) != "/")
        rPath += "/";
    body.remoteFilePath = rPath;
    body.remoteFileName = remoteFileName;
    body.fileSize = fileInfo.size();
    QJsonDocument ackDoc;
    if(m_tcpClient->proxyCall(BdRequestUploadFile_E,&body,ackDoc))
    {
        //开始上传文件
        QFile file(localFullPath);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this,tr("Upload File"),fileInfo.fileName() + tr(",file does not exist!"));
            BD_REQUEST_STOP_UPLOAD_FILE_S body;
            body.reason = ReasonFileOpenFialed_E;
            body.msg = QString("Failed to open file \"%1\"").arg(localFullPath);
            m_tcpClient->proxyCall(BdRequestStopUploadFile_E,&body,ackDoc);
            return res;
        }
        setActionEnable(false);
        BD_TRANSFER_FILE_DATA_S body;
        memset(&body,0,sizeof (BD_TRANSFER_FILE_DATA_S));
        body.file_size = file.size();
        body.not_sent_bytes = body.file_size;
        qint64 rb;
        m_tcpClient->resetCcp();
        qint64 buffer_len = sizeof (body.file_data);
        quint32 cwnd = 0;
        while(body.not_sent_bytes > 0){
#if 1
            cwnd = m_tcpClient->getCcpCwnd();
            cwnd = body.not_sent_bytes > cwnd ? cwnd : body.not_sent_bytes;
            rb = file.read((char*)body.file_data,cwnd);
            body.currently_sent_bytes = rb;
            body.sent_bytes += rb;
            body.not_sent_bytes -= rb;
            if(!m_tcpClient->uploadFileData(body))
                break;
#else
            rb = file.read((char*)body.file_data,buffer_len);
            body.currently_sent_bytes = rb;
            body.sent_bytes += rb;
            body.not_sent_bytes -= rb;
            if(!m_tcpClient->uploadFileData(body))
                break;
#endif
            on_ShowTipsProgressBar(body.file_size,body.sent_bytes);
            qApp->processEvents();

            if(m_bCancelUpload) //取消上传
            {
                m_tcpClient->proxyCall(BdRequestStopUploadFile_E,&body,ackDoc);
                setActionEnable(true);
                file.close();
                return res;
            }
        }
        setActionEnable(true);
        file.close();
        if(body.not_sent_bytes > 0)
        {
            m_tcpClient->proxyCall(BdRequestStopUploadFile_E,&body,ackDoc);
            QMessageBox::warning(this,tr("Upload File"),tr("An error occurred during data transmission!"));
            return res;
        }
        else
        {
            QMessageBox::information(this,tr("Upload File"),tr("File upload successfully!"));
            res = true;
        }
    }
    else
    {
        QMessageBox::warning(this,tr("Upload File"),tr("Upload failed!"));
    }
    return res;
}

bool MainWindow::downloadFile(const QString &localPath,const QString &remoteFullPath)
{
    m_tcpClient->resetCcp();
    bool res = false;
    m_bCancelDownload = false;
    if(localPath.length() <= 0 || remoteFullPath.length() <= 0)
    {
        QMessageBox::warning(this,tr("Download File"),tr("The remote file name or local save path cannot be empty!"));
        return res;
    }
    QString lPath = localPath;
    if(lPath.mid(lPath.length() - 1) != "/")
        lPath += "/";
    BD_REQUEST_DOWNLOAD_FILE_S body;
    QFileInfo remoteFile(remoteFullPath);
    QFileInfo fileInfo(lPath+remoteFile.fileName());
    if(fileInfo.exists())
    {
        QFile::remove(fileInfo.absoluteFilePath());
    }

    body.remoteFileFullPath = remoteFullPath;
    QJsonDocument ackDoc;
    if(m_tcpClient->proxyCall(BdRequestDownloadFile_E,&body,ackDoc))
    {
        QJsonObject ackObj = ackDoc.object();
        if(!ackObj.contains("data"))
        {
            QMessageBox::warning(this,tr("Download File"),tr("Parse json error!"));
            return res;
        }
        QJsonObject dataObj = ackObj["data"].toObject();
        if(!dataObj.contains("FileSize"))
        {
            QMessageBox::warning(this,tr("Download File"),tr("Json format error!"));
            return res;
        }

        uint32_t fileSize = dataObj["FileSize"].toDouble();
        if(fileSize <= 0)
        {
            QMessageBox::warning(this,tr("Download File"),tr("File size error!"));
            return res;
        }
        //开始上传文件
        QFile file(fileInfo.absoluteFilePath());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            QMessageBox::warning(this,tr("Download File"),fileInfo.absoluteFilePath() + tr(",failed to open!"));
            return res;
        }
        setActionEnable(false);
        BD_TRANSFER_FILE_DATA tBody;
        BD_TMN_DOWNLOAD_FILE_DATA_S body;
        memset(&body,0,sizeof (BD_TMN_DOWNLOAD_FILE_DATA_S));
        body.file_size = fileSize;
        body.not_received_bytes = fileSize;
        qint64 wb;
        qint64 buffer_len = sizeof (tBody.file_data);
        quint32 cwnd = m_tcpClient->getCcpCwnd();
        body.request_receive_bytes = body.not_received_bytes > cwnd ? cwnd : body.not_received_bytes;
        while(body.not_received_bytes > 0){
#if 1
            if(!m_tcpClient->downloadFileData(body,tBody))
                break;
            if(tBody.file_size == 0 || tBody.currently_sent_bytes <= 0)
                break;
            wb = file.write((char*)tBody.file_data,tBody.currently_sent_bytes);
            body.not_received_bytes -= wb;
            cwnd = m_tcpClient->getCcpCwnd();
            body.request_receive_bytes = body.not_received_bytes > cwnd ? cwnd : body.not_received_bytes;
            body.received_bytes += wb;
#else
            if(!m_tcpClient->downloadFileData(body,tBody))
                break;
            if(tBody.currently_sent_bytes > buffer_len || tBody.file_size == 0 || tBody.currently_sent_bytes <= 0)
                break;
            wb = file.write((char*)tBody.file_data,tBody.currently_sent_bytes);
            body.not_received_bytes -= wb;
            body.request_receive_bytes = body.not_received_bytes > buffer_len ? buffer_len : body.not_received_bytes;
            body.received_bytes += wb;
#endif


            on_ShowTipsProgressBar(body.file_size,body.received_bytes);
            qApp->processEvents();

            if(m_bCancelDownload) //取消下载
            {
                m_tcpClient->proxyCall(BdRequestStopDownloadFile_E,&body,ackDoc);
                setActionEnable(true);
                file.close();
                return res;
            }
        }
        setActionEnable(true);
        file.close();
        if(body.not_received_bytes > 0)
        {
            m_tcpClient->proxyCall(BdRequestStopDownloadFile_E,&body,ackDoc);
            QMessageBox::warning(this,tr("Download File"),tr("An error occurred during data transmission!"));
            return res;
        }
        else
        {
            QMessageBox::information(this,tr("Download File"),tr("File download successfully!"));
            res = true;
        }
    }
    return res;
}

void MainWindow::on_ProductList_clicked(const QModelIndex &index)
{
    ui->ProductInfoWidgets->setCurrentIndex(index.row());
}


void MainWindow::on_action_ClientTest_triggered(bool checked)
{
    m_tcpCilentTests->startStopTest(checked);
}

void MainWindow::on_action_SaveProduct_triggered()
{
    if(ui->ProductInfoWidgets->currentIndex() >= 0)
    {
        CProductVersionInfo *pVersion = qobject_cast<CProductVersionInfo*>(ui->ProductInfoWidgets->currentWidget());
        QJsonDocument ackJson;
        if(!m_tcpClient->proxyCall(BdRequestSaveProductInfo_E,(void*)&pVersion->productInfo(),ackJson))
            QMessageBox::warning(this,tr("Save Product"),tr("Failed to save product information!"));
    }
}

void MainWindow::on_action_AddProduct_triggered()
{
    QString productName = QInputDialog::getText(this,tr("Input Device Name"),tr("DeviceName"));
    if(productName.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Product"),tr("Product name cannot be empty!"));
        return;
    }
    PRODUCT_INFO_S info;
    info.productName = productName;
    info.productTypeId = productName.toULongLong();
    info.saveFilePath = QString("./version/%1/").arg(productName);
    QJsonDocument ackJson;
    if(!m_tcpClient->proxyCall(BdRequestSaveProductInfo_E,&info,ackJson))
    {
        QMessageBox::warning(this,tr("Save Product"),tr("Failed to save product information!"));
        return;
    }
    addProduct(productName);
}

void MainWindow::on_action_DeleteProduct_triggered()
{
    if(ui->ProductInfoWidgets->currentIndex() < 0)
        return;
    QMessageBox msgBox;
    msgBox.setText("Delete product.");
    msgBox.setInformativeText("After the product is deleted, it cannot be restored. Do you want to delete it?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel)
        return;

    int row = ui->ProductInfoWidgets->currentIndex();
    CProductVersionInfo *pVersionInfo = qobject_cast<CProductVersionInfo*>(ui->ProductInfoWidgets->currentWidget());
    QJsonDocument ackJson;
    if(m_tcpClient->proxyCall(BdRequestDeleteProductInfo_E,(void*)&pVersionInfo->productInfo(),ackJson))
    {
        removeProduct(pVersionInfo->productName());
    }
}

void MainWindow::on_action_RefreshProductInfo_triggered()
{
    foreach (CProductVersionInfo *pVersionInfo, m_productList) {
        removeProduct(pVersionInfo->productName());
    }
    m_tipsItem->setText(tr("Loading product information......"));
    QMap<QString,PRODUCT_INFO_S> products = CCommon::Instance()->LoadProducts();

    foreach (PRODUCT_INFO_S info, products) {
        CProductVersionInfo *pVersionInfo = addProduct(info.productName);
        pVersionInfo->setProductInfo(info);
    }
}

void MainWindow::on_action_Administrator_triggered()
{
    CAdministratorDlg admin(m_tcpClient.data(),this);
    admin.exec();
}

void MainWindow::on_action_Events_triggered()
{
    CEventsDlg eventsDlg(m_tcpClient.data(),this);
    eventsDlg.exec();
}
