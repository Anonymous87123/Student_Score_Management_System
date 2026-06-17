#pragma once

#include <QString>
#include <QPixmap>
#include <QWidget>

class QPaintEvent;

namespace EduSys {

class BackgroundHostWidget : public QWidget {
public:
    explicit BackgroundHostWidget(QWidget* parent = nullptr);

    void setBackgroundImagePath(const QString& path);
    void clearBackgroundImage();
    void setBackgroundOpacity(qreal opacity);

    QString backgroundImagePath() const;
    qreal backgroundOpacity() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void reloadPixmap();

    QString backgroundImagePath_;
    QPixmap backgroundPixmap_;
    qreal backgroundOpacity_ = 0.18;
};

} // namespace EduSys
