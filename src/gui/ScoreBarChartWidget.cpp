#include "EduSys/gui/ScoreBarChartWidget.hpp"

#include <algorithm>
#include <utility>

#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPaintEvent>

namespace {

constexpr int kTopMargin = 56;
constexpr int kLeftMargin = 150;
constexpr int kRightMargin = 70;
constexpr int kBottomMargin = 42;
constexpr int kRowHeight = 36;
constexpr int kBarHeight = 18;
constexpr int kMinChartWidth = 360;
constexpr double kMaxScore = 100.0;

QColor colorForScore(double score) {
    if (score >= 90.0) {
        return QColor(42, 157, 143);
    }
    if (score >= 80.0) {
        return QColor(69, 123, 190);
    }
    if (score >= 60.0) {
        return QColor(233, 167, 49);
    }
    return QColor(214, 73, 78);
}

QString chartLabel(const EduSys::Score& score) {
    return QString::fromStdString(score.getCourseId() + " / " + score.getSemester());
}

} // namespace

namespace EduSys {

ScoreBarChartWidget::ScoreBarChartWidget(QWidget* parent)
    : QWidget(parent) {
    setAutoFillBackground(false);
}

void ScoreBarChartWidget::setScores(std::vector<Score> scores) {
    std::sort(scores.begin(), scores.end(), [](const Score& lhs, const Score& rhs) {
        if (lhs.getSemester() != rhs.getSemester()) {
            return lhs.getSemester() < rhs.getSemester();
        }
        return lhs.getCourseId() < rhs.getCourseId();
    });

    scores_ = std::move(scores);
    updateGeometry();
    update();
}

QSize ScoreBarChartWidget::sizeHint() const {
    const int chartHeight = kTopMargin + kBottomMargin
        + std::max(6, static_cast<int>(scores_.size())) * kRowHeight;
    return QSize(760, chartHeight);
}

QSize ScoreBarChartWidget::minimumSizeHint() const {
    return QSize(520, 320);
}

void ScoreBarChartWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient background(rect().topLeft(), rect().bottomRight());
    background.setColorAt(0.0, QColor(248, 252, 255));
    background.setColorAt(1.0, QColor(255, 250, 245));
    painter.fillRect(rect(), background);

    painter.setPen(QColor(30, 41, 59));
    QFont titleFont = painter.font();
    titleFont.setPointSize(titleFont.pointSize() + 3);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(20, 16, width() - 40, 26),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString::fromUtf8(u8"我的成绩总评条形图"));

    QFont normalFont = painter.font();
    normalFont.setPointSize(normalFont.pointSize() - 3);
    normalFont.setBold(false);
    painter.setFont(normalFont);

    if (scores_.empty()) {
        painter.setPen(QColor(100, 116, 139));
        painter.drawText(rect(), Qt::AlignCenter, QString::fromUtf8(u8"暂无成绩记录"));
        return;
    }

    const int chartLeft = kLeftMargin;
    const int chartTop = kTopMargin;
    const int chartWidth = std::max(kMinChartWidth, width() - kLeftMargin - kRightMargin);
    const int chartHeight = static_cast<int>(scores_.size()) * kRowHeight;

    painter.setPen(QPen(QColor(203, 213, 225), 1));
    for (int tick = 0; tick <= 100; tick += 20) {
        const int x = chartLeft + static_cast<int>(chartWidth * tick / kMaxScore);
        painter.drawLine(x, chartTop - 4, x, chartTop + chartHeight);
        painter.setPen(QColor(100, 116, 139));
        painter.drawText(QRect(x - 18, chartTop + chartHeight + 8, 36, 18),
                         Qt::AlignCenter,
                         QString::number(tick));
        painter.setPen(QPen(QColor(203, 213, 225), 1));
    }

    painter.setPen(QColor(71, 85, 105));
    painter.drawText(QRect(chartLeft, chartTop + chartHeight + 24, chartWidth, 18),
                     Qt::AlignCenter,
                     QString::fromUtf8(u8"总评分数"));

    const QFontMetrics metrics(painter.font());
    for (int row = 0; row < static_cast<int>(scores_.size()); ++row) {
        const Score& score = scores_[static_cast<std::size_t>(row)];
        const int y = chartTop + row * kRowHeight;
        const double total = std::clamp(score.getTotalScore(), 0.0, kMaxScore);
        const int barWidth = static_cast<int>(chartWidth * total / kMaxScore);

        painter.setPen(QColor(51, 65, 85));
        const QString label = metrics.elidedText(chartLabel(score), Qt::ElideRight, kLeftMargin - 24);
        painter.drawText(QRect(12, y, kLeftMargin - 24, kRowHeight),
                         Qt::AlignRight | Qt::AlignVCenter,
                         label);

        const QRect barRect(chartLeft, y + (kRowHeight - kBarHeight) / 2, barWidth, kBarHeight);
        painter.setPen(Qt::NoPen);
        painter.setBrush(colorForScore(total));
        painter.drawRoundedRect(barRect, 8, 8);

        painter.setPen(QColor(15, 23, 42));
        painter.drawText(QRect(chartLeft + barWidth + 8, y, kRightMargin - 12, kRowHeight),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QString::number(score.getTotalScore(), 'f', 1));
    }
}

} // namespace EduSys
