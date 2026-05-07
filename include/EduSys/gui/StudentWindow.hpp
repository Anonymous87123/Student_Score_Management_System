#pragma once

#include <QMainWindow>

#include "EduSys/service/Session.hpp"

namespace EduSys {

class AppContext;

class StudentWindow : public QMainWindow {
public:
    StudentWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    AppContext& appContext_;
    Session     session_;
};

} // namespace EduSys
