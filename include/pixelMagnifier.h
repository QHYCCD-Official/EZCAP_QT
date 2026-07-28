#ifndef PIXELMAGNIFIER_H
#define PIXELMAGNIFIER_H

#include <QWidget>
#include <QImage>
#include <QString>
#include <QStringList>

/**
 * @brief The PixelMagnifierWidget class
 * 浮动放大镜控件：显示鼠标周围小区域的放大图、光标像素值以及区域统计信息
 * (Min/Max/Mean/StdDev)。作为图像显示区 viewport 的子控件浮于图像上方。
 */
class PixelMagnifierWidget : public QWidget
{
public:
    explicit PixelMagnifierWidget(QWidget *parent = 0);

    /**
     * @brief setContent 设置放大镜显示内容
     * @param region      N x N 区域采样图（每像素代表一个图像像素，未放大）
     * @param pixelText   光标处像素信息，例如 "(1024,768)  4096" 或 "R:.. G:.. B:.."
     * @param statLines   区域统计信息行，例如 Min/Max/Mean/Std
     */
    void setContent(const QImage &region, const QString &pixelText, const QStringList &statLines);

    /**
     * @brief placeNear 将放大镜放置到光标附近（自动在边缘处翻转方向）
     * @param cursorPos    光标在 viewport 中的坐标
     * @param viewportSize viewport 大小
     */
    void placeNear(const QPoint &cursorPos, const QSize &viewportSize);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QImage      m_region;      //N x N 区域图
    QString     m_pixelText;   //光标像素值文本
    QStringList m_statLines;   //统计信息文本行
};

#endif // PIXELMAGNIFIER_H
