#include "pixelMagnifier.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>

namespace {
const int kCellSize   = 13;                 //每个图像像素放大后的显示尺寸(px)
const int kMargin     = 8;                  //内边距
const int kTextLineH  = 17;                 //文本行高
const int kGap        = 6;                  //放大图与文本区间距
const int kOffsetX    = 18;                 //相对光标的默认偏移
const int kOffsetY    = 18;
}

PixelMagnifierWidget::PixelMagnifierWidget(QWidget *parent)
    : QWidget(parent)
{
    //不响应鼠标事件，事件穿透到下层图像label，避免干扰悬停跟踪
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setFocusPolicy(Qt::NoFocus);
    hide();
}

void PixelMagnifierWidget::setContent(const QImage &region, const QString &pixelText, const QStringList &statLines)
{
    m_region    = region;
    m_pixelText = pixelText;
    m_statLines = statLines;

    int regionSide = qMax(1, m_region.width()) * kCellSize;
    int textLines  = 1 + m_statLines.size();//像素值行 + 统计行
    int w = regionSide + kMargin * 2;
    int h = kMargin + regionSide + kGap + textLines * kTextLineH + kMargin;

    if(size() != QSize(w, h))
        setFixedSize(w, h);

    update();
}

void PixelMagnifierWidget::placeNear(const QPoint &cursorPos, const QSize &viewportSize)
{
    int x = cursorPos.x() + kOffsetX;
    int y = cursorPos.y() + kOffsetY;

    //靠近右/下边缘时翻转到光标另一侧
    if(x + width() > viewportSize.width())
        x = cursorPos.x() - kOffsetX - width();
    if(y + height() > viewportSize.height())
        y = cursorPos.y() - kOffsetY - height();

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    move(x, y);
}

void PixelMagnifierWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if(m_region.isNull())
        return;

    QPainter p(this);

    //背景
    p.fillRect(rect(), QColor(25, 25, 28, 235));
    p.setPen(QPen(QColor(90, 90, 96), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    const int n          = m_region.width();//区域边长（图像像素数）
    const int regionSide = n * kCellSize;
    const QRect imgRect(kMargin, kMargin, regionSide, regionSide);

    //区域放大图（最近邻，保留像素块感）
    p.drawImage(imgRect, m_region);

    //像素质感网格线
    p.setPen(QPen(QColor(70, 70, 76, 120), 1));
    for(int i = 1; i < n; ++i)
    {
        p.drawLine(imgRect.left() + i * kCellSize, imgRect.top(),
                   imgRect.left() + i * kCellSize, imgRect.bottom());
        p.drawLine(imgRect.left(), imgRect.top() + i * kCellSize,
                   imgRect.right(), imgRect.top() + i * kCellSize);
    }

    //中心像素（光标所在像素）高亮框
    const int c = n / 2;
    const QRect centerCell(imgRect.left() + c * kCellSize,
                           imgRect.top()  + c * kCellSize,
                           kCellSize, kCellSize);
    p.setPen(QPen(QColor(255, 80, 80), 1));
    p.drawRect(centerCell.adjusted(0, 0, -1, -1));

    //十字线
    p.setPen(QPen(QColor(255, 80, 80, 180), 1));
    int cx = centerCell.x() + kCellSize / 2;
    int cy = centerCell.y() + kCellSize / 2;
    p.drawLine(imgRect.left(), cy, centerCell.left(), cy);
    p.drawLine(centerCell.right() + 1, cy, imgRect.right(), cy);
    p.drawLine(cx, imgRect.top(), cx, centerCell.top());
    p.drawLine(cx, centerCell.bottom() + 1, cx, imgRect.bottom());

    //放大图边框
    p.setPen(QPen(QColor(120, 120, 128), 1));
    p.drawRect(imgRect.adjusted(0, 0, -1, -1));

    //文本信息
    QFont f = p.font();
    f.setPointSize(8);
    p.setFont(f);
    p.setPen(QColor(230, 230, 230));

    int ty = imgRect.bottom() + kGap + kTextLineH - 4;
    p.drawText(kMargin, ty, m_pixelText);
    ty += kTextLineH;
    for(int i = 0; i < m_statLines.size(); ++i)
    {
        p.drawText(kMargin, ty, m_statLines.at(i));
        ty += kTextLineH;
    }
}
