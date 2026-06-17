#pragma once

#include <vector>

#include <QWidget>

#include "EduSys/model/Score.hpp"

namespace EduSys {

class ScoreBarChartWidget : public QWidget {
public:
    explicit ScoreBarChartWidget(QWidget* parent = nullptr);

    void setScores(std::vector<Score> scores);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<Score> scores_;
};

} // namespace EduSys
