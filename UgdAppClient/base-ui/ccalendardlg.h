#ifndef CCALENDARDLG_H
#define CCALENDARDLG_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
class CCalendarDlg;
}

class CCalendarDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CCalendarDlg(QWidget *parent = nullptr);
    ~CCalendarDlg();
    void setSelectDateTime(const QDateTime &dateTime);
    QDateTime getSelectDateTime();
    static bool getSelectDateTime(QDateTime &dateTime, QWidget *parent = nullptr);

private slots:
    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

private:
    Ui::CCalendarDlg *ui;
};

#endif // CCALENDARDLG_H
