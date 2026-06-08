#pragma once

#include <QDialog>

#include "EduSys/model/Score.hpp"

class QDoubleSpinBox;
class QLineEdit;

namespace EduSys {

class ScoreEditDialog : public QDialog {
public:
    explicit ScoreEditDialog(QWidget* parent = nullptr);
    ScoreEditDialog(const Score& score, QWidget* parent = nullptr);

    Score score() const;
    void setCourseIdLocked(const std::string& courseId);

private:
    void setupUi();
    void loadScore(const Score& score);
    void validateAndAccept();

    QLineEdit*      studentIdEdit_ = nullptr;
    QLineEdit*      courseIdEdit_ = nullptr;
    QLineEdit*      semesterEdit_ = nullptr;
    QDoubleSpinBox* usualSpin_ = nullptr;
    QDoubleSpinBox* finalSpin_ = nullptr;
    QDoubleSpinBox* totalSpin_ = nullptr;
};

} // namespace EduSys
