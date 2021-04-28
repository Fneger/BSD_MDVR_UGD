#include "cuseritem.h"
#include "ui_cuseritem.h"
#include <QListWidgetItem>

CUserItem::CUserItem(QListWidgetItem *item, const USER_INFO_S &userInfo, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CUserItem),
    m_item(item),
    m_info(userInfo)
{
    ui->setupUi(this);
    ui->NameLabel->setText(m_info.name);
    ui->PasswordLabel->setText(m_info.password);
    if(m_info.auth == BdUserAuthAdmin_E)
        ui->AuthLabel->setText(tr("Admin"));
    else
        ui->AuthLabel->setText(tr("General"));
    ui->DateLabel->setText(m_info.date);
    ui->BriefMessageLabel->setText(m_info.message);
    ui->MessageLabel->setText(m_info.message);
    ui->MessageGroupBox->hide();
    QSize size = m_item->sizeHint();
    size.setHeight(42);
    m_item->setSizeHint(size);
}

CUserItem::~CUserItem()
{
    delete ui;
}

USER_INFO_S CUserItem::info()
{
    return m_info;
}

void CUserItem::on_DetailBtn_clicked()
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
