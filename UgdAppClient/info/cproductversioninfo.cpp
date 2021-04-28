#include "cproductversioninfo.h"
#include "ui_cproductversioninfo.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QJsonObject>
#include <QFileDialog>
#include "ccommon.h"
#include "ctcpclient.h"
#include "caddversindlg.h"
#include "cversioninfo.h"
#include "cmcuinfodlg.h"
#include "caddgroupfiledlg.h"
#include "cgroupfileitem.h"
#include "mainwindow.h"

CProductVersionInfo::CProductVersionInfo(CTcpClient *pTcpClient, const QString &sName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CProductVersionInfo),
    m_tcpClient(pTcpClient),
    m_bIsLoading(false)
{
    assert(m_tcpClient != nullptr);
    ui->setupUi(this);
    ui->ProductNameLabel->setText(sName);
    m_producuInfo.productName = sName;
    m_producuInfo.productTypeId = sName.toULongLong();
    m_producuInfo.saveFilePath = QString("./version/%1/").arg(sName);
    ui->AddMcuTypeBtn->hide();
    ui->RemoveMcuTypeBtn->hide();
}

CProductVersionInfo::~CProductVersionInfo()
{
    CCommon::Instance()->RemoveAllItems(ui->CrcVersionListWidget);
    CCommon::Instance()->RemoveAllItems(ui->McuVersionListWidget);
    CCommon::Instance()->RemoveAllItems(ui->GroupFileListWidget);
    CCommon::Instance()->RemoveAllItems(ui->FileListWidget);
    delete ui;
}

const PRODUCT_INFO_S &CProductVersionInfo::productInfo()
{
    return m_producuInfo;
}

void CProductVersionInfo::setProductInfo(const PRODUCT_INFO_S &info)
{
    m_bIsLoading = true;
    m_producuInfo = info;
    m_producuInfo.saveFilePath = QString("./version/%1/").arg(m_producuInfo.productName);

    CCommon::Instance()->RemoveAllItems(ui->CrcVersionListWidget);
    CCommon::Instance()->RemoveAllItems(ui->McuVersionListWidget);
    CCommon::Instance()->RemoveAllItems(ui->GroupFileListWidget);
    CCommon::Instance()->RemoveAllItems(ui->FileListWidget);

    ui->ProductNameLabel->setText(info.productName);
    ui->MessageLabel->setText(info.productDesp);
    foreach (VERSION_INFO_S info, m_producuInfo.crcVersionList) {
        QListWidgetItem *item = new QListWidgetItem(info.name);
        ui->CrcVersionListWidget->addItem(item);
        ui->CrcVersionListWidget->setItemWidget(item,new CVersionInfo(item,info,this));
    }
    ui->CrcVersionListWidget->sortItems();

    int count = ui->CrcVersionListWidget->count();

    while(count--){
        QListWidgetItem *item = ui->CrcVersionListWidget->item(count);
        CVersionInfo *pVersion = (CVersionInfo*)ui->CrcVersionListWidget->itemWidget(item);
        if(pVersion->defaultVersion())
            ui->CrcVersionListWidget->setCurrentRow(count);
    }

    ui->McuTypeNameCmBox->clear();
    if(m_producuInfo.mcuInfos.size() > 0)
    {
        ui->McuTypeNameCmBox->addItems(info.mcuInfos.keys());
        ui->McuTypeNameCmBox->setCurrentIndex(0);
        MCU_INFO_S firstMcu = info.mcuInfos.first();
        foreach (VERSION_INFO_S info, firstMcu.mcuVersionList) {
            QListWidgetItem *item = new QListWidgetItem(info.name);
            ui->McuVersionListWidget->addItem(item);
            ui->McuVersionListWidget->setItemWidget(item,new CVersionInfo(item,info,this));
            if(info.isDefault)
                ui->McuVersionListWidget->setCurrentItem(item);
        }
        ui->McuVersionListWidget->sortItems();
    }

    foreach (GROUP_FILE_INFO_S info, m_producuInfo.groupFileInfos) {
        QListWidgetItem *item = new QListWidgetItem(info.name);
        ui->GroupFileListWidget->addItem(item);
        ui->GroupFileListWidget->setItemWidget(item,new CGroupFileItem(item,info,this));
    }
    ui->GroupFileListWidget->sortItems();
    if(ui->GroupFileListWidget->count() > 0)
        ui->GroupFileListWidget->setCurrentRow(0);
    m_bIsLoading = false;
}

void CProductVersionInfo::hintSelectVersion()
{
    QMessageBox::warning(this,tr("Version Operation"),tr("Please select the corresponding version!"));
}

bool CProductVersionInfo::addMcuType(const MCU_INFO_S &mcuInfo)
{
    bool res = false;
    if(mcuInfo.mcuTypeName.length() <= 0)
    {
        QMessageBox::warning(this,tr("Add Mcu Type"),tr("The mcu type name illegal!"));
        return res;
    }

    QJsonDocument ackDoc;
    PRODUCT_INFO_S body = m_producuInfo;
    body.mcuInfos[mcuInfo.mcuTypeName] = mcuInfo;
    if(m_tcpClient->proxyCall(BdRequestAddMcuType_E,&body,ackDoc))
    {
        m_producuInfo = body;
        ui->McuTypeNameCmBox->addItem(mcuInfo.mcuTypeName);
        ui->McuTypeNameCmBox->setCurrentText(mcuInfo.mcuTypeName);
        res = true;
    }
    else
        QMessageBox::warning(this,tr("Add MCU"),tr("Failed to add new mcu type!"));
    return res;
}

bool CProductVersionInfo::removeMcuType(const MCU_INFO_S &mcuInfo)
{
    bool res = false;
    QJsonDocument ackDoc;
    BD_REQUEST_REMOVE_MCU_S body;
    body.productName = m_producuInfo.productName;
    body.mcuTypeName = mcuInfo.mcuTypeName;
    if(m_tcpClient->proxyCall(BdRequestRemoveMcuType_E,&body,ackDoc))
    {
        m_producuInfo.mcuInfos.remove(body.mcuTypeName);
        int count = ui->McuTypeNameCmBox->count();
        while (count--) {
            if(body.mcuTypeName == ui->McuTypeNameCmBox->itemText(count))
            {
                ui->McuTypeNameCmBox->removeItem(count);
                break;
            }
        }
        if(m_producuInfo.mcuInfos.size() > 0)
        {
            QString currMcuTypeName = ui->McuTypeNameCmBox->currentText();
            MCU_INFO_S mcuInfo = m_producuInfo.mcuInfos[currMcuTypeName];
        }
    }
    else
        QMessageBox::warning(this,tr("Remove MCU"),tr("Failed to remove mcu type!"));
    return res;
}

void CProductVersionInfo::setButtonsEnable(bool bEnable)
{
    ui->SetCrcDefaultVersionBtn->setEnabled(bEnable);
    ui->AddNewCrcVersionBtn->setEnabled(bEnable);
    ui->DeleteSelectedCrcVersionBtn->setEnabled(bEnable);
    ui->DownloadSelectedCrcVersionBtn->setEnabled(bEnable);
    ui->SetMcuDefaultVersionBtn->setEnabled(bEnable);
    ui->AddNewMcuVersionBtn->setEnabled(bEnable);
    ui->DeleteSelectedMcuVersionBtn->setEnabled(bEnable);
    ui->DownloadSelectedMcuVersionBtn->setEnabled(bEnable);
    ui->SetGroupFileDefaultBtn->setEnabled(bEnable);
    ui->AddGroupFileBtn->setEnabled(bEnable);
    ui->DeleteSelectedGroupFileBtn->setEnabled(bEnable);
    ui->AddFileBtn->setEnabled(bEnable);
    ui->RemoveFileBtn->setEnabled(bEnable);
}

bool CProductVersionInfo::addFileToGroup(const QString &fileName)
{
    bool res = false;
    if(fileName.length() < 0)
        return res;
    int row = ui->GroupFileListWidget->currentRow();
    if(row >= 0)
    {

        QListWidgetItem *item = ui->GroupFileListWidget->currentItem();
        CGroupFileItem *pGroup = (CGroupFileItem*)ui->GroupFileListWidget->itemWidget(item);
        GROUP_FILE_INFO_S gorupInfo = pGroup->info();
        QFileInfo fileInfo(fileName);
        VERSION_INFO_S info;
        info.name = fileInfo.fileName();
        info.date = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        info.fileName = info.name;
        info.fileSize = fileInfo.size();
        info.isDefault = false;

        if(m_producuInfo.groupFileInfos[gorupInfo.name].fileInfos.contains(info.name))
        {
            QMessageBox::information(this,tr("Add File"),tr("Adding failed, the file already exists!"));
            return res;
        }
        setButtonsEnable(false);
        if(MainWindow::Instance()->uploadFile(fileName,gorupInfo.savePath,info.name))
        {
            gorupInfo.fileInfos[info.name] = info;
            PRODUCT_INFO_S body = m_producuInfo;
            body.groupFileInfos[gorupInfo.name] = gorupInfo;
            if(gorupInfo.isDefault)
                body.groupFileDefaultVersion = gorupInfo;
            QJsonDocument ackDoc;
            if(m_tcpClient->proxyCall(BdRequestSaveProductInfo_E,&body,ackDoc))
            {
                QListWidgetItem *item = new QListWidgetItem(info.name);
                ui->FileListWidget->insertItem(0,item);
                ui->FileListWidget->setItemWidget(item,new CVersionInfo(item,info,this));
                ui->FileListWidget->setCurrentItem(item);
                ui->FileListWidget->sortItems();
                m_producuInfo = body;
                pGroup->setInfo(gorupInfo);
                res = true;
            }
            else
            {
                BD_REQUEST_DELETE_FILE_S body;
                body.fileFullPath = gorupInfo.savePath + info.name;
                QJsonDocument ackDoc;
                if(!m_tcpClient->proxyCall(BdRequestDeleteFile_E,&body,ackDoc))
                {
                    QMessageBox::warning(this,tr("Delete File"),tr("Failed to delete file!"));
                }
            }
        }
        setButtonsEnable(true);
    }
    else
        hintSelectVersion();
    return res;
}

bool CProductVersionInfo::removeFileFromGroup(int index)
{
    bool res = false;
    if(index < 0)
        return res;
    int row = ui->GroupFileListWidget->currentRow();
    if(row >= 0)
    {
        QListWidgetItem *item = ui->GroupFileListWidget->currentItem();
        CGroupFileItem *pGroup = (CGroupFileItem*)ui->GroupFileListWidget->itemWidget(item);
        GROUP_FILE_INFO_S gorupInfo = pGroup->info();

        QListWidgetItem *fileItem = ui->FileListWidget->item(index);
        CVersionInfo *pVersion = (CVersionInfo*)ui->FileListWidget->itemWidget(fileItem);
        VERSION_INFO_S vInfo = pVersion->versionInfo();
        {
            BD_REQUEST_DELETE_FILE_S body;
            body.fileFullPath = gorupInfo.savePath + vInfo.fileName;
            QJsonDocument ackDoc;
            if(!m_tcpClient->proxyCall(BdRequestDeleteFile_E,&body,ackDoc))
            {
                QMessageBox::warning(this,tr("Remove File"),tr("Failed to delete file!"));
                return res;
            }
        }
        gorupInfo.fileInfos.remove(vInfo.name);
        PRODUCT_INFO_S body = m_producuInfo;
        body.groupFileInfos[gorupInfo.name] = gorupInfo;
        if(gorupInfo.isDefault)
            body.groupFileDefaultVersion = gorupInfo;
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestSaveProductInfo_E,&body,ackDoc))
        {
            CCommon::Instance()->RemoveItem(ui->FileListWidget,index);
            m_producuInfo = body;
            pGroup->setInfo(gorupInfo);
        }
        else
            QMessageBox::warning(this,tr("Remove File"),tr("Failed to remove file from group!"));
    }
    else
        hintSelectVersion();
    return res;
}

void CProductVersionInfo::on_SetCrcDefaultVersionBtn_clicked()
{
    if(ui->CrcVersionListWidget->currentRow() >= 0)
    {
        QListWidgetItem *item = ui->CrcVersionListWidget->currentItem();
        CVersionInfo *pVersion = (CVersionInfo*)ui->CrcVersionListWidget->itemWidget(item);
        QString editName = pVersion->versionInfo().name;
        QMap<QString,VERSION_INFO_S> versionList = m_producuInfo.crcVersionList;
        foreach (VERSION_INFO_S info, versionList) {
            if(info.isDefault)
            {
                info.isDefault = false;
                versionList[info.name] = info;
            }
        }

        versionList[editName].isDefault = true;
        PRODUCT_INFO_S body = m_producuInfo;
        body.crcDefaultVersion = versionList[editName];
        body.crcVersionList = versionList;
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestSetDefaultCrcVersion_E,&body,ackDoc))
        {
            int count = ui->CrcVersionListWidget->count();
            QListWidgetItem *item;
            while(count--){
                item = ui->CrcVersionListWidget->item(count);
                CVersionInfo *pVersion = (CVersionInfo*)ui->CrcVersionListWidget->itemWidget(item);
                if(pVersion->defaultVersion())
                    pVersion->setDefaultVersion(false);
            }
            pVersion->setDefaultVersion(true);
            m_producuInfo = body;
        }
        else
            QMessageBox::information(this,tr("Set Default Version"),tr("Failed to set the default version!"));
    }
    else
        hintSelectVersion();
}


void CProductVersionInfo::on_AddNewCrcVersionBtn_clicked()
{
    VERSION_INFO_S vInfo;
    if(CAddVersinDlg::addNewVersion(vInfo,this))
    {

        QString localFileName = vInfo.fileName;
        QFileInfo fileInfo(vInfo.fileName);
        vInfo.fileName = fileInfo.fileName();
        vInfo.fileSize = fileInfo.size();
        QStringList strList = vInfo.fileName.split("-");
        bool isIllegal = false;
        if(!vInfo.fileName.contains(m_producuInfo.productName) || fileInfo.suffix() != "crc" || strList.size() < 3)
            isIllegal = true;

        if(strList.first().length() != 3 || strList.at(1).length() != 9)
            isIllegal = true;

        if(isIllegal)
        {
            QMessageBox::information(this,tr("Add Version"),tr("File type is illegal!"));
            return;
        }

        foreach (VERSION_INFO_S info, m_producuInfo.crcVersionList) {
            if(info.name == vInfo.name)
            {
                QMessageBox::information(this,tr("Add Version"),tr("Adding failed, the version already exists!"));
                return;
            }
        }

        PRODUCT_INFO_S body = m_producuInfo;
        body.crcVersionList[vInfo.name] = vInfo;

        QJsonDocument ackDoc;
        setButtonsEnable(false);
        if(!MainWindow::Instance()->uploadFile(localFileName,m_producuInfo.saveFilePath,vInfo.fileName))
        {
            setButtonsEnable(true);
            return;
        }
        setButtonsEnable(true);

        if(m_tcpClient->proxyCall(BdRequestAddNewCrcVersion_E,&body,ackDoc)) //添加成功后，上传升级包
        {

            QListWidgetItem *item = new QListWidgetItem(vInfo.name);
            ui->CrcVersionListWidget->insertItem(0,item);
            ui->CrcVersionListWidget->setItemWidget(item,new CVersionInfo(item,vInfo,this));
            ui->CrcVersionListWidget->setCurrentItem(item);
            ui->CrcVersionListWidget->sortItems();
            m_producuInfo = body;
        }
    }
}

void CProductVersionInfo::on_DeleteSelectedCrcVersionBtn_clicked()
{
    int index = ui->CrcVersionListWidget->currentRow();
    if(index >= 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Remove version.");
        msgBox.setInformativeText("The operation cannot be restored,Do you want to remove the current version?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Cancel)
            return;

        QListWidgetItem *item = ui->CrcVersionListWidget->currentItem();
        CVersionInfo *pVersion = (CVersionInfo*)ui->CrcVersionListWidget->itemWidget(item);
        VERSION_INFO_S removeInfo = pVersion->versionInfo();

        {
            BD_REQUEST_DELETE_FILE_S body;
            body.fileFullPath = m_producuInfo.saveFilePath + pVersion->versionInfo().fileName;
            QJsonDocument ackDoc;
            if(!m_tcpClient->proxyCall(BdRequestDeleteFile_E,&body,ackDoc))
            {
                QMessageBox::warning(this,tr("Remove Version"),tr("Failed to remove file!"));
                return;
            }
        }

        PRODUCT_INFO_S body = m_producuInfo;
        if(body.crcDefaultVersion.name == removeInfo.name)
        {
            body.crcDefaultVersion.name.clear();
            body.crcDefaultVersion.date.clear();
            body.crcDefaultVersion.message.clear();
            body.crcDefaultVersion.fileSize = 0;
            body.crcDefaultVersion.fileName.clear();
            body.crcDefaultVersion.isDefault = false;
        }
        body.crcVersionList.remove(removeInfo.name);
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestRemoveCrcVersion_E,&body,ackDoc))
        {
            CCommon::Instance()->RemoveItem(ui->CrcVersionListWidget,index);
            m_producuInfo = body;
        }
        else
            QMessageBox::warning(this,tr("Remove Version"),tr("Failed to remove version!"));
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_DownloadSelectedCrcVersionBtn_clicked()
{
    if(ui->CrcVersionListWidget->currentRow() >= 0)
    {
        QString dirName = QFileDialog::getExistingDirectory(this,tr("Select Save Path"),Settings.value("UiSettings/sExistingDir",".").toString());
        if(dirName.length() <= 0)
        {
            return;
        }
        Settings.setValue("UiSettings/sExistingDir",dirName);

        QListWidgetItem *item = ui->CrcVersionListWidget->currentItem();
        CVersionInfo *pVersion = (CVersionInfo*)ui->CrcVersionListWidget->itemWidget(item);
        VERSION_INFO_S info = pVersion->versionInfo();
        setButtonsEnable(false);
        MainWindow::Instance()->downloadFile(dirName,m_producuInfo.saveFilePath + info.fileName);
        setButtonsEnable(true);
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_AddMcuTypeBtn_clicked()
{
    MCU_INFO_S mcuInfo;
    if(CMcuInfoDlg::getMcuInfo(m_producuInfo.productName,mcuInfo,this))
    {
        if(m_producuInfo.mcuInfos.contains(mcuInfo.mcuTypeName))
        {
            QMessageBox::information(this,tr("Add Type"),tr("Adding failed, the type name already exists!"));
            return;
        }
        addMcuType(mcuInfo);
    }
}

void CProductVersionInfo::on_RemoveMcuTypeBtn_clicked()
{
    if(ui->McuTypeNameCmBox->currentIndex() < 0)
    {
        QMessageBox::warning(this,tr("Remove Type"),tr("Please select the corresponding type!"));
        return;
    }
    QMessageBox msgBox;
    msgBox.setText("Remove mcu type.");
    msgBox.setInformativeText("The operation cannot be restored,Do you want to remove the current type?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel)
        return;
    MCU_INFO_S mcuInfo;
    mcuInfo.mcuTypeName = ui->McuTypeNameCmBox->currentText();
    removeMcuType(mcuInfo);
}

void CProductVersionInfo::on_SetMcuDefaultVersionBtn_clicked()
{
    if(ui->McuTypeNameCmBox->currentIndex() < 0)
        return;
    int index = ui->McuVersionListWidget->currentRow();
    if(index >= 0)
    {
        QListWidgetItem *item = ui->McuVersionListWidget->currentItem();
        CVersionInfo *pVersion = (CVersionInfo*)ui->McuVersionListWidget->itemWidget(item);
        QString mcuTypeName = ui->McuTypeNameCmBox->currentText();
        VERSION_INFO_S mcuInfo = pVersion->versionInfo();
        QMap<QString,VERSION_INFO_S> versionList = m_producuInfo.mcuInfos[mcuTypeName].mcuVersionList;
        VERSION_INFO_S editMcuInfo = versionList[mcuInfo.name];
        foreach (VERSION_INFO_S info, versionList) {
            if(info.isDefault)
            {
                info.isDefault = false;
                versionList[info.name] = info;
            }
        }
        editMcuInfo.isDefault = true;
        versionList[editMcuInfo.name] = editMcuInfo;
        PRODUCT_INFO_S body = m_producuInfo;
        body.mcuInfos[mcuTypeName].mcuDefaultVersion = mcuInfo;
        body.mcuInfos[mcuTypeName].mcuVersionList = versionList;
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestSetDefaultMcuVersion_E,&body,ackDoc))
        {
            int count = ui->McuVersionListWidget->count();
            QListWidgetItem *item;
            while(count--){
                item = ui->McuVersionListWidget->item(count);
                CVersionInfo *pVersion = (CVersionInfo*)ui->McuVersionListWidget->itemWidget(item);
                if(pVersion->defaultVersion())
                    pVersion->setDefaultVersion(false);
            }
            pVersion->setDefaultVersion(true);
            m_producuInfo = body;
        }
        else
            QMessageBox::information(this,tr("Set Default Version"),tr("Failed to set the default version!"));
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_AddNewMcuVersionBtn_clicked()
{
    VERSION_INFO_S vInfo;
    if(CAddVersinDlg::addNewVersion(vInfo,this))
    {
        QString mcuTypeName;
        QString localFileName = vInfo.fileName;

        QFileInfo fileInfo(vInfo.fileName);
        vInfo.fileName = fileInfo.fileName();
        vInfo.fileSize = fileInfo.size();
        QStringList strList = vInfo.fileName.split("-");
        bool isIllegal = false;
        if(strList.size() >= 3)
        {
            if(strList.first().length() == 3 && strList.at(1).length() == 9 && fileInfo.suffix() == "hex")
            {
                isIllegal = true;
                mcuTypeName = strList.first();
            }
        }

        if(!isIllegal || !vInfo.fileName.contains(m_producuInfo.productName))
        {
            QMessageBox::information(this,tr("Add Version"),tr("File type is illegal!"));
            return;
        }

        if(!m_producuInfo.mcuInfos.contains(mcuTypeName))
        {
            MCU_INFO_S mcuInfo;
            mcuInfo.mcuTypeName = mcuTypeName;
            mcuInfo.mcuTypeId = mcuTypeName.toULongLong();
            if(!addMcuType(mcuInfo))
                return;
        }
        ui->McuTypeNameCmBox->setCurrentText(mcuTypeName);
        qApp->processEvents();

        MCU_INFO_S mcuInfo = m_producuInfo.mcuInfos[mcuTypeName];
        foreach (VERSION_INFO_S info, mcuInfo.mcuVersionList) {
            if(info.name == vInfo.name)
            {
                QMessageBox::information(this,tr("Add Version"),tr("Adding failed, the version already exists!"));
                return;
            }
        }
        QJsonDocument ackDoc;
        setButtonsEnable(false);
        if(!MainWindow::Instance()->uploadFile(localFileName,m_producuInfo.saveFilePath,vInfo.fileName)) //上传成功后，再添加版本信息
        {
            setButtonsEnable(true);
            return;
        }
        setButtonsEnable(true);

        PRODUCT_INFO_S body = m_producuInfo;
        body.mcuInfos[mcuTypeName].mcuVersionList[vInfo.name] = vInfo;

        if(m_tcpClient->proxyCall(BdRequestAddNewMcuVersion_E,&body,ackDoc)) //添加成功后，上传升级包
        {
            QListWidgetItem *item = new QListWidgetItem();
            ui->McuVersionListWidget->insertItem(0,item);
            ui->McuVersionListWidget->setItemWidget(item,new CVersionInfo(item,vInfo,this));
            ui->McuVersionListWidget->setCurrentItem(item);
            ui->McuVersionListWidget->sortItems();
            m_producuInfo = body;
        }
    }
}

void CProductVersionInfo::on_DeleteSelectedMcuVersionBtn_clicked()
{
    if(ui->McuTypeNameCmBox->currentIndex() < 0)
        return;
    int index = ui->McuVersionListWidget->currentRow();
    if(index >= 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Remove version.");
        msgBox.setInformativeText("The operation cannot be restored,Do you want to remove the current version?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Cancel)
            return;

        QListWidgetItem *item = ui->McuVersionListWidget->currentItem();
        CVersionInfo *pVersion = (CVersionInfo*)ui->McuVersionListWidget->itemWidget(item);
        VERSION_INFO_S removeInfo = pVersion->versionInfo();

        {
            BD_REQUEST_DELETE_FILE_S body;
            body.fileFullPath = m_producuInfo.saveFilePath + pVersion->versionInfo().fileName;
            QJsonDocument ackDoc;
            if(!m_tcpClient->proxyCall(BdRequestDeleteFile_E,&body,ackDoc))
            {
                QMessageBox::warning(this,tr("Remove Version"),tr("Failed to remove file!"));
                return;
            }
        }

        QString mcuType = ui->McuTypeNameCmBox->currentText();
        PRODUCT_INFO_S body = m_producuInfo;
        MCU_INFO_S mcuInfo = m_producuInfo.mcuInfos[mcuType];
        if(mcuInfo.mcuDefaultVersion.name == removeInfo.name)
        {
            mcuInfo.mcuDefaultVersion.name.clear();
            mcuInfo.mcuDefaultVersion.date.clear();
            mcuInfo.mcuDefaultVersion.message.clear();
            mcuInfo.mcuDefaultVersion.fileSize = 0;
            mcuInfo.mcuDefaultVersion.isDefault = false;
        }
        mcuInfo.mcuVersionList.remove(removeInfo.name);
        body.mcuInfos[mcuType] = mcuInfo;
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestRemoveMcuVersion_E,&body,ackDoc))
        {
            CCommon::Instance()->RemoveItem(ui->McuVersionListWidget,index);
            if(ui->McuVersionListWidget->count() == 0)
            {
                MCU_INFO_S mcuInfo;
                mcuInfo.mcuTypeName = ui->McuTypeNameCmBox->currentText();
                removeMcuType(mcuInfo);
            }
            m_producuInfo = body;
        }
        else
            QMessageBox::warning(this,tr("Remove Version"),tr("Failed to remove version!"));
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_DownloadSelectedMcuVersionBtn_clicked()
{
    if(ui->McuTypeNameCmBox->currentIndex() < 0)
        return;
    if(ui->McuVersionListWidget->currentRow() >= 0)
    {
        QString dirName = QFileDialog::getExistingDirectory(this,tr("Select Save Path"),Settings.value("UiSettings/sExistingDir",".").toString());
        if(dirName.length() <= 0)
        {
            return;
        }
        Settings.setValue("UiSettings/sExistingDir",dirName);

        QListWidgetItem *item = ui->McuVersionListWidget->currentItem();
        CVersionInfo *pVersion = (CVersionInfo*)ui->McuVersionListWidget->itemWidget(item);
        VERSION_INFO_S info = pVersion->versionInfo();
        setButtonsEnable(false);
        MainWindow::Instance()->downloadFile(dirName,m_producuInfo.saveFilePath + info.fileName);
        setButtonsEnable(true);
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_McuTypeNameCmBox_currentIndexChanged(int index)
{
    if(m_bIsLoading)
        return;
    CCommon::Instance()->RemoveAllItems(ui->McuVersionListWidget);
    if(index >= 0)
    {
        QString mcuTypeName = ui->McuTypeNameCmBox->currentText();
        if(m_producuInfo.mcuInfos.contains(mcuTypeName))
        {
            MCU_INFO_S mcuInfo = m_producuInfo.mcuInfos[mcuTypeName];
            foreach (VERSION_INFO_S info, mcuInfo.mcuVersionList) {
                QListWidgetItem *item = new QListWidgetItem();
                ui->McuVersionListWidget->addItem(item);
                ui->McuVersionListWidget->setItemWidget(item,new CVersionInfo(item,info,this));
            }
        }
    }
}

void CProductVersionInfo::on_SetGroupFileDefaultBtn_clicked()
{
    int index = ui->GroupFileListWidget->currentRow();
    if(index >= 0)
    {
        QListWidgetItem *item = ui->GroupFileListWidget->currentItem();
        CGroupFileItem *pGroup = (CGroupFileItem*)ui->GroupFileListWidget->itemWidget(item);
        GROUP_FILE_INFO_S groupInfo = pGroup->info();

        QMap<QString,GROUP_FILE_INFO_S> groups = m_producuInfo.groupFileInfos;
        foreach (GROUP_FILE_INFO_S info, groups) {
            if(info.isDefault)
                groups[info.name].isDefault = false;
        }
        groupInfo.isDefault = true;
        PRODUCT_INFO_S body = m_producuInfo;
        body.groupFileDefaultVersion = groupInfo;
        body.groupFileInfos = groups;
        body.groupFileInfos[groupInfo.name] = groupInfo;
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestSaveProductInfo_E,&body,ackDoc))
        {
            int count = ui->GroupFileListWidget->count();
            while (count--) {
                QListWidgetItem *item = ui->GroupFileListWidget->item(count);
                CGroupFileItem *pGroup = (CGroupFileItem*)ui->GroupFileListWidget->itemWidget(item);
                pGroup->setDefaultVersion(false);
            }
            pGroup->setDefaultVersion(true);
            m_producuInfo = body;
        }
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_AddGroupFileBtn_clicked()
{
    GROUP_FILE_INFO_S info;
    if(CAddGroupFileDlg::addNewGroupFile(info,this))
    {
        info.savePath = QString("./version/%1/%2/").arg(m_producuInfo.productName).arg(info.name);
        if(m_producuInfo.groupFileInfos.contains(info.name))
        {
            QMessageBox::information(this,tr("Add Group File"),tr("Adding failed, the group file already exists!"));
            return;
        }

        BD_REQUEST_EDIT_GROUP_FILE_S body;
        body.productName = m_producuInfo.productName;
        body.groupFileInfo = info;
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestAddGroupFile_E,&body,ackDoc))
        {
            QListWidgetItem *item = new QListWidgetItem(info.name);
            ui->GroupFileListWidget->insertItem(0,item);
            ui->GroupFileListWidget->setItemWidget(item,new CGroupFileItem(item,info,this));
            ui->GroupFileListWidget->setCurrentItem(item);
            ui->GroupFileListWidget->sortItems();
            m_producuInfo.groupFileInfos[info.name] = info;
        }
        else
            QMessageBox::information(this,tr("Add Group File"),tr("Adding failed!"));
    }
}

void CProductVersionInfo::on_DeleteSelectedGroupFileBtn_clicked()
{
    int index = ui->GroupFileListWidget->currentRow();
    if(index >= 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Remove version.");
        msgBox.setInformativeText("The operation cannot be restored,Do you want to remove the current version?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Cancel)
            return;
        QListWidgetItem *item = ui->GroupFileListWidget->currentItem();
        CGroupFileItem *pGroup = (CGroupFileItem*)ui->GroupFileListWidget->itemWidget(item);

        BD_REQUEST_EDIT_GROUP_FILE_S body;
        body.productName = m_producuInfo.productName;
        body.groupFileInfo = pGroup->info();
        QJsonDocument ackDoc;
        if(m_tcpClient->proxyCall(BdRequestRemoveGroupFile_E,&body,ackDoc))
        {
            m_producuInfo.groupFileInfos.remove(body.groupFileInfo.name);
            if(body.groupFileInfo.isDefault)
            {
                m_producuInfo.groupFileDefaultVersion.name.clear();
                m_producuInfo.groupFileDefaultVersion.date.clear();
                m_producuInfo.groupFileDefaultVersion.savePath.clear();
                m_producuInfo.groupFileDefaultVersion.fileInfos.clear();
                m_producuInfo.groupFileDefaultVersion.fileInfos.clear();
                m_tcpClient->proxyCall(BdRequestSaveProductInfo_E,&m_producuInfo,ackDoc);
            }
            CCommon::Instance()->RemoveItem(ui->GroupFileListWidget,index);
            CCommon::Instance()->RemoveAllItems(ui->FileListWidget);
        }
        else
            QMessageBox::information(this,tr("Remove Group File"),tr("Removing failed!"));
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_DownloadSelectedGroupFileBtn_clicked()
{
    int index = ui->GroupFileListWidget->currentRow();
    if(index >= 0)
    {
        QString dirName = QFileDialog::getExistingDirectory(this,tr("Select Save Path"),Settings.value("UiSettings/sExistingDir",".").toString());
        if(dirName.length() <= 0)
        {
            return;
        }
        Settings.setValue("UiSettings/sExistingDir",dirName);


        QDir dir(dirName);

        QListWidgetItem *item = ui->GroupFileListWidget->currentItem();
        CGroupFileItem *pGroup = (CGroupFileItem*)ui->GroupFileListWidget->itemWidget(item);
        GROUP_FILE_INFO_S groupInfo = pGroup->info();
        QString savePath = dirName + "/" + groupInfo.name;
        bool res = false;
        dir.setPath(savePath);
        if(dir.exists())
            res = true;
        else
        {
            dir.setPath(dirName);
            res = dir.mkdir(groupInfo.name);
        }
        if(res)
        {
            foreach (VERSION_INFO_S vInfo, groupInfo.fileInfos) {
                setButtonsEnable(false);
                MainWindow::Instance()->downloadFile(savePath,groupInfo.savePath + vInfo.fileName);
                setButtonsEnable(true);
            }
        }
        else
            QMessageBox::warning(this,tr("Download File"),tr("Failed to create save path!"));
    }
    else
        hintSelectVersion();
}

void CProductVersionInfo::on_AddFileBtn_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,tr("Add File"),Settings.value("UiSettings/sOpenFileDir",".").toString());
    if(files.size() <= 0)
    {
        return;
    }
    QFileInfo fileInfos(files.first());
    Settings.setValue("UiSettings/sOpenFileDir",fileInfos.absolutePath());
    foreach (QString fileName, files) {
        if(!addFileToGroup(fileName))
            break;
    }
}

void CProductVersionInfo::on_RemoveFileBtn_clicked()
{
    QList<QListWidgetItem*> items = ui->FileListWidget->selectedItems();
    if(items.size() > 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Remove File");
        msgBox.setInformativeText("The operation cannot be restored,Do you want to remove the current selected files?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Cancel)
            return;

        int row = -1;
        while (items.size() > 0) {
            row = ui->FileListWidget->row(items.first());
            removeFileFromGroup(row);
            items = ui->FileListWidget->selectedItems();
        }
    }
    else
        QMessageBox::warning(this,tr("Remove File"),tr("Please select the corresponding files!"));
}

void CProductVersionInfo::on_GroupFileListWidget_currentRowChanged(int currentRow)
{
    CCommon::Instance()->RemoveAllItems(ui->FileListWidget);
    if(currentRow >= 0)
    {
        QListWidgetItem *item = ui->GroupFileListWidget->currentItem();
        CGroupFileItem *pGroup = (CGroupFileItem*)ui->GroupFileListWidget->itemWidget(item);
        GROUP_FILE_INFO_S info = pGroup->info();
        foreach (VERSION_INFO_S vInfo, info.fileInfos) {
            QListWidgetItem *item = new QListWidgetItem(vInfo.name);
            ui->FileListWidget->addItem(item);
            ui->FileListWidget->setItemWidget(item,new CVersionInfo(item,vInfo,this));
        }
        ui->FileListWidget->sortItems();
    }
}
