#pragma once

#include <QMainWindow>

#include "EduSys/service/Session.hpp"

namespace EduSys {

class AppContext;

class TeacherWindow : public QMainWindow {
public:
    TeacherWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    AppContext& appContext_;
    Session     session_;
};

} // namespace EduSys
