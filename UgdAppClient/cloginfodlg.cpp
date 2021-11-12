#include "cloginfodlg.h"
#include "ui_cloginfodlg.h"
#include "ctcpclient.h"
#include "ccommon.h"
#include <QMenu>
#include "ccalendardlg.h"
#include "st_global_def.h"
#include <QMessageBox>
#include <QListView>

using namespace GsGlobalDef;
CLogInfoDlg::CLogInfoDlg(CTcpClient *pTcpClient, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CLogInfoDlg),
    m_tcpClient(pTcpClient),
    m_isLoading(false)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Log Search"));
    ui->ContentIndexCbx->setView(new QListView(this));

    ui->LogView->setShowGrid(false);//关闭网格
    ui->LogView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//关闭滚动条
    ui->LogView->verticalHeader()->setVisible(false);//隐藏纵向表头
    QStringList list;
    list << tr("Product") <<
            tr("Device Number") <<
            tr("IMEI") <<
            tr("Fw Version") <<
            tr("Log Type") <<
            tr("Log Subtype") <<
            tr("Status") <<
            tr("Date Time") <<
            tr("Message");
    ui->LogView->setColumnCount(9);
    ui->LogView->setHorizontalHeaderLabels(list);

    ui->LogView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->LogView->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    ui->LogView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);//默认滚动条
    ui->LogView->setSelectionBehavior ( QAbstractItemView::SelectRows);
    ui->LogView->setWordWrap(true);

    ui->TurningLeftBtn->setToolTip(tr("To First Page"));
    ui->TurningRightBtn->setToolTip(tr("To Last Page"));
    ui->BackBtn->setToolTip(tr("Previous Page"));
    ui->MoreBtn->setToolTip(tr("Next Page"));
    memset(&m_pageInfo, 0, sizeof (m_pageInfo));

    ui->TestGroupBox->hide();

    m_dateTimeFormat = "yyyy/MM/dd hh:mm:ss";
    ui->StartDateTimeBtn->setText(Settings.value("UiSettings/sLogSearchStartDateTime", QDateTime::currentDateTime()).toDateTime().toString("yyyy/MM/dd") + " 00:00:00");
    ui->EndDateTimeBtn->setText(Settings.value("UiSettings/sLogSearchEndDateTime", QDateTime::currentDateTime()).toDateTime().toString("yyyy/MM/dd") + " 23:59:59");
    ui->DevNumberEdit->setText(Settings.value("UiSettings/sLogSearchDevNumber", "0001").toString());
    ui->ImeiNumberEdit->setText(Settings.value("UiSettings/sLogSearchImeiNumber", "").toString());
    ui->FwVersionEdit->setText(Settings.value("UiSettings/sLogSearchFwVersion", "").toString());
    ui->FuzzyCheckBox->setChecked(Settings.value("UiSettings/sLogSearchFuzzy", true).toBool());

    ui->DevNumberEditT->setText(Settings.value("UiSettings/sLogSearchDevNumber", "0001").toString());
    ui->ImeiNumberEditT->setText(Settings.value("UiSettings/sLogSearchImeiNumber", "").toString());
    ui->FwVersionEditT->setText(Settings.value("UiSettings/sLogSearchFwVersion", "").toString());

    m_logTypes << "UPGRADE" << "DEVICE";
    ui->LogTypeCmbxT->clear();
    ui->LogTypeCmbxT->addItems(m_logTypes);
    ui->LogTypeCmbx->clear();
    ui->LogTypeCmbx->addItems(m_logTypes);

    m_resultTypes << "SUCC" << "IN EXEC" << "FAILED" << "ALL";
    m_devModuleStatusTypes << "NORMAL" << "ABNORMAL" << "ALL";
    ui->LogResultCmbxT->clear();
    ui->LogResultCmbxT->addItems(m_resultTypes);
    ui->LogResultCmbx->clear();
    ui->LogResultCmbx->addItems(m_resultTypes);
    m_upgradeTypes << "CRC" << "MCU" << "GROUP" << "ALL";
    m_devModuleTypes << "4G" << "GPS" << "DISK" << "PWR" << "ALL";

    int sel = Settings.value("UiSettings/sLogSearchType", 0).toInt();
    ui->LogTypeCmbx->setCurrentIndex(sel >= m_logTypes.size() ? 0 : sel);
    on_LogTypeCmbx_currentIndexChanged(sel);
    sel = Settings.value("UiSettings/sLogSearchResult", 0).toInt();
    ui->LogResultCmbx->setCurrentIndex(sel >= m_resultTypes.size() ? 0 : sel);
}

CLogInfoDlg::~CLogInfoDlg()
{
    delete ui;
}

void CLogInfoDlg::showEvent(QShowEvent *)
{
    on_QueryLogBtn_clicked();
}

void CLogInfoDlg::setProductsName(const QStringList &names)
{
    ui->ProductNameCmbx->clear();
    ui->ProductNameCmbxT->clear();
    ui->ProductNameCmbx->addItems(names);
    ui->ProductNameCmbx->addItem(tr("ALL"));
    ui->ProductNameCmbxT->addItems(names);
    int sel = Settings.value("UiSettings/sLogSearchSelectProduct",0).toInt();
    if(sel < names.size() && sel > 0)
    {
        ui->ProductNameCmbx->setCurrentIndex(sel);
    }
}

void CLogInfoDlg::queryLogs()
{
    if(ui->ProductNameCmbx->count() <= 0)
        return;
    BD_REQUEST_LOG_S info;
    info.productName = ui->ProductNameCmbx->currentText();
    info.devNumber = ui->DevNumberEdit->text();
    info.imeiNumber = ui->ImeiNumberEdit->text();
    info.fwVersion = ui->FwVersionEdit->text();
    info.type = ui->LogTypeCmbx->currentIndex();
    info.subtype = ui->LogSubtypeCmbx->currentIndex();
    info.result = ui->LogResultCmbx->currentIndex();
    info.startDateTime = QDateTime::fromString(ui->StartDateTimeBtn->text(), m_dateTimeFormat);
    info.endDateTime = QDateTime::fromString(ui->EndDateTimeBtn->text(), m_dateTimeFormat);
    info.fuzzySearch = ui->FuzzyCheckBox->isChecked();
    info.pageNo = m_pageInfo.page_no;
    info.pageSize = m_pageInfo.page_size;
    QJsonDocument ackDoc;
    if(m_tcpClient->proxyCall(BdRequestQueryLog_E,&info,ackDoc))
    {
        QJsonObject ackObj = ackDoc.object();
        if(ackObj.contains("data"))
        {
            QJsonObject dataObj = ackObj["data"].toObject();
            if(!dataObj.contains("Logs"))
                return;
            if(dataObj.contains("TotalCount"))
            {
                m_pageInfo.total_size = dataObj["TotalCount"].toInt();
                m_pageInfo.pages = m_pageInfo.total_size / m_pageInfo.page_size + (m_pageInfo.total_size % m_pageInfo.page_size > 0 ? 1 : 0);
                m_pageInfo.last_page = m_pageInfo.pages;
                ui->ContentIndexCbx->clear();
                for(int i = 0; i < m_pageInfo.pages; i++)
                {
                    QString pageIndexStr = tr("NS") + QString::number(i+1) + tr(" Page ") + "/" + tr(" Total ") + QString::number(m_pageInfo.pages) + tr(" Pages ");
                    m_isLoading = true;
                    ui->ContentIndexCbx->addItem(pageIndexStr);
                    m_isLoading = false;
                }
            }
            QJsonArray infoArray = dataObj["Logs"].toArray();

            QList<BD_REQUEST_LOG_S> infos;
            foreach (QJsonValue value, infoArray) {
                QJsonObject infoObj = value.toObject();
                BD_REQUEST_LOG_S info;
                info.productName = infoObj["ProductName"].toString();
                info.devNumber = infoObj["DevNumber"].toString();
                info.imeiNumber = infoObj["ImeiNumber"].toString();
                info.fwVersion = infoObj["FwVersion"].toString();
                info.type = infoObj["Type"].toInt();
                info.subtype = infoObj["Subtype"].toInt();
                info.result = infoObj["Result"].toInt();
                info.dateTime = QDateTime::fromString(infoObj["DateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                info.serverDateTime = QDateTime::fromString(infoObj["ServerDateTime"].toString(), "yyyy-MM-dd hh:mm:ss");
                info.message = infoObj["Message"].toString();
                infos << info;
            }
            updateData(infos);
        }
    }
    m_isLoading = true;
    ui->ContentIndexCbx->setCurrentIndex(m_pageInfo.page_no - 1);
    m_isLoading = false;
}

void CLogInfoDlg::updateData(const QList<BD_REQUEST_LOG_S> &infos)
{
    int row = 0;
    int column = 0;
    QTableWidget *view = ui->LogView;
    ui->LogView->setRowCount(0);
    view->setRowCount(infos.size());
    int index;
    QColor itemsColor;
    foreach (BD_REQUEST_LOG_S info, infos) {
        if(info.result == 0)
            itemsColor = QColor("#FF666666");
        else
            itemsColor = QColor("#FFFF7000");
        column = 0;
        index = ui->ProductNameCmbx->currentIndex();
        if(index == ui->ProductNameCmbx->count() - 1)
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.productName);
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.devNumber);
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.imeiNumber);
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.fwVersion);
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        if(0)
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.type >= m_logTypes.size() ? "UNKNOW" : m_logTypes.at(info.type));
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        index = ui->LogSubtypeCmbx->currentIndex();
        if(index == ui->LogSubtypeCmbx->count() - 1)
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.subtype >= m_logSubtypes.size() - 1 ? "UNKNOW" : m_logSubtypes.at(info.subtype));
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        index = ui->LogResultCmbx->currentIndex();
        if(index == ui->LogResultCmbx->count() - 1)
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.result >= m_statusTypes.size() - 1 ? "UNKNOW" : m_statusTypes.at(info.result));
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.serverDateTime.toString("yyyy-MM-dd hh:mm:ss"));
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        {
            QTableWidgetItem *item = new QTableWidgetItem(info.message);
            item->setTextAlignment(Qt::AlignCenter);
            item->setTextColor(itemsColor);
            view->setItem(row,column++,item);
        }
        row++;
    }
    view->resizeRowsToContents();
}

void CLogInfoDlg::on_QueryLogBtn_clicked()
{
    if(ui->ProductNameCmbx->count() <= 0)
        return;
    if(ui->DevNumberEdit->text().size() <= 0)
    {
        QMessageBox::warning(this,tr("Query Log"),tr("The device number cannot be empty!"));
    }
    m_pageInfo.page_no = 1;
    m_pageInfo.first_page = 1;
    m_pageInfo.page_size = 20;
    ui->ContentIndexCbx->clear();

    QStringList list;
    list << tr("Product") <<
            tr("Device Number") <<
            tr("IMEI") <<
            tr("Fw Version") <<
            tr("Log Type") <<
            tr("Log Subtype") <<
            tr("Status") <<
            tr("Date Time") <<
            tr("Server DT") <<
            tr("Message");
    list.clear();
    int rows = 0;
    int index = ui->ProductNameCmbx->currentIndex();
    if(index == ui->ProductNameCmbx->count() - 1)
    {
        list << tr("Product");
        rows++;
    }
    {
        list << tr("Device Number");
        rows++;
    }
    {
        list << tr("IMEI");
        rows++;
    }
    {
        list << tr("Fw Version");
        rows++;
    }
#if 0
    index = ui->LogTypeCmbxT->currentIndex();
    if(index == ui->LogTypeCmbxT->count() - 1)
    {
        list << tr("Log Type");
        rows++;
    }
#endif
    index = ui->LogSubtypeCmbx->currentIndex();
    if(index == ui->LogSubtypeCmbx->count() - 1)
    {
        list << tr("Log Subtype");
        rows++;
    }
    index = ui->LogResultCmbx->currentIndex();
    if(index == ui->LogResultCmbx->count() - 1)
    {
        list << tr("Status");
        rows++;
    }
    {
        list << tr("Date Time");
        rows++;
    }
    {
        list << tr("Server DT");
        rows++;
    }
    {
        list << tr("Message");
        rows++;
    }
    ui->LogView->setColumnCount(rows);
    ui->LogView->setHorizontalHeaderLabels(list);
    ui->LogView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->LogView->horizontalHeader()->setSectionResizeMode(rows-1, QHeaderView::Stretch);

    queryLogs();

    Settings.setValue("UiSettings/sLogSearchSelectProduct",ui->ProductNameCmbx->currentIndex());
    Settings.setValue("UiSettings/sLogSearchDevNumber", ui->DevNumberEdit->text());
    Settings.setValue("UiSettings/sLogSearchImeiNumber", ui->ImeiNumberEdit->text());
    Settings.setValue("UiSettings/sLogSearchFwVersion", ui->FwVersionEdit->text());
    Settings.setValue("UiSettings/sLogSearchFuzzy", ui->FuzzyCheckBox->isChecked());
    Settings.setValue("UiSettings/sLogSearchType", ui->LogTypeCmbx->currentIndex());
    Settings.setValue("UiSettings/sLogSearchSubtype", ui->LogSubtypeCmbx->currentIndex());
    Settings.setValue("UiSettings/sLogSearchResult", ui->LogResultCmbx->currentIndex());
}

void CLogInfoDlg::on_AddLogBtn_clicked()
{
    if(ui->ProductNameCmbxT->count() <= 0)
        return;
    BD_REQUEST_LOG_S info;
    info.productName = ui->ProductNameCmbxT->currentText();
    info.devNumber = ui->DevNumberEditT->text();
    info.imeiNumber = ui->ImeiNumberEditT->text();
    info.fwVersion = ui->FwVersionEditT->text();
    info.type = ui->LogTypeCmbxT->currentIndex();
    info.subtype = ui->LogSubtypeCmbxT->currentIndex();
    info.result = ui->LogResultCmbxT->currentIndex();
    info.dateTime = ui->DateTimeEditT->dateTime();
    info.message = ui->MessageExitT->toPlainText();
    QJsonDocument ackDoc;
    if(!m_tcpClient->proxyCall(BdRequestAddLog_E,&info,ackDoc))
    {

    }
}

void CLogInfoDlg::on_TurningLeftBtn_clicked()
{
    m_pageInfo.page_no = m_pageInfo.first_page;
    queryLogs();
}

void CLogInfoDlg::on_TurningRightBtn_clicked()
{
    m_pageInfo.page_no = m_pageInfo.last_page;
    queryLogs();
}

void CLogInfoDlg::on_BackBtn_clicked()
{
    if(m_pageInfo.page_no <= m_pageInfo.first_page)
        return;
    m_pageInfo.page_no--;
    queryLogs();
}

void CLogInfoDlg::on_MoreBtn_clicked()
{
    if(m_pageInfo.page_no >= m_pageInfo.last_page)
        return;
    m_pageInfo.page_no++;
    queryLogs();
}

void CLogInfoDlg::on_ContentIndexCbx_currentIndexChanged(int index)
{
    if(m_isLoading || index < 0)
        return;
    m_pageInfo.page_no = index + 1;
    queryLogs();
}

void CLogInfoDlg::on_StartDateTimeBtn_clicked()
{
    QDateTime dt = QDateTime::fromString(ui->StartDateTimeBtn->text(), m_dateTimeFormat);
    if(CCalendarDlg::getSelectDateTime(dt, this))
    {
        Settings.setValue("UiSettings/sLogSearchStartDateTime", dt);
        ui->StartDateTimeBtn->setText(dt.toString(m_dateTimeFormat));
    }
}

void CLogInfoDlg::on_EndDateTimeBtn_clicked()
{
    QDateTime dt = QDateTime::fromString(ui->EndDateTimeBtn->text(), m_dateTimeFormat);
    if(CCalendarDlg::getSelectDateTime(dt, this))
    {
        Settings.setValue("UiSettings/sLogSearchEndDateTime", dt);
        ui->EndDateTimeBtn->setText(dt.toString(m_dateTimeFormat));
    }
}

void CLogInfoDlg::on_LogTypeCmbx_currentIndexChanged(int index)
{
    switch (index) {
    case LOG_UPGRADE_E:
        ui->LogSubtypeCmbx->setEnabled(true);
        ui->LogSubtypeCmbx->clear();
        ui->LogSubtypeCmbx->addItems(m_upgradeTypes);
        ui->LogSubtypeCmbx->setCurrentIndex(0);
        m_logSubtypes = m_upgradeTypes;
        ui->LogResultCmbx->setEnabled(true);
        ui->LogResultCmbx->clear();
        ui->LogResultCmbx->addItems(m_resultTypes);
        ui->LogResultCmbx->setCurrentIndex(0);
        m_statusTypes = m_resultTypes;
        break;
    case LOG_DEV_MODULE_E:
        ui->LogSubtypeCmbx->setEnabled(true);
        ui->LogSubtypeCmbx->clear();
        ui->LogSubtypeCmbx->addItems(m_devModuleTypes);
        ui->LogSubtypeCmbx->setCurrentIndex(0);
        m_logSubtypes = m_devModuleTypes;
        ui->LogResultCmbx->setEnabled(true);
        ui->LogResultCmbx->clear();
        ui->LogResultCmbx->addItems(m_devModuleStatusTypes);
        ui->LogResultCmbx->setCurrentIndex(0);
        m_statusTypes = m_devModuleStatusTypes;
        break;
    default:
        ui->LogSubtypeCmbx->setEnabled(false);
        ui->LogSubtypeCmbx->clear();
        ui->LogSubtypeCmbx->addItem(tr("UNUSE"));
        m_logSubtypes << tr("UNUSE");
        ui->LogResultCmbx->setEnabled(false);
        ui->LogResultCmbx->clear();
        ui->LogResultCmbx->addItems(m_logSubtypes);
        ui->LogResultCmbx->setCurrentIndex(0);
        m_statusTypes = m_logSubtypes;
        break;
    }
}

void CLogInfoDlg::on_LogTypeCmbxT_currentIndexChanged(int index)
{
    switch (index) {
    case LOG_UPGRADE_E:
        ui->LogSubtypeCmbxT->setEnabled(true);
        ui->LogSubtypeCmbxT->clear();
        ui->LogSubtypeCmbxT->addItems(m_upgradeTypes);
        ui->LogSubtypeCmbxT->setCurrentIndex(0);
        ui->LogResultCmbxT->setEnabled(true);
        ui->LogResultCmbxT->clear();
        ui->LogResultCmbxT->addItems(m_resultTypes);
        ui->LogResultCmbxT->setCurrentIndex(0);
        break;
    case LOG_DEV_MODULE_E:
        ui->LogSubtypeCmbxT->setEnabled(true);
        ui->LogSubtypeCmbxT->clear();
        ui->LogSubtypeCmbxT->addItems(m_devModuleTypes);
        ui->LogSubtypeCmbxT->setCurrentIndex(0);
        ui->LogResultCmbxT->setEnabled(true);
        ui->LogResultCmbxT->clear();
        ui->LogResultCmbxT->addItems(m_devModuleStatusTypes);
        ui->LogResultCmbxT->setCurrentIndex(0);
        break;
    default:
        ui->LogSubtypeCmbxT->setEnabled(false);
        ui->LogSubtypeCmbxT->clear();
        ui->LogSubtypeCmbxT->addItem(tr("UNUSE"));
        ui->LogResultCmbxT->setEnabled(false);
        ui->LogResultCmbxT->clear();
        ui->LogResultCmbxT->addItem(tr("UNUSE"));
        break;
    }
}
