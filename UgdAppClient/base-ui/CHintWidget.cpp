#include "CHintWidget.h"
#include <QTimer>
#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QBitmap>
#include <QGridLayout>
#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>
#include <QMutex>


void CHintWidget::ShowHint( QString hint, int time)
{
	QByteArray array1 = hint.toLatin1();
	QByteArray array2 = hint.toLocal8Bit();
	QByteArray array3 = hint.toUtf8();
	QString str = hint.toLocal8Bit();
    m_lblHint->setText(hint);
    m_lblHint->adjustSize();
    this->adjustSize();
    this->show();

    QRect rect = this->geometry();
    int x = (m_ss.width() - rect.width()) / 2;
    int y = (m_ss.height() - rect.height()) / 2;
    this->move(x, y);
	if (0 != time)
	{
		QTimer::singleShot(time * 1000, this, SLOT(HideHint()));
	}
}

void CHintWidget::ShowLoading(int time)
{
	m_lblLoading->show();
	m_lblLoading->resize(120, 120);
	m_lblLoading->move(300, 300);
	m_mveLoading->start();
}

void CHintWidget::HideHint()
{
    m_lblHint->clear();
    this->hide();
}

void CHintWidget::HideLoading()
{
	m_mveLoading->stop();
    m_lblLoading->hide();
}

CHintWidget*CHintWidget::Instance()
{
    static QMutex s_dataMutex;
    static CHintWidget *pInstance = nullptr;
    s_dataMutex.lock();
    if (pInstance == nullptr)
    {
        pInstance = new CHintWidget();
    }
    s_dataMutex.unlock();
    return pInstance;
}

CHintWidget::CHintWidget(QWidget* parent) : QFrame(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Dialog);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setWindowOpacity(0.6);
    setMinimumSize(400,240);
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(0,0,0,0);
	m_MainRect = QApplication::desktop()->availableGeometry(0);
	m_Origin = m_MainRect.topLeft();
	m_ss = QApplication::desktop()->availableGeometry().size();
	m_mveLoading = new QMovie(":/res/Dialog/gif/LODING.gif", "gif", parent);

    QFont f(QString::fromLocal8Bit("微软雅黑"), 32, 32);
    m_lblHint = new QLabel(this);
    m_lblHint->setFont(f);
    m_lblHint->setStyleSheet("background-color:#FF000000;"
                             "color:#FFFFFFFF;"
                             "border-radius:10px;");
    m_lblHint->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    gridLayout->addWidget(m_lblHint);

    m_lblLoading = new QLabel(this);
    gridLayout->addWidget(m_lblLoading);
    m_lblLoading->hide();
	m_lblLoading->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	m_lblLoading->setFocusPolicy(Qt::NoFocus);
	m_lblLoading->setAttribute(Qt::WA_ShowWithoutActivating, true);
	m_lblLoading->setAttribute(Qt::WA_TranslucentBackground, true);
	m_mveLoading->setScaledSize(QSize(120, 120));
	m_lblLoading->setMovie(m_mveLoading);

//	m_HintTimer = new QTimer(parent);
//	m_HintTimer->setSingleShot(true);
//	connect(m_HintTimer, SIGNAL(timeout()), this, SLOT(HideHint()), Qt::QueuedConnection);
    //connect(CEnExLogicPool::instance(), &CEnExLogicPool::s_ShowHint, this, &CHintWidget::ShowHint,Qt::QueuedConnection);
}

CHintWidget::~CHintWidget()
{

}

void CHintWidget::paintEvent(QPaintEvent *)
{
    //画边界圆角
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(),10,10);
    setMask(bmp);
}



