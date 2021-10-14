/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *txtData;
    QPushButton *pushSent;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *txtIp;
    QLabel *label_3;
    QLineEdit *txtPort;
    QPushButton *pushConnect;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QLabel *label_4;
    QLineEdit *lineEdit;
    QPushButton *timeBut;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(431, 466);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));

        verticalLayout->addWidget(textEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        txtData = new QLineEdit(centralWidget);
        txtData->setObjectName(QString::fromUtf8("txtData"));

        horizontalLayout->addWidget(txtData);

        pushSent = new QPushButton(centralWidget);
        pushSent->setObjectName(QString::fromUtf8("pushSent"));

        horizontalLayout->addWidget(pushSent);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        txtIp = new QLineEdit(centralWidget);
        txtIp->setObjectName(QString::fromUtf8("txtIp"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(txtIp->sizePolicy().hasHeightForWidth());
        txtIp->setSizePolicy(sizePolicy);
        txtIp->setEchoMode(QLineEdit::Normal);

        horizontalLayout_2->addWidget(txtIp);

        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        txtPort = new QLineEdit(centralWidget);
        txtPort->setObjectName(QString::fromUtf8("txtPort"));
        QSizePolicy sizePolicy1(QSizePolicy::Ignored, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(txtPort->sizePolicy().hasHeightForWidth());
        txtPort->setSizePolicy(sizePolicy1);
        txtPort->setMinimumSize(QSize(60, 0));
        txtPort->setMaximumSize(QSize(60, 16777215));

        horizontalLayout_2->addWidget(txtPort);

        pushConnect = new QPushButton(centralWidget);
        pushConnect->setObjectName(QString::fromUtf8("pushConnect"));

        horizontalLayout_2->addWidget(pushConnect);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalSpacer = new QSpacerItem(188, 18, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_3->addWidget(label_4);

        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(50);
        sizePolicy2.setVerticalStretch(20);
        sizePolicy2.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
        lineEdit->setSizePolicy(sizePolicy2);
        lineEdit->setMinimumSize(QSize(50, 0));
        lineEdit->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_3->addWidget(lineEdit);

        timeBut = new QPushButton(centralWidget);
        timeBut->setObjectName(QString::fromUtf8("timeBut"));

        horizontalLayout_3->addWidget(timeBut);


        verticalLayout->addLayout(horizontalLayout_3);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QApplication::translate("MainWindow", "\345\217\221\351\200\201\346\225\260\346\215\256\357\274\232", nullptr));
        pushSent->setText(QApplication::translate("MainWindow", "\345\217\221\351\200\201\346\225\260\346\215\256", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "IP\345\234\260\345\235\200\357\274\232", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "\347\253\257\345\217\243\357\274\232", nullptr));
        pushConnect->setText(QApplication::translate("MainWindow", "\350\277\236\346\216\245", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "\351\227\264\351\232\224\346\227\266\351\227\264(s):", nullptr));
        timeBut->setText(QApplication::translate("MainWindow", "\345\220\257\345\212\250\345\256\232\346\227\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
