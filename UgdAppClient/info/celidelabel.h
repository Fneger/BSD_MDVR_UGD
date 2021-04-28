#ifndef CELIDELABEL_H
#define CELIDELABEL_H

#include <QLabel>

class CElideLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CElideLabel(QWidget *parent=0, Qt::WindowFlags f=0);
    explicit CElideLabel(const QString &text, QWidget *parent=0, Qt::WindowFlags f=0);

    void setText(const QString &text);
    void setFullText(const QString &text);
    void setTextLimitShrink(const QString &text, int width);
    void setTextLimitExpand(const QString &text);
    QString fullText() const;

protected:
    void paintEvent(QPaintEvent *);

private:
    void elideText();

private:
    QString m_fullText;
};

#endif // CELIDELABEL_H
