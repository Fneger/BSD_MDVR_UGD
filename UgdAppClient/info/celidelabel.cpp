#include "celidelabel.h"
#include <QDebug>


CElideLabel::CElideLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
}

CElideLabel::CElideLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f), m_fullText(text)
{
    this->setMinimumWidth(0);
    setTextFormat(Qt::PlainText);
}

void CElideLabel::setText(const QString &text)
{
    setFullText(text);
}

void CElideLabel::setFullText(const QString &text)
{
    m_fullText = text;
    update();
}

void CElideLabel::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void CElideLabel::setTextLimitExpand(const QString &text)
{
    int textWidth = this->fontMetrics().width(text);
    this->setMaximumWidth(textWidth);
    setFullText(text);
}

QString CElideLabel::fullText() const
{
    return m_fullText;
}

void CElideLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    elideText();
}

void CElideLabel::elideText()
{
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) - this->width();
    if (dif > 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width());
        QLabel::setText(showText);
        if (showText != m_fullText) {
            this->setToolTip(m_fullText.left(1024));
        } else {
            this->setToolTip("");
        }
    } else {
        QLabel::setText(m_fullText);
        this->setToolTip("");
    }
}
