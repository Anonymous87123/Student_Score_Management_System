#include "EduSys/gui/BackgroundHostWidget.hpp"

#include <algorithm>

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>

namespace EduSys {

BackgroundHostWidget::BackgroundHostWidget(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
}

void BackgroundHostWidget::setBackgroundImagePath(const QString& path) {
    backgroundImagePath_ = path;
    reloadPixmap();
    update();
}

void BackgroundHostWidget::clearBackgroundImage() {
    backgroundImagePath_.clear();
    backgroundPixmap_ = QPixmap();
    update();
}

void BackgroundHostWidget::setBackgroundOpacity(qreal opacity) {
    backgroundOpacity_ = std::clamp(opacity, 0.05, 1.0);
    update();
}

QString BackgroundHostWidget::backgroundImagePath() const {
    return backgroundImagePath_;
}

qreal BackgroundHostWidget::backgroundOpacity() const {
    return backgroundOpacity_;
}

void BackgroundHostWidget::reloadPixmap() {
    backgroundPixmap_ = backgroundImagePath_.isEmpty() ? QPixmap() : QPixmap(backgroundImagePath_);
    if (backgroundPixmap_.isNull()) {
        backgroundImagePath_.clear();
    }
}

void BackgroundHostWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(rect(), QColor("#f5f8fc"));

    if (!backgroundPixmap_.isNull()) {
        const QPixmap scaled = backgroundPixmap_.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const QPoint topLeft((width() - scaled.width()) / 2, (height() - scaled.height()) / 2);
        painter.setOpacity(backgroundOpacity_);
        painter.drawPixmap(topLeft, scaled);
    }
}

} // namespace EduSys
