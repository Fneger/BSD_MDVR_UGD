#ifndef CHINTWIDGET_H
#define CHINTWIDGET_H

#include <QFrame>
#include <QMovie>

class QLabel;
class CHintWidget : public QFrame
{
    Q_OBJECT
public:
	void ShowLoading(int time = 0);
    static CHintWidget* Instance();
    void paintEvent(QPaintEvent *);

public slots:
	void HideHint();
	void HideLoading();
	void ShowHint(QString hint, int time = 0);


private:
	explicit CHintWidget(QWidget *parent = nullptr);
	virtual ~CHintWidget();
	QLabel* m_lblHint;
	QLabel* m_lblLoading;
	QMovie* m_mveLoading;
//	QTimer* m_HintTimer;
	QSize m_ss;
	QPoint m_Origin;
    QRect m_MainRect;

signals:

public slots:
};

#endif // CHINTWIDGET_H
