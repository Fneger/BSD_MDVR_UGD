#include "cproductitem.h"
#include "ui_cproductitem.h"
#include <QListWidgetItem>
#include <QDebug>

CProductItem::CProductItem(QListWidgetItem *item, PRODUCT_INFO_S &info, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CProductItem),
    m_item(item),
    m_info(info)
{
    ui->setupUi(this);
    ui->NameLabel->setText(m_info.productName);
    ui->DateLabel->setText(m_info.date);
    ui->MessageEdit->setText(m_info.productDesp);
    ui->BriefMessageLabel->setText(m_info.productDesp);
    ui->MessageGroupBox->hide();
    QSize size = m_item->sizeHint();
    size.setHeight(42);
    m_item->setSizeHint(size);
}

CProductItem::~CProductItem()
{
    delete ui;
}

PRODUCT_INFO_S CProductItem::info()
{
    m_info.productDesp = ui->MessageEdit->toPlainText();
    return m_info;
}

void CProductItem::on_DetailBtn_clicked()
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
