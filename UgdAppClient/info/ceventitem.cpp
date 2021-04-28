#include "ceventitem.h"
#include "ui_ceventitem.h"
#include <QListWidgetItem>
#include "ccommon.h"

CEventItem::CEventItem(QListWidgetItem *item, const EVENT_INFO_S &info, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CEventItem),
    m_item(item),
    m_info(info)
{
    ui->setupUi(this);
    setInfo(info);
    QSize size = m_item->sizeHint();
    size.setHeight(40);
    m_item->setSizeHint(size);
}

CEventItem::~CEventItem()
{
    delete ui;
}

void CEventItem::setInfo(const EVENT_INFO_S &info)
{
    m_info = info;
    CCommon *pCommon = CCommon::Instance();
    ui->EventTypeLabel->setText(pCommon->GetEventName(info.eventType));
    ui->IdTypeNameLabel->setText(pCommon->GetEventDeviceIdTypeName(info.idType));
    ui->IdNameLabel->setText(info.id);
    ui->StatusLabel->setText(pCommon->GetEventStatusName(info.status));
    switch (info.status) {
    case EventStatusWaitting_E:
        ui->StatusLabel->setStyleSheet("QLabel{color:blue}");
        break;
    case EventStatusExecuting_E:
        ui->StatusLabel->setStyleSheet("QLabel{color:green}");
        break;
    case EventStatusSucceed_E:
        ui->StatusLabel->setStyleSheet("QLabel{color:#A0522D}");
        break;
    case EventStatusFailed_E:
        ui->StatusLabel->setStyleSheet("QLabel{color:red}");
        break;
    }
    ui->Para1Label->setText(info.eventPara1);
}
