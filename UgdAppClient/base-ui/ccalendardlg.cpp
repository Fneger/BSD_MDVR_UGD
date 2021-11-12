#include "ccalendardlg.h"
#include "ui_ccalendardlg.h"

CCalendarDlg::CCalendarDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CCalendarDlg)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Date Time Edit"));
    QLocale local = QLocale::system();
    QLocale::Language lang = local.language();
    ui->CalendarWidget->setLocale(lang);
}

CCalendarDlg::~CCalendarDlg()
{
    delete ui;
}

void CCalendarDlg::setSelectDateTime(const QDateTime &dateTime)
{
    ui->CalendarWidget->setSelectedDate(dateTime.date());
    ui->TimeEdit->setTime(dateTime.time());
}

QDateTime CCalendarDlg::getSelectDateTime()
{
    QDateTime dt;
    dt.setDate(ui->CalendarWidget->selectedDate());
    dt.setTime(ui->TimeEdit->time());
    return dt;
}

void CCalendarDlg::on_CancelBtn_clicked()
{
    emit reject();
    close();
}

void CCalendarDlg::on_OkBtn_clicked()
{
    emit accept();
    close();
}

bool CCalendarDlg::getSelectDateTime(QDateTime &dateTime, QWidget *parent)
{
    bool res = false;
    CCalendarDlg dlg(parent);
    dlg.setSelectDateTime(dateTime);
    if(dlg.exec() == QDialog::Accepted)
    {
        dateTime = dlg.getSelectDateTime();
        res = true;
    }
    return res;
}
