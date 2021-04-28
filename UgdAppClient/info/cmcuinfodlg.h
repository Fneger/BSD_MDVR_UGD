#ifndef CMCUINFODLG_H
#define CMCUINFODLG_H

#include <QDialog>
#include "st_message.h"
using namespace BdUgdServer;
namespace Ui {
class CMcuInfoDlg;
}

class CMcuInfoDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CMcuInfoDlg(const QString sProductName,QWidget *parent = nullptr);
    ~CMcuInfoDlg();
    int clickedType()const { return m_clickedType; }
    MCU_INFO_S mcuInfo();
    static bool getMcuInfo(const QString &sProductName, MCU_INFO_S &mcuInfo, QWidget *parent = nullptr);

private slots:
    void on_CancelBtn_clicked();

    void on_OkBtn_clicked();

private:
    Ui::CMcuInfoDlg *ui;
    int m_clickedType;
    QString m_productName;
};

#endif // CMCUINFODLG_H
