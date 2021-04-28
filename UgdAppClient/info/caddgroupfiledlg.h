#ifndef CADDGROUPFILEDLG_H
#define CADDGROUPFILEDLG_H

#include <QDialog>
#include "st_message.h"

using namespace BdUgdServer;

namespace Ui {
class CAddGroupFileDlg;
}

class CAddGroupFileDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CAddGroupFileDlg(QWidget *parent = nullptr);
    ~CAddGroupFileDlg();
    GROUP_FILE_INFO_S info();
    static bool addNewGroupFile(GROUP_FILE_INFO_S &info,QWidget *parent = nullptr);

private slots:
    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

private:
    Ui::CAddGroupFileDlg *ui;
    GROUP_FILE_INFO_S m_info;
};

#endif // CADDGROUPFILEDLG_H
