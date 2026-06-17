#pragma once

#include <vector>

#include <QString>

namespace EduSys {
class BackgroundHostWidget;
}

namespace EduSys::GuiBackground {

QString backgroundFilePath(const QString& roleKey);
bool replaceBackgroundImage(const QString& roleKey, const QString& sourcePath, QString* errorMessage = nullptr);
bool clearBackgroundImage(const QString& roleKey, QString* errorMessage = nullptr);
qreal loadOpacity(const QString& roleKey, qreal fallback = 0.18);
void saveOpacity(const QString& roleKey, qreal opacity);
void applyBackgroundToPage(EduSys::BackgroundHostWidget* page, const QString& roleKey);
void applyBackgroundToPages(const std::vector<EduSys::BackgroundHostWidget*>& pages, const QString& roleKey);

} // namespace EduSys::GuiBackground
