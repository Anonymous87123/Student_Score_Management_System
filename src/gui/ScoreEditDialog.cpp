#include "EduSys/gui/ScoreEditDialog.hpp"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

namespace EduSys {

ScoreEditDialog::ScoreEditDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"录入成绩"));
}

ScoreEditDialog::ScoreEditDialog(const Score& score, QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle(QString::fromUtf8(u8"编辑成绩"));
    loadScore(score);
}

Score ScoreEditDialog::score() const {
    return Score(
        studentIdEdit_->text().trimmed().toStdString(),
        courseIdEdit_->text().trimmed().toStdString(),
        semesterEdit_->text().trimmed().toStdString(),
        usualSpin_->value(),
        finalSpin_->value(),
        totalSpin_->value());
}

void ScoreEditDialog::setupUi() {
    setModal(true);
    setMinimumWidth(420);

    auto* rootLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    studentIdEdit_ = new QLineEdit(this);
    courseIdEdit_ = new QLineEdit(this);
    semesterEdit_ = new QLineEdit(this);
    semesterEdit_->setPlaceholderText(QString::fromUtf8(u8"例如：2025-2026-1"));

    usualSpin_ = new QDoubleSpinBox(this);
    finalSpin_ = new QDoubleSpinBox(this);
    totalSpin_ = new QDoubleSpinBox(this);
    for (QDoubleSpinBox* spin : {usualSpin_, finalSpin_, totalSpin_}) {
        spin->setRange(0.0, 100.0);
        spin->setDecimals(1);
        spin->setSingleStep(0.5);
    }

    formLayout->addRow(QString::fromUtf8(u8"学号"), studentIdEdit_);
    formLayout->addRow(QString::fromUtf8(u8"课程号"), courseIdEdit_);
    formLayout->addRow(QString::fromUtf8(u8"学期"), semesterEdit_);
    formLayout->addRow(QString::fromUtf8(u8"平时成绩"), usualSpin_);
    formLayout->addRow(QString::fromUtf8(u8"期末成绩"), finalSpin_);
    formLayout->addRow(QString::fromUtf8(u8"总评成绩"), totalSpin_);
    rootLayout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(this);
    buttons->addButton(QString::fromUtf8(u8"保存"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QString::fromUtf8(u8"取消"), QDialogButtonBox::RejectRole);
    rootLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] { validateAndAccept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ScoreEditDialog::loadScore(const Score& score) {
    studentIdEdit_->setText(QString::fromStdString(score.getStudentId()));
    courseIdEdit_->setText(QString::fromStdString(score.getCourseId()));
    semesterEdit_->setText(QString::fromStdString(score.getSemester()));
    studentIdEdit_->setEnabled(false);
    courseIdEdit_->setEnabled(false);
    semesterEdit_->setEnabled(false);
    usualSpin_->setValue(score.getUsualScore());
    finalSpin_->setValue(score.getFinalScore());
    totalSpin_->setValue(score.getTotalScore());
}

void ScoreEditDialog::setCourseIdLocked(const std::string& courseId) {
    courseIdEdit_->setText(QString::fromStdString(courseId));
    courseIdEdit_->setEnabled(false);
}

void ScoreEditDialog::validateAndAccept() {
    if (studentIdEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"学号不能为空。"));
        studentIdEdit_->setFocus();
        return;
    }
    if (courseIdEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"课程号不能为空。"));
        courseIdEdit_->setFocus();
        return;
    }
    if (semesterEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"学期不能为空。"));
        semesterEdit_->setFocus();
        return;
    }
    accept();
}

} // namespace EduSys
