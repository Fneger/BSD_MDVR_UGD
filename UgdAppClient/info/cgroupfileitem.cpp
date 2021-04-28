#include "cgroupfileitem.h"
#include "ui_cgroupfileitem.h"
#include <QListWidget>

CGroupFileItem::CGroupFileItem(QListWidgetItem *item,GROUP_FILE_INFO_S &info,QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CGroupFileItem),
    m_item(item),
    m_info(info)
{
    ui->setupUi(this);
    ui->NameLabel->setText(m_info.name);
    ui->DateLabel->setText(m_info.date);
    ui->MessageEdit->setText(m_info.message);
    ui->BriefMessageLabel->setText(m_info.message);
    ui->MessageGroupBox->hide();
    QSize size = m_item->sizeHint();
    size.setHeight(42);
    setDefaultVersion(m_info.isDefault);
    m_item->setSizeHint(size);
}

CGroupFileItem::~CGroupFileItem()
{
    delete ui;
}

GROUP_FILE_INFO_S CGroupFileItem::info()
{
    m_info.message = ui->MessageEdit->toPlainText();
    return m_info;
}

void CGroupFileItem::setInfo(const GROUP_FILE_INFO_S &info)
{
    m_info = info;
}

void CGroupFileItem::setDefaultVersion(bool isDefault)
{
    m_info.isDefault = isDefault;
    if(m_info.isDefault)
        ui->UsedMarkLabel->setText(tr("(Default Version)"));
    else
        ui->UsedMarkLabel->setText("");
}

void CGroupFileItem::on_DetailBtn_clicked()
{
    QSize size = m_item->sizeHint();
    if(ui->MessageGroupBox->isVisible())
    {
        size.setHeight(42);
        ui->MessageGroupBox->hide();
    }
    else
    {
        size.setHeight(127);
        ui->MessageGroupBox->show();
    }
    m_item->setSizeHint(size);
}
