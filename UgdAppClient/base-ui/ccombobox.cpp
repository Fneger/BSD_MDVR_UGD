#include "ccombobox.h"
#include <QLineEdit>

CComboBox::CComboBox(QWidget *parent) : QComboBox(parent)
{
    QLineEdit *l = this->lineEdit();
    l->setAlignment(Qt::AlignCenter);
}
