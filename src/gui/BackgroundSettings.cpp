#include "EduSys/gui/BackgroundSettings.hpp"

#include "EduSys/gui/BackgroundHostWidget.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

namespace EduSys::GuiBackground {

namespace {

QString backgroundDirPath() {
    return QStringLiteral("data/gui_backgrounds");
}

QString settingsFilePath() {
    return QDir(backgroundDirPath()).filePath(QStringLiteral("backgrounds.ini"));
}

QString opacityKey(const QString& roleKey) {
    return QStringLiteral("gui/background/%1/opacity").arg(roleKey);
}

QString targetFilePath(const QString& roleKey) {
    return QDir(backgroundDirPath()).filePath(roleKey + QStringLiteral(".png"));
}

bool ensureBackgroundDir(QString* errorMessage) {
    QDir dir;
    if (dir.mkpath(backgroundDirPath())) {
        return true;
    }
    if (errorMessage) {
        *errorMessage = QStringLiteral("无法创建背景图片目录：%1").arg(backgroundDirPath());
    }
    return false;
}

} // namespace

QString backgroundFilePath(const QString& roleKey) {
    return targetFilePath(roleKey);
}

bool replaceBackgroundImage(const QString& roleKey, const QString& sourcePath, QString* errorMessage) {
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("选择的图片文件不存在。");
        }
        return false;
    }

    if (!ensureBackgroundDir(errorMessage)) {
        return false;
    }

    const QString targetPath = targetFilePath(roleKey);
    const QFileInfo targetInfo(targetPath);
    if (sourceInfo.absoluteFilePath() == targetInfo.absoluteFilePath()) {
        return true;
    }

    QFile::remove(targetPath);
    if (!QFile::copy(sourceInfo.absoluteFilePath(), targetPath)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("复制背景图片失败，请检查文件权限或磁盘空间。");
        }
        return false;
    }
    return true;
}

bool clearBackgroundImage(const QString& roleKey, QString* errorMessage) {
    const QString targetPath = targetFilePath(roleKey);
    if (QFile::exists(targetPath) && !QFile::remove(targetPath)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("删除背景图片失败，请检查文件权限。");
        }
        return false;
    }
    return true;
}

qreal loadOpacity(const QString& roleKey, qreal fallback) {
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    return settings.value(opacityKey(roleKey), fallback).toDouble();
}

void saveOpacity(const QString& roleKey, qreal opacity) {
    ensureBackgroundDir(nullptr);
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    settings.setValue(opacityKey(roleKey), opacity);
    settings.sync();
}

void applyBackgroundToPage(BackgroundHostWidget* page, const QString& roleKey) {
    if (!page) {
        return;
    }

    page->setBackgroundImagePath(backgroundFilePath(roleKey));
    page->setBackgroundOpacity(loadOpacity(roleKey));
}

void applyBackgroundToPages(const std::vector<BackgroundHostWidget*>& pages, const QString& roleKey) {
    for (auto* page : pages) {
        applyBackgroundToPage(page, roleKey);
    }
}

} // namespace EduSys::GuiBackground
