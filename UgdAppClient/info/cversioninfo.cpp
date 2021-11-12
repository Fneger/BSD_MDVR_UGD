#include "cversioninfo.h"
#include "ui_cversioninfo.h"
#include "QListWidgetItem"

CVersionInfo::CVersionInfo(QListWidgetItem *item,const VERSION_INFO_S &versionInfo, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CVersionInfo),
    m_item(item),
    m_info(versionInfo)
{
    ui->setupUi(this);
    ui->NameLabel->setText(m_info.fileName);
    ui->NameLabel->installEventFilter(this);
    QString fileSize;
    if(m_info.fileSize >= 1024)
    {
        if(m_info.fileSize%1024 > 0)
            fileSize = QString::number(m_info.fileSize/1024 + 1);
        else
            fileSize = QString::number(m_info.fileSize/1024);

        for(int i=fileSize.length() - 3; i>=0; i-=3)
        {
            fileSize.insert(i,',');
        }
        fileSize += " KB";
    }
    else
        fileSize = QString::number(m_info.fileSize);
    ui->FileSizeLabel->setText(fileSize);
    ui->BriefMessageLabel->setText(m_info.message);
    setDefaultVersion(m_info.isDefault);
    ui->MessageLabel->setText(m_info.message);
    ui->MessageLabel->installEventFilter(this);
    ui->MessageGroupBox->hide();
    QSize size = m_item->sizeHint();
    size.setHeight(42);
    m_item->setSizeHint(size);
}

CVersionInfo::~CVersionInfo()
{
    delete ui;
}

bool CVersionInfo::eventFilter(QObject *o, QEvent *e)
{
    if((o == ui->MessageLabel || o == ui->NameLabel) && e->type()==QEvent::MouseButtonPress)
    {
        m_item->setSelected(true);
    }
    return QFrame::eventFilter(o, e);
}

void CVersionInfo::setDefaultVersion(bool isDefault)
{
    m_info.isDefault = isDefault;
    if(m_info.isDefault)
        ui->UsedMarkLabel->setText(tr("(Default Version)"));
    else
        ui->UsedMarkLabel->setText("");
}

void CVersionInfo::on_DetailBtn_clicked()
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
