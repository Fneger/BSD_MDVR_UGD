#ifndef CADDVERSINDLG_H
#define CADDVERSINDLG_H

#include <QDialog>
#include "st_message.h"

namespace Ui {
class CAddVersinDlg;
}

using namespace BdUgdServer;
class CAddVersinDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CAddVersinDlg(QWidget *parent = nullptr);
    ~CAddVersinDlg();
    VERSION_INFO_S versionInfo();
    static bool addNewVersion(VERSION_INFO_S &versionInfo,QWidget *parent = nullptr);

private slots:
    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

    void on_SelectedFilePath_clicked();

private:
    Ui::CAddVersinDlg *ui;
    VERSION_INFO_S m_versionInfo;
};

#endif // CADDVERSINDLG_H
