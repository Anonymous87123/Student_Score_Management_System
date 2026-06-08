# EduSys - 学生成绩管理系统

EduSys 是一个基于 `C++17` 的学生成绩管理系统。仓库里现在有两条入口，需要先区分清楚：

- `edusys.exe`：CLI 入口，装配逻辑在 `src/app/main.cpp`
- `edusys_gui.exe`：Qt Widgets 入口，装配逻辑在 `src/app/gui_main.cpp`

两条入口共享同一套 `AppContext`、`service`、`model`、`storage` 和 `report`。GUI 复用同一套业务逻辑，只替换界面层。

当前仓库已经完成的内容：

- 三类角色登录与权限控制：`Admin`、`Teacher`、`Student`
- 学生、课程、成绩三大核心对象的 CRUD（新增、查询、修改、删除）与查询
- GPA、课程统计、课程排名、学业预警
- `.dat` 二进制持久化与显式序列化
- `warning_report.txt` 文本报告与课程 CSV 导出
- `--self-test` 自检入口与 `tools/corrupt_check.bat` 文件损坏脚本
- `introduceQt` 分支的 Qt Widgets GUI
- 架构图册、答辩问答稿、测试用例文档、CLI 演示教程

这份 `README.md` 不只写运行命令，也会作为项目说明和代码解读文档使用。下面会先分清入口，再按功能链和目录树逐级展开，把主要文件的具体内容、职责边界、使用时机都讲清楚。

## 1. 快速开始

### 1.0 先解释几个常见词

建议先了解下面这些术语，再阅读后续运行说明和代码说明：

| 词 | 在本项目里的意思 |
| --- | --- |
| CLI | 控制台版本，也就是运行 `edusys.exe` 后在终端窗口里输入菜单编号、学号、课程号。 |
| GUI | 图形界面版本，也就是 `edusys_gui.exe` 弹出的 Qt 窗口。 |
| Qt Widgets | Qt 的传统桌面控件方案，按钮、输入框、表格、标签页都属于 Widgets。 |
| CMake | 生成工程和构建规则的工具。Qt 分支用它生成 `edusys` 和 `edusys_gui` 两个目标。 |
| `AppContext` | 应用装配对象。可以把它理解成“把仓储、服务、报表导出器统一准备好的一张工作台”。CLI 和 GUI 都从这里拿同一批服务对象。 |
| `Session` | 当前登录状态。里面记录“当前是谁、是什么角色、对应哪个学生或教师编号”。权限判断主要看它。 |
| Service / 服务层 | 写业务规则的地方，例如谁能删学生、教师能不能改某门课的成绩、分数范围是否合法。 |
| Repository / 仓储 | 负责从 `data/*.dat` 读出对象、再把对象写回文件的类。它不决定业务规则，只负责存取。 |
| Model / 实体 | 学生、教师、课程、成绩、账号这些数据对象本身。 |
| ReportExporter | 把统计结果写成 `warning_report.txt` 或 CSV 文件的导出器。 |
| seed | 第一次运行且数据为空时，系统自动写入默认账号和样例数据。 |
| upsert | “有就更新，没有就新增”。成绩录入用这个策略，避免同一学生同一课程同一学期重复多条记录。 |
| `.dat` | 本项目自己的二进制数据文件，不是普通文本，不建议手工打开修改。 |
| 显式序列化 | 每个对象自己规定字段按什么顺序写入文件、再按什么顺序读回来。 |
| 级联删除 | 删除一个对象时，顺手清掉和它相关的其他记录。例如删学生时，也删除该学生成绩和学生账号。 |
| 白名单 | 只允许出现在清单里的对象被操作。教师端的“我的课程”就是白名单。 |
| `--self-test` | 程序内置的自动回归检查入口，不进入菜单，直接验证一批边界。 |

### 1.1 基础操作流程

本节提供基础运行步骤。后续章节用于功能说明、代码解读和答辩准备，可按需阅读。

打开终端，进入项目根目录：

```cmd
cd /d D:\Student_Score_Management_System
```

PowerShell 可以使用：

```powershell
cd D:\Student_Score_Management_System
```

进入目录后，确认当前目录为项目根目录：

```cmd
dir README.md
dir CMakeLists.txt
dir src
dir include
```

如果能列出这些文件或目录，说明当前目录正确。后面运行 `edusys.exe`、`edusys_gui.exe` 时，也建议都在项目根目录执行，因为程序里的 `data/...` 是按当前运行目录来找的。

CLI 快速命令如下：

```cmd
build.bat
edusys.exe --self-test
edusys.exe
tools\corrupt_check.bat
```

命令含义如下：

- `build.bat`：用 `g++` 在仓库根目录构建 `edusys.exe`
- `edusys.exe --self-test`：执行 Week 11 + Week 13 A-E 组内建自检
- `edusys.exe`：进入正常登录与菜单交互，用户名留空直接退出，连续 3 次认证失败后退出
- `tools\corrupt_check.bat`：执行 Week 13 F 组文件损坏测试并自动恢复

这里的 Week 11 / Week 13 是课程开发阶段编号，不是日历周。A-E / F 是测试分组：A-E 主要覆盖认证、字段非法、重复主键、级联删除、教师权限；F 专门覆盖 `.dat` 文件损坏。`corrupt_check.bat` 会临时制造损坏文件并从备份恢复，运行时不要手动中断脚本。

首次操作建议按下面顺序执行：

1. 执行 `build.bat`。
2. 如果编译成功，根目录下会生成或更新 `edusys.exe`。如果出现编译错误，应先处理错误，再继续执行后续命令。
3. 执行 `edusys.exe --self-test`。
4. 如果看到 `Week 11 : self-check PASSED` 和 `Week 13 : boundary-check PASSED`，说明核心功能和边界检查正常。
5. 执行 `edusys.exe` 进入手工菜单。
6. 登录时可使用 `admin / admin123`，进入管理员菜单后按菜单编号操作。
7. 退出当前账号时，在菜单里输入 `0`；关闭整个 CLI 程序时，回到登录界面后用户名留空并回车。
8. `tools\corrupt_check.bat` 是文件损坏测试，适合自检通过后执行；课堂手工演示功能时不一定要先执行它。

如果目标是演示 CLI 端功能，建议优先使用 [`docs/cli-demo-guide.md`](docs/cli-demo-guide.md)。该文档按“输入内容、预计输出、演示顺序”组织，更适合课堂现场演示。

### 1.2 CLI 参数约定

当前 CLI 只认两种启动方式：

- 无参数：进入交互式登录和菜单
- `--self-test`：进入 Week 11 + Week 13 自检

除此之外的其他参数都会被当成非法参数直接报错退出。

### 1.3 Qt 入口

`introduceQt` 分支下还有一条独立 GUI 入口，构建命令由 Qt / CMake 决定，核心是生成 `edusys_gui.exe`。阅读这一段前，先确认自己确实在 Qt 分支；如果只是演示或验证 CLI，`edusys.exe` 仍然可以单独运行，不需要先启动 GUI。

下面命令里的 `-DCMAKE_PREFIX_PATH=...` 要填 Qt 安装目录，不是本项目目录，也不是 `build-qt-introduceQt` 目录。例如你的 Qt 安装在 `D:\Qt\6.11.0\mingw_64`，这里就填这个路径。

如果已经安装 Qt，并且路径类似 `D:\Qt\6.11.0\mingw_64`，可以按下面顺序操作：

1. 进入项目根目录。
2. 输入 `git branch --show-current`，确认当前分支是不是 `introduceQt`。
3. 如果不是 `introduceQt`，输入 `git switch introduceQt`。
4. 确认 Qt 的 CMake、Ninja、MinGW 路径都存在，例如 `D:\Qt\Tools\CMake_64\bin\cmake.exe`、`D:\Qt\Tools\Ninja\ninja.exe`、`D:\Qt\Tools\mingw1310_64\bin\g++.exe`。
5. 运行 CMake 配置命令，生成 `build-qt-introduceQt` 构建目录。
6. 运行 Ninja 编译命令，生成 `edusys.exe` 和 `edusys_gui.exe`。
7. 运行 `build-qt-introduceQt\edusys_gui.exe`。
8. 如果弹出登录窗口，说明 GUI 已经启动成功。

```cmd
cmake -S . -B build-qt-introduceQt -G Ninja -DCMAKE_PREFIX_PATH=...
cmake --build build-qt-introduceQt --target edusys edusys_gui
build-qt-introduceQt\edusys_gui.exe
```

说明：

- 先用 `git branch --show-current` 确认当前分支；如果不在 Qt 分支，可用 `git switch introduceQt` 切过去。
- `build.bat` 只负责 CLI，不负责 Qt；想运行 GUI 不要只执行 `build.bat`。
- `edusys_gui.exe` 的启动入口是 `src/app/gui_main.cpp`，不会复用 CLI 的 `src/app/main.cpp`。
- 两条入口共享 `AppContext`、服务层、仓储层和报表层。`AppContext` 是 CLI 和 GUI 共用的运行时装配入口。

如果使用当前机器上已经验证过的 Qt MinGW 路径，可以使用下面的完整命令：

```cmd
D:\Qt\Tools\CMake_64\bin\cmake.exe -S . -B build-qt-introduceQt -G Ninja -DCMAKE_PREFIX_PATH=D:\Qt\6.11.0\mingw_64 -DCMAKE_MAKE_PROGRAM=D:\Qt\Tools\Ninja\ninja.exe -DCMAKE_CXX_COMPILER=D:\Qt\Tools\mingw1310_64\bin\g++.exe
D:\Qt\Tools\Ninja\ninja.exe -C build-qt-introduceQt edusys edusys_gui
build-qt-introduceQt\edusys_gui.exe
```

三条命令分别对应：

- 第一条：配置工程。正常情况下会看到 `Configuring done`、`Generating done`、`Build files have been written to: ...`。
- 第二条：编译程序。正常情况下会看到 Ninja 编译进度，最后没有错误输出。
- 第三条：启动 GUI。正常情况下会弹出登录框。

登录测试可使用默认账号：

| 用户名 | 密码 | 打开的窗口 |
| --- | --- | --- |
| `admin` | `admin123` | 管理员端 |
| `t001` | `t001pw` | 教师端 |
| `s001` | `s001pw` | 学生端 |

GUI 退出方式：在角色窗口点击“退出登录”会回到登录框；在登录框点击“退出”，或者用户名留空后确认退出，会关闭整个 GUI。

### 1.4 两种构建方式

#### 方式 A：CMake / MSVC

```bash
cmake -S . -B build
cmake --build build --config Release
```

说明：

- 这条路径适合 Visual Studio / CMake 用户
- 产物通常位于 `build/Release/edusys.exe` 或 CMake 指定输出目录
- `build/` 目录里的内容大多数是自动生成文件，不是手写业务代码

#### 方式 B：`g++` 备用脚本

```bash
build.bat
```

说明：

- 这条路径适合课程环境中只装了 MinGW / g++ 的情况
- `build.bat` 会编译 CLI 需要的 `src/app/main.cpp`、公共层、模型层、报表层、服务层、存储层和 `src/view/*.cpp`，不会编译 `src/app/gui_main.cpp` 或 `src/gui/*.cpp`
- `tools/corrupt_check.bat` 默认就假定你运行的是根目录下这个 `edusys.exe`

### 1.5 先分清两条入口

CLI 和 GUI 是两个不同的可执行程序，不是同一个程序里的两个模式。运行 `edusys.exe` 后不会出现“切换到 GUI”的菜单；运行 `edusys_gui.exe` 后也不会进入 CLI 菜单。

如果你现在是来做课程演示，默认看 CLI 这组内容：

- `edusys.exe`
- `docs/cli-demo-guide.md`
- `demo_input.txt`

如果你的目标是 `introduceQt` 分支的桌面窗口版，优先看 GUI 这组内容：

- `edusys_gui.exe`
- `src/app/gui_main.cpp`
- `src/gui/*`

### 1.6 退出方式补充

CLI 里有两种退出：

- 在角色菜单中输入 `0. Logout`，只是退出当前角色菜单，程序会回到登录界面，不会直接关闭程序。
- 如果要彻底关闭 CLI 程序，要在登录界面把用户名留空。

Qt GUI 里也有两种退出：

- 在角色主窗口点“退出登录”，会关闭当前 `AdminWindow` / `TeacherWindow` / `StudentWindow`，然后回到 `LoginDialog`。
- 在登录框里点“退出”，或者用户名留空后确认退出，整个 GUI 程序才会结束。

这个行为由 `src/app/gui_main.cpp` 的登录循环负责，不属于 CLI 菜单逻辑。

## 2. 系统功能概览

### 2.1 三类角色

| 角色 | 能做什么 | 不能做什么 |
| --- | --- | --- |
| `Admin` | 学生/课程/成绩的完整 CRUD（Create/Read/Update/Delete，即新增、查询、修改、删除）、统计、预警报告、CSV 导出、修改自己的密码 | 无业务级限制 |
| `Teacher` | 只查看并维护自己授课课程的成绩、查看自己课程的统计、修改自己的密码 | 不能改学生与课程基础数据，不能动别人的课程 |
| `Student` | 只查看自己的资料、成绩、GPA，修改自己的密码 | 不能写成绩，不能看别人数据 |

### 2.2 CLI 菜单与 Qt 窗口总览

CLI 和 Qt GUI 是两条并列入口。CLI 入口是 `edusys.exe`，用户通过键盘输入菜单编号；Qt 入口是 `edusys_gui.exe`，用户通过登录框、页签、表格、按钮和弹窗操作。界面代码不同，业务服务代码相同，数据文件路径规则相同。也就是说，CLI 和 GUI 不是互相调用，而是分别把用户操作交给同一套服务层处理。

这里说“功能等价”，主要指学生、课程、成绩、统计、报告导出、密码修改这些业务功能等价；`--self-test` 和 `tools/corrupt_check.bat` 仍然是 CLI/脚本验证入口，不做成 GUI 菜单项。

先看 CLI 的菜单结构：

```text
Login loop
  -> username / password
  -> username empty = quit
  -> 3 failed attempts = exit
  -> AuthService.authenticate()

Admin Menu
  1. Student management
  2. Course management
  3. Score management
  4. Statistics
  5. Generate warning report
  6. Change my password
  7. Export course CSV (stats + ranking)
  0. Logout

Teacher Menu
  1. List my courses
  2. View scores for my course
  3. Record / update one score
  4. Delete one score
  5. Course statistics (my course only)
  6. Change my password
  0. Logout

Student Menu
  1. View my profile
  2. View my scores
  3. View my GPA
  4. Change my password
  0. Logout
```

再看 Qt GUI 的窗口结构：

```text
GUI startup
  -> QApplication
  -> AppContext.initializeData()
  -> LoginDialog
       username empty / click Exit = quit application
       3 failed attempts = quit application
       AuthService.authenticate()
  -> createRoleWindow(session)

AdminWindow
  Tab 1. 学生管理
  Tab 2. 课程管理
  Tab 3. 成绩管理
  Tab 4. 统计分析
  Tab 5. 报告导出
  Tab 6. 账户

TeacherWindow
  Tab 1. 我的课程
  Tab 2. 我的课程成绩
  Tab 3. 我的课程统计
  Tab 4. 账户

StudentWindow
  Tab 1. 我的资料
  Tab 2. 我的成绩
  Tab 3. 我的 GPA
  Tab 4. 账户
```

这一段要分清：CLI 的“菜单编号”只属于 `src/view/`；Qt 的“页签和按钮”只属于 `src/gui/`。它们不是互相调用的关系，而是并列调用 `AppContext` 里装配好的服务对象。`AppContext` 负责统一装配认证、学生、课程、成绩、统计、报表等服务，CLI 和 GUI 都从这里取得这些对象。

还要注意一个运行目录细节：代码里的 `data/...` 是相对当前运行目录的路径。通常在项目根目录启动两个程序时，CLI 和 GUI 会读写同一个 `data/` 目录；如果从不同工作目录启动，它们可能各自读写不同位置下的 `data/`。为了减少混乱，建议统一从项目根目录启动 `edusys.exe` 和 `edusys_gui.exe`。

### 2.2.1 CLI 功能到代码的总索引

如果只想快速对照“菜单里选什么、代码走哪里、数据写哪里”，可以先看这张表。`docs/cli-demo-guide.md` 的章节号是课堂手工演示参考，不是程序里的代码行号。
如果你只是要运行或演示项目，先看前两列；如果你要读代码或答辩解释实现，再看后面的 View、Service、数据落点和验证方式。

| 角色/入口 | 菜单功能 | View 入口 | Service / Report 入口 | 主要数据落点 | 演示/验证方式 |
| --- | --- | --- | --- | --- | --- |
| 登录界面 | 用户名密码登录、空用户名退出、3 次失败退出 | `runInteractiveLoop()` | `AuthService::authenticate()` | `data/users.dat`、`data/app.log` | `cli-demo-guide` 第 3、9 节；`--self-test` A 组 |
| Admin 1 | 学生列表、按学号查、新增、编辑、删除学生 | `AdminMenu::studentMenu()` | `StudentService::listAll/findById/create/update/remove()` | `data/students.dat`、删除时还影响 `scores.dat` 和 `users.dat` | `cli-demo-guide` 第 4.1-4.6 节；`--self-test` B/C/D 组 |
| Admin 2 | 课程列表、按课程号查、新增、编辑、删除课程 | `AdminMenu::courseMenu()` | `CourseService::listAll/findById/create/update/remove()` | `data/courses.dat`、删除时还影响 `scores.dat` | `cli-demo-guide` 第 4.7-4.12 节；`--self-test` B/C 组 |
| Admin 3 | 成绩列表、按学生查、按课程查、录入/更新、删除单条成绩 | `AdminMenu::scoreMenu()` | `ScoreService::listAll/findByStudent/findByCourse/upsert/remove()` | `data/scores.dat` | `cli-demo-guide` 第 4.13-4.19 节；`--self-test` B/E 组 |
| Admin 4 | 课程统计、课程排名、学生 GPA | `AdminMenu::statsMenu()` | `StatsService::computeCourseStats/rankByCourse/computeGpaFor()` | 只读 `students.dat/courses.dat/scores.dat` | `cli-demo-guide` 第 4.20-4.23 节 |
| Admin 5 | 生成学业预警报告 | `AdminMenu::generateWarningReport()` | `ReportExporter::exportWarningReport()` | `data/warning_report.txt` | `cli-demo-guide` 第 4.24 节 |
| Admin 6 | 修改管理员自己的密码 | `AdminMenu::changePassword()` | `AuthService::changePassword()` | `data/users.dat` | `cli-demo-guide` 第 7 节；`--self-test` A 组覆盖失败边界 |
| Admin 7 | 导出课程统计 CSV 和排名 CSV | `AdminMenu::exportCsv()` | `ReportExporter::exportCourseStatsCsv/exportRankingCsv()` | `data/course_stats_<courseId>.csv`、`data/ranking_<courseId>.csv` | `cli-demo-guide` 第 4.25 节 |
| Teacher 1 | 查看我的课程 | `TeacherMenu::listMyCourses()` | `CourseService::listAll()` 后按 `teacherId` 过滤 | 只读 `data/courses.dat` | `cli-demo-guide` 第 5.2 节 |
| Teacher 2-5 | 查看本课程成绩、录入/更新、删除、统计 | `TeacherMenu::viewMyScores/upsertScore/deleteScore/courseStats()` | `ScoreService` 和 `StatsService`，先经 `pickOwnCourseId()` 选择本人课程 | 读写 `data/scores.dat`，统计只读 | `cli-demo-guide` 第 5.3-5.7 节；`--self-test` E 组 |
| Teacher 6 | 修改教师自己的密码 | `TeacherMenu::changePassword()` | `AuthService::changePassword()` | `data/users.dat` | `cli-demo-guide` 第 7 节说明入口，按需手工演示 |
| Student 1-3 | 查看我的资料、我的成绩、我的 GPA | `StudentMenu::viewProfile/viewMyScores/viewMyGpa()` | `StudentService`、`ScoreService`、`StatsService` | 只读 `students.dat/scores.dat/courses.dat` | `cli-demo-guide` 第 6.2-6.4 节 |
| Student 4 | 修改学生自己的密码 | `StudentMenu::changePassword()` | `AuthService::changePassword()` | `data/users.dat` | `cli-demo-guide` 第 7 节说明入口，按需手工演示 |
| `--self-test` | 自动回归检查，不进入菜单 | `main()` 参数分支 | 多个 Service 直接被调用 | 可能写日志；首次空数据会自动写入默认示例数据 | `README` 第 13.1 节、`docs/test-cases.md` |
| `tools/corrupt_check.bat` | 文件损坏恢复检查 | 批处理脚本 | 外部制造损坏后运行 `edusys.exe --self-test` | 临时备份/恢复 `data/*.dat`，输出到 `data/__corrupt_out__/` | `README` 第 13.2 节、`docs/test-cases.md` F 组 |

这张表也说明了一个边界：`docs/cli-demo-guide.md` 的主线会实际演示大部分 CLI 操作，但不是每个失败分支都逐项手打；失败边界主要由 `--self-test`、`corrupt_check.bat` 和演示文档的可选补充部分覆盖。

### 2.2.2 Qt GUI 功能到代码的总索引

下面这张表只讲 Qt GUI，不讲 CLI 菜单。它的作用是把 `edusys_gui.exe` 里每一个主要窗口功能和真实代码连起来，说明 GUI 操作会经过同一套服务层，并在相同运行目录下读写同一批真实数据文件。

表里的“Qt 代码入口”和“Service / Report 入口”都是源码位置或函数名，不是需要手动执行的命令。真正运行 GUI 的命令仍然是启动 `edusys_gui.exe`。

| GUI 入口/窗口 | 页面或动作 | Qt 代码入口 | Service / Report 入口 | 主要数据落点 | 与 CLI 的关系 |
| --- | --- | --- | --- | --- | --- |
| GUI 启动 | 初始化 Qt 应用、初始化 `AppContext`、数据为空时写入默认示例数据、进入登录循环 | `src/app/gui_main.cpp` 的 `main()` | `AppContext::initializeData()` | 首次数据为空时写入 `data/*.dat`，日志写入 `data/app.log` | 和 CLI 一样先装配核心层；GUI 不进入 `runInteractiveLoop()` |
| GUI 登录 | 用户名密码登录、空用户名退出、点击退出、连续 3 次失败退出 | `LoginDialog::tryLogin()` | `AuthService::authenticate()` | 读取 `data/users.dat`，写 `data/app.log` | 行为对齐 CLI 登录循环，只是错误提示换成 `QMessageBox` |
| 角色分发 | 按 `Session.role` 打开对应主窗口 | `createRoleWindow()` | 使用 `Session` 判断 `Admin/Teacher/Student` | 不直接落盘 | 和 CLI 的角色分发规则相同，但创建的是 Qt 主窗口 |
| AdminWindow | 6 个页签的主窗口容器 | `AdminWindow::AdminWindow()` | 后续页签分别调用各自服务 | 按具体功能落盘 | 对应 CLI 的 Admin 菜单功能集合 |
| Admin 学生管理 | 列表、按学号查看、新增、编辑、级联删除 | `createStudentPage()`、`refreshStudentTable()`、`showStudentById()`、`createStudent()`、`editSelectedStudent()`、`removeSelectedStudent()` | `StudentService::listAll/findById/create/update/remove()` | `students.dat`；删除学生时还影响 `scores.dat`、`users.dat` | 功能等价于 CLI `AdminMenu::studentMenu()` |
| Admin 课程管理 | 列表、按课程号查看、新增、编辑、级联删除 | `createCoursePage()`、`refreshCourseTable()`、`showCourseById()`、`createCourse()`、`editSelectedCourse()`、`removeSelectedCourse()` | `CourseService::listAll/findById/create/update/remove()` | `courses.dat`；删除课程时还影响 `scores.dat` | 功能等价于 CLI `AdminMenu::courseMenu()` |
| Admin 成绩管理 | 全部成绩、按学生查、按课程查、录入/更新、编辑、删除单条 | `createScorePage()`、`refreshScoreTable()`、`showScoresByStudent()`、`showScoresByCourse()`、`createScore()`、`editSelectedScore()`、`removeSelectedScore()` | `ScoreService::listAll/findByStudent/findByCourse/upsert/remove()` | `scores.dat` | 功能等价于 CLI `AdminMenu::scoreMenu()` |
| Admin 统计分析 | 课程统计、课程排名、学生 GPA | `createStatsPage()`、`queryCourseStats()`、`queryCourseRanking()`、`queryStudentGpa()` | `StatsService::computeCourseStats/rankByCourse/computeGpaFor()` | 只读 `students.dat/courses.dat/scores.dat` | 功能等价于 CLI `AdminMenu::statsMenu()` |
| Admin 报告导出 | 学业预警报告、课程统计 CSV、课程排名 CSV | `createReportPage()`、`exportWarningReport()`、`exportCourseStatsCsv()`、`exportRankingCsv()` | `ReportExporter::exportWarningReport/exportCourseStatsCsv/exportRankingCsv()` | `warning_report.txt`、`course_stats_<courseId>.csv`、`ranking_<courseId>.csv` | 导出路径和 CLI 保持一致 |
| Admin 账户 | 修改密码、退出登录 | `createAccountPage()`、`ChangePasswordDialog` | `AuthService::changePassword()` | `users.dat` | 退出登录是关闭主窗口，`gui_main.cpp` 再回到登录框 |
| TeacherWindow | 4 个页签的教师窗口容器 | `TeacherWindow::TeacherWindow()` | 后续页签分别调用课程、成绩、统计、认证服务 | 按具体功能落盘 | 对应 CLI 的 Teacher 菜单功能集合 |
| Teacher 我的课程 | 查看本人课程 | `createMyCoursesPage()`、`refreshMyCourses()`、`myCourses()` | `CourseService::listAll()` | 只读 `courses.dat` | Service 已按教师 `ownerId` 过滤；GUI 表格只展示本人课程 |
| Teacher 我的课程成绩 | 选择本人课程、加载成绩、录入/编辑/删除成绩 | `createMyScoresPage()`、`refreshMyScores()`、`createScore()`、`editSelectedScore()`、`removeSelectedScore()` | `ScoreService::findByCourse/upsert/remove()` | `scores.dat` | GUI 先用下拉框限制课程；Service 仍会拒绝越权课程 |
| Teacher 我的课程统计 | 选择本人课程、课程统计、课程排名 | `createMyStatsPage()`、`queryCourseStats()`、`queryCourseRanking()` | `StatsService::computeCourseStats/rankByCourse()` | 只读 `courses.dat/scores.dat/students.dat` | 教师只能统计自己的课，规则与 CLI 一致 |
| Teacher 账户 | 修改密码、退出登录 | `createAccountPage()`、`ChangePasswordDialog` | `AuthService::changePassword()` | `users.dat` | 退出后回到 GUI 登录框 |
| StudentWindow | 4 个页签的学生窗口容器 | `StudentWindow::StudentWindow()` | 后续页签分别调用学生、成绩、统计、认证服务 | 按具体功能落盘 | 对应 CLI 的 Student 菜单功能集合 |
| Student 我的资料 | 查看自己的学生档案 | `createProfilePage()` | `StudentService::findById(session_, session_.getOwnerId())` | 只读 `students.dat` | 和 CLI 一样只能看自己 |
| Student 我的成绩 | 查看自己的成绩列表 | `createMyScoresPage()`、`refreshMyScores()` | `ScoreService::findByStudent(session_, session_.getOwnerId())` | 只读 `scores.dat` | 和 CLI 一样只能看自己 |
| Student 我的 GPA | 查看自己的 GPA | `createMyGpaPage()`、`refreshMyGpa()` | `StatsService::computeGpaFor(session_, session_.getOwnerId())` | 只读 `students.dat/courses.dat/scores.dat` | 和 CLI 一样只能算自己 |
| Student 账户 | 修改密码、退出登录 | `createAccountPage()`、`ChangePasswordDialog` | `AuthService::changePassword()` | `users.dat` | 退出后回到 GUI 登录框 |
| GUI 表单对话框 | 新增/编辑学生、课程、成绩、修改密码 | `StudentEditDialog`、`CourseEditDialog`、`ScoreEditDialog`、`ChangePasswordDialog` | 学生/课程/成绩对话框收集字段后由窗口调用 Service；改密码对话框内部调用 `AuthService::changePassword()` | 按被调用 Service 决定 | 对话框不直接写 `.dat`，复杂规则仍由 Service 处理 |
| CLI 保留入口 | 自动自检、坏文件脚本、手工 CLI 演示 | 不属于 `src/gui/` | `--self-test`、`tools/corrupt_check.bat`、`docs/cli-demo-guide.md` | 按各自入口决定 | 这些不是 GUI 菜单项，仍由 CLI/脚本维护 |

如果要一句话概括这张表：从业务层角度看，Qt GUI 是把“输入输出方式”从 `cin/cout` 换成了 Qt 控件；从界面层角度看，GUI 仍然需要单独处理窗口、对话框、按钮事件和表格刷新。真正决定能不能登录、能不能越权、能不能写入、删除时清理哪些关联数据的地方，仍然是 `service/` 和 `storage/`。

### 2.3 默认样例账号

| 用户名 | 密码 | 角色 | 说明 |
| --- | --- | --- | --- |
| `admin` | `admin123` | `Admin` | 管理员账号，不关联任何 `Person` 实体 |
| `t001` | `t001pw` | `Teacher` | 关联教师 `T001` |
| `s001` | `s001pw` | `Student` | 关联学生 `S001` |

补充说明：

- 仓库第一次运行且五个 `.dat` 文件都为空时，会自动 seed 初始数据。这里的 seed 指写入默认账号、学生、课程和成绩样例数据。
- Week 11 自检会在首次 `SEEDED` 运行里做一次破坏性演化：新增 `S003`、删除 `S002`、更新 `S001` 的分数。`SEEDED` 是自检输出里的状态词，表示这次运行触发了初始化数据。
- 因此“当前数据状态”和“刚 seed 完的初始状态”可能不同，这是设计的一部分，不是数据漂移

### 2.4 本 README 的审查标准

为了让完全没看过代码的人也能判断“功能是否有对应实现”，后面的解读按下面标准写：

- 说明一个功能时，同时交代它从哪个入口进入。
- 说明一个文件时，同时交代它调用了哪个服务、服务又读写了哪个仓储。
- 说明权限控制时，同时交代哪个角色会被允许、哪个角色会被拒绝、拒绝发生在哪一层。
- 说明持久化时，同时交代数据最终写到哪个 `.dat`、`.txt` 或 `.csv` 文件。
- 说明测试时，同时交代 `--self-test` 或脚本验证的是哪条业务边界。
- 如果某个文件只是构建产物或本地工具配置，就不要把它作为业务功能描述，而是说明它为什么不是核心代码。

也就是说，这份 README 不是只列功能名称，而是按“可追踪链路”来讲。一个功能如果已经实现，应该能顺着这条链看下去：

```text
用户动作
  -> CLI 菜单类 / Qt 窗口类
  -> Service 业务规则
  -> Repository 读写集合
  -> Model 显式序列化
  -> data/ 下的 .dat / .txt / .csv 文件
```

后面会贴一些关键代码片段。读这些代码时不用逐行背语法，重点看三件事：

- 入口在哪里：是 `main.cpp`、`AdminMenu.cpp`、`TeacherMenu.cpp`，还是 Qt 的窗口文件。
- 规则在哪里：通常在 `src/service/*.cpp`，例如权限、校验、级联删除。
- 最后写到哪里：通常是 `data/*.dat`、`data/warning_report.txt` 或 `data/*.csv`。

如果你完全没看过这个项目，可以先只看每段代码前后的文字和表格；等知道功能链路后，再回头看代码细节。

### 2.5 先看懂系统如何启动

这个项目最容易混淆的地方是：CLI 和 GUI 有两个入口，但它们不是两套业务系统。

可以先把启动过程理解成三步：

```text
启动 exe
  -> 创建 AppContext，把数据读写工具和业务服务准备好
  -> 登录成功后，根据角色进入对应菜单或窗口
```

所以 `main.cpp` 和 `gui_main.cpp` 的主要区别不是“业务规则不同”，而是“一个把用户带到控制台菜单，一个把用户带到 Qt 窗口”。真正判断权限、校验字段、读写文件的代码，仍然在同一批 Service 和 Repository 里。

CLI 的入口在 [`src/app/main.cpp`](src/app/main.cpp)。它负责三件事：

- 解析命令行参数，决定是跑 `--self-test` 还是进入交互菜单。
- 创建 `AppContext`，让仓储、服务、导出器都准备好。
- 登录成功后按角色创建 `AdminMenu`、`TeacherMenu` 或 `StudentMenu`。

GUI 的入口在 [`src/app/gui_main.cpp`](src/app/gui_main.cpp)。它也做三件事：

- 创建 `QApplication` 和同一套 `AppContext`。
- 弹出 `LoginDialog` 完成认证。
- 登录成功后按角色创建 `AdminWindow`、`TeacherWindow` 或 `StudentWindow`。

两边共同使用的是 [`src/app/AppContext.cpp`](src/app/AppContext.cpp)。这个文件的构造函数把所有核心对象装配到一起：

如果你不熟悉 C++ 构造函数，先看结构就行：冒号后面这一串叫“成员初始化列表”，每一行都是在创建一个仓储或服务对象；括号里传进去的是它依赖的对象。

```cpp
AppContext::AppContext()
    : studentRepo()
    , teacherRepo()
    , userRepo()
    , courseRepo()
    , scoreRepo()
    , authService(userRepo)
    , studentService(studentRepo, scoreRepo, userRepo)
    , courseService(courseRepo, scoreRepo, teacherRepo)
    , scoreService(scoreRepo, courseRepo, studentRepo)
    , statsService(studentRepo, courseRepo, scoreRepo)
    , reportExporter(statsService) {}
```

这段代码说明几个关键事实：

- `AuthService` 只依赖 `UserRepository`，所以登录只碰账号表。
- `StudentService` 同时依赖学生、成绩、账号仓储，所以它能做“删学生时连成绩和账号一起清”的级联删除。
- `CourseService` 同时依赖课程、成绩、教师仓储，所以它能检查授课教师是否存在，也能删课时清理相关成绩。
- `ScoreService` 同时依赖成绩、课程、学生仓储，所以它能检查学生存在、课程存在、教师是否有权操作该课程。
- `StatsService` 依赖学生、课程、成绩仓储，所以统计可以把学生姓名、课程学分和成绩记录合并计算。
- `ReportExporter` 依赖 `StatsService`，说明报表只是把统计结果写成文件，不重新计算一套统计逻辑。

这里的 `studentRepo`、`courseRepo`、`scoreRepo` 这些名字可以理解成“数据表的读写对象”。服务层拿到这些对象以后，才有能力读学生、读课程、写成绩、导出报告。这样写的好处是：控制台和 Qt 不需要自己知道 `.dat` 文件怎么读写，只需要调用服务。

初始化数据也在 `AppContext` 里完成：

```cpp
bool AppContext::initializeData() {
    ensureDirectoryExists(DATA_DIR);

    const bool allEmpty =
        studentRepo.loadAll().empty() && teacherRepo.loadAll().empty() &&
        userRepo.loadAll().empty()    && courseRepo.loadAll().empty()  &&
        scoreRepo.loadAll().empty();

    if (!allEmpty) {
        Logger::instance().info("Existing data files detected -> skip seeding.");
        return false;
    }

    Logger::instance().info("All repositories empty -> seeding initial sample data.");
    seedSampleData(studentRepo, teacherRepo, userRepo, courseRepo, scoreRepo);
    return true;
}
```

这段逻辑的意思是：不是每次启动都覆盖数据。只有五个仓储都为空时才写入默认账号和样例数据。只要已有 `.dat` 数据，程序就跳过 seed，保持上次运行后的状态。

### 2.6 登录、会话和角色分发

CLI 登录流程从 `runInteractiveLoop()` 开始。用户输入用户名和密码后，入口层不直接查文件，而是调用 `AuthService::authenticate()`：

```cpp
UserAccount acc = authSvc.authenticate(username, password);
Session session;
session.login(acc.getUsername(), acc.getRole(), acc.getOwnerId());
```

这里的 `ownerId` 是账号关联的业务编号：教师账号 `t001` 关联教师编号 `T001`，学生账号 `s001` 关联学生编号 `S001`。管理员账号不对应某个学生或教师，所以它不依赖 `ownerId` 去限制可见数据。

认证成功后，`main.cpp` 根据账号角色分发到三个菜单：

```cpp
switch (acc.getRole()) {
    case RoleType::Admin: {
        AdminMenu menu(session, authSvc, studentSvc, courseSvc, scoreSvc,
                       statsSvc, reportExporter);
        menu.run();
        break;
    }
    case RoleType::Teacher: {
        TeacherMenu menu(session, authSvc, courseSvc, scoreSvc, statsSvc);
        menu.run();
        break;
    }
    case RoleType::Student: {
        StudentMenu menu(session, authSvc, studentSvc, scoreSvc, statsSvc);
        menu.run();
        break;
    }
}
```

密码校验在 [`src/service/AuthService.cpp`](src/service/AuthService.cpp)：

```cpp
UserAccount AuthService::authenticate(const std::string& username,
                                      const std::string& password) {
    auto users = userRepo_.loadAll();
    auto it = findByUsername(users, username);

    if (it == users.end()) {
        Logger::instance().warn("Auth failed: unknown user '" + username + "'");
        throw AuthException("Invalid username or password");
    }
    if (!it->isEnabled()) {
        Logger::instance().warn("Auth failed: account disabled '" + username + "'");
        throw AuthException("Account is disabled");
    }
    if (!PasswordHasher::verify(password, it->getPasswordHash())) {
        Logger::instance().warn("Auth failed: bad password for '" + username + "'");
        throw AuthException("Invalid username or password");
    }

    Logger::instance().info("Auth OK: '" + username + "'");
    return *it;
}
```

这条链路可以这样审查：

| 审查点 | 代码位置 | 说明 |
| --- | --- | --- |
| 用户输入从哪里来 | `src/app/main.cpp` 的 `runInteractiveLoop()` | 只负责读取用户名、密码和处理 3 次失败退出 |
| 账号从哪里查 | `AuthService::authenticate()` 调用 `userRepo_.loadAll()` | 账号数据来自 `data/users.dat` |
| 密码怎么验证 | `PasswordHasher::verify()` | 不直接比较明文 |
| 会话怎么形成 | `Session::login(username, role, ownerId)` | 后续所有权限判断都依赖 `Session` |
| 失败怎么处理 | 抛 `AuthException`，CLI 捕获后显示 `[ERR]` | 失败不会进入任何菜单 |

GUI 登录是同一条服务链。[`src/gui/LoginDialog.cpp`](src/gui/LoginDialog.cpp) 做界面输入，最终仍然调用同一个 `AuthService`，成功后把 `Session` 交给 `gui_main.cpp`，再由 `createRoleWindow()` 创建对应主窗口。

### 2.7 修改密码功能链路

三类角色的 CLI 菜单里都有“Change my password”。入口分别在 [`src/view/AdminMenu.cpp`](src/view/AdminMenu.cpp)、[`src/view/TeacherMenu.cpp`](src/view/TeacherMenu.cpp) 和 [`src/view/StudentMenu.cpp`](src/view/StudentMenu.cpp)。这三个菜单读入旧密码和新密码后，都调用同一个服务方法：

```text
AdminMenu::changePassword()
TeacherMenu::changePassword()
StudentMenu::changePassword()
  -> AuthService::changePassword(session, oldPw, newPw)
  -> UserRepository::loadAll()
  -> PasswordHasher::verify(oldPw, oldHash)
  -> UserAccount::setPasswordHash(PasswordHasher::hash(newPw))
  -> UserRepository::saveAll()
  -> data/users.dat
```

CLI 菜单层只负责读取输入和显示成功/失败，规则在 [`src/service/AuthService.cpp`](src/service/AuthService.cpp)：

```cpp
void AuthService::changePassword(const Session& session,
                                 const std::string& oldPassword,
                                 const std::string& newPassword) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in");
    }
    if (newPassword.empty()) {
        throw ValidationException("New password must not be empty");
    }

    auto users = userRepo_.loadAll();
    auto it = findByUsername(users, session.getUsername());
    if (it == users.end()) {
        throw AuthException("Session user no longer exists: " + session.getUsername());
    }
    if (!PasswordHasher::verify(oldPassword, it->getPasswordHash())) {
        Logger::instance().warn("ChangePassword failed: bad old password for '"
                                + session.getUsername() + "'");
        throw AuthException("Old password is incorrect");
    }

    it->setPasswordHash(PasswordHasher::hash(newPassword));
    userRepo_.saveAll(users);
    Logger::instance().info("Password changed for '" + session.getUsername() + "'");
}
```

这段逻辑可以拆成几步看：

| 步骤 | 发生位置 | 说明 |
| --- | --- | --- |
| 1 | `session.isLoggedIn()` | 未登录不能改密码 |
| 2 | `newPassword.empty()` | 新密码不能为空 |
| 3 | `findByUsername(users, session.getUsername())` | 只能修改当前会话自己的账号 |
| 4 | `PasswordHasher::verify(oldPassword, it->getPasswordHash())` | 旧密码必须正确 |
| 5 | `setPasswordHash(PasswordHasher::hash(newPassword))` | 新密码重新哈希，不保存明文 |
| 6 | `userRepo_.saveAll(users)` | 最终写回 `data/users.dat` |

Week 13 自检覆盖了“未登录改密码”和“新密码为空”两个负面边界；成功改密码本身属于 CLI/GUI 手工演示路径，因为它会改变当前账号密码，演示时需要记住新密码或提前准备可恢复的数据。

### 2.8 学生管理功能链路

学生管理只允许管理员写入，学生本人只能读自己的资料。服务层允许教师只读学生列表，但当前 CLI 教师菜单没有提供“学生列表”入口，所以课堂演示 CLI 时不要把它当成教师端菜单功能。写入限制集中在 [`src/service/StudentService.cpp`](src/service/StudentService.cpp)。

管理员在 CLI 里进入学生管理的入口是 [`src/view/AdminMenu.cpp`](src/view/AdminMenu.cpp) 的 `studentMenu()`。这个菜单负责显示选项、读取学号和字段，然后调用 `StudentService`：

```text
AdminMenu::studentMenu()
  -> studentSvc_.listAll(session_)
  -> studentSvc_.findById(session_, id)
  -> studentSvc_.create(session_, student)
  -> studentSvc_.update(session_, student)
  -> studentSvc_.remove(session_, id)
```

管理员限制在 `StudentService` 的私有辅助函数里：

```cpp
void requireAdmin(const Session& session, const std::string& op) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=" + op + ")");
    }
    if (!session.isAdmin()) {
        throw PermissionException("Admin role required for op=" + op);
    }
}
```

新增学生的核心链路是：

```cpp
void StudentService::create(const Session& session, const Student& student) {
    requireAdmin(session, "Student.create");
    validateStudent(student);

    auto all = studentRepo_.loadAll();
    auto dup = std::find_if(all.begin(), all.end(),
        [&](const Student& s) { return s.getId() == student.getId(); });
    if (dup != all.end()) {
        throw ValidationException("Student id already exists: " + student.getId());
    }
    all.push_back(student);
    studentRepo_.saveAll(all);
    Logger::instance().info("Student created: " + student.getId());
}
```

这段代码能看出：

- `requireAdmin()` 先拦住非管理员。
- `validateStudent()` 检查学号不能为空、姓名不能为空、入学年份必须大于 0；学号唯一性由后面的查重逻辑保证。
- `studentRepo_.loadAll()` 把 `data/students.dat` 读成内存列表。
- `std::find_if()` 检查学号是否重复。
- `studentRepo_.saveAll(all)` 整批覆写学生数据文件。
- `Logger::instance().info()` 把操作写到 `data/app.log`。

学生删除涉及成绩和账号的级联清理，代码如下：

```cpp
void StudentService::remove(const Session& session, const std::string& id) {
    requireAdmin(session, "Student.remove");

    auto students = studentRepo_.loadAll();
    auto it = std::find_if(students.begin(), students.end(),
        [&](const Student& s) { return s.getId() == id; });
    if (it == students.end()) {
        throw ValidationException("Student not found: " + id);
    }

    auto scores = scoreRepo_.loadAll();
    const auto scoresBefore = scores.size();
    scores.erase(std::remove_if(scores.begin(), scores.end(),
        [&](const Score& sc) { return sc.getStudentId() == id; }),
        scores.end());
    const auto removedScores = scoresBefore - scores.size();
    scoreRepo_.saveAll(scores);

    auto users = userRepo_.loadAll();
    const auto usersBefore = users.size();
    users.erase(std::remove_if(users.begin(), users.end(),
        [&](const UserAccount& u) {
            return u.getRole() == RoleType::Student && u.getOwnerId() == id;
        }),
        users.end());
    const auto removedUsers = usersBefore - users.size();
    userRepo_.saveAll(users);

    students.erase(it);
    studentRepo_.saveAll(students);
}
```

这条链路要按顺序理解：

| 步骤 | 发生位置 | 结果 |
| --- | --- | --- |
| 1 | `requireAdmin()` | 非管理员不能删除学生 |
| 2 | `studentRepo_.loadAll()` | 先确认学生确实存在 |
| 3 | `scoreRepo_.loadAll()` + `scoreRepo_.saveAll()` | 删除该学生所有成绩，写回 `data/scores.dat` |
| 4 | `userRepo_.loadAll()` + `userRepo_.saveAll()` | 删除该学生对应登录账号，写回 `data/users.dat` |
| 5 | `studentRepo_.saveAll()` | 删除学生本体，写回 `data/students.dat` |
| 6 | `Logger` | 记录删除了多少成绩和账号 |

上面的代码片段为了突出级联删除顺序，没有把函数末尾的日志行完整贴出。实际实现会继续把删除的学生编号、被清理的成绩数量、被清理的账号数量写入 `data/app.log`。

学生级联删除可以按上述顺序追踪。老师如果问“怎么保证删除学生后没有残留成绩和残留账号”，可以看这里，再看 `--self-test` 里的 D 组检查。

### 2.9 课程管理功能链路

课程管理同样只有管理员可以写。入口在 `AdminMenu::courseMenu()`，最终调用 [`src/service/CourseService.cpp`](src/service/CourseService.cpp)。

新增课程时，服务层不只检查课程字段，还会检查授课教师是否真实存在：

```cpp
void CourseService::create(const Session& session, const Course& course) {
    requireAdmin(session, "Course.create");
    validateCourse(course);

    auto teachers = teacherRepo_.loadAll();
    auto teacherIt = std::find_if(teachers.begin(), teachers.end(),
        [&](const Teacher& t) { return t.getId() == course.getTeacherId(); });
    if (teacherIt == teachers.end()) {
        throw ValidationException("Course teacherId not found: " + course.getTeacherId());
    }

    auto courses = courseRepo_.loadAll();
    auto dup = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == course.getCourseId(); });
    if (dup != courses.end()) {
        throw ValidationException("Course id already exists: " + course.getCourseId());
    }
    courses.push_back(course);
    courseRepo_.saveAll(courses);
}
```

这里体现了两个完整性规则：

- 课程号不能重复，因为 `courseRepo_.loadAll()` 后会查重。
- `teacherId` 必须能在 `teachers.dat` 里找到，否则课程不能创建。

删除课程时，也有级联删除，只不过它只需要清成绩，不需要清账号：

```cpp
void CourseService::remove(const Session& session, const std::string& courseId) {
    requireAdmin(session, "Course.remove");

    auto courses = courseRepo_.loadAll();
    auto it = std::find_if(courses.begin(), courses.end(),
        [&](const Course& c) { return c.getCourseId() == courseId; });
    if (it == courses.end()) {
        throw ValidationException("Course not found: " + courseId);
    }

    auto scores = scoreRepo_.loadAll();
    const auto scoresBefore = scores.size();
    scores.erase(std::remove_if(scores.begin(), scores.end(),
        [&](const Score& sc) { return sc.getCourseId() == courseId; }),
        scores.end());
    const auto removedScores = scoresBefore - scores.size();
    scoreRepo_.saveAll(scores);

    courses.erase(it);
    courseRepo_.saveAll(courses);
}
```

这说明课程删除的真实影响范围是：

- `data/courses.dat` 删除课程本体。
- `data/scores.dat` 删除该课程下所有成绩。
- `data/users.dat` 不变，因为课程不是账号所有者。
- `data/teachers.dat` 不变，因为删课不等于删教师。

### 2.10 成绩管理功能链路

成绩管理涉及的权限判断较多。管理员能管理全部成绩，教师只能管理自己课程的成绩，学生只能看自己的成绩，不能写。

CLI 入口在 `AdminMenu::scoreMenu()` 和 [`src/view/TeacherMenu.cpp`](src/view/TeacherMenu.cpp)。两边最终都落到 [`src/service/ScoreService.cpp`](src/service/ScoreService.cpp)。

成绩写入使用 `upsert`，也就是同一个 `(studentId, courseId, semester)` 已存在则更新，不存在则新增。这里的三个分数含义是：

- `usualScore`：平时分。
- `finalScore`：期末分。
- `totalScore`：总评。

三项分数都由菜单输入，范围都是 `0-100`。系统在这里校验范围，但不自动用平时分和期末分计算总评。

```cpp
void ScoreService::upsert(const Session& session, const Score& score) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in (op=Score.upsert)");
    }
    if (session.isStudent()) {
        throw PermissionException("Student is not allowed to write scores");
    }

    if (score.getStudentId().empty() || score.getCourseId().empty() || score.getSemester().empty()) {
        throw ValidationException("Score key fields (studentId/courseId/semester) must not be empty");
    }
    validateRange(score.getUsualScore(), "usualScore");
    validateRange(score.getFinalScore(), "finalScore");
    validateRange(score.getTotalScore(), "totalScore");

    requireStudentExists(studentRepo_, score.getStudentId());
    Course course = requireCourse(courseRepo_, score.getCourseId());

    if (session.isTeacher() && course.getTeacherId() != session.getOwnerId()) {
        throw PermissionException("Teacher can only write scores of own courses");
    }

    auto scores = scoreRepo_.loadAll();
    auto it = std::find_if(scores.begin(), scores.end(),
        [&](const Score& s) {
            return sameKey(s, score.getStudentId(), score.getCourseId(), score.getSemester());
        });
    const bool isUpdate = (it != scores.end());
    if (isUpdate) {
        *it = score;
    } else {
        scores.push_back(score);
    }
    scoreRepo_.saveAll(scores);
}
```

成绩写入的审查点可以归纳为：

| 审查点 | 具体代码 | 防住的问题 |
| --- | --- | --- |
| 必须登录 | `if (!session.isLoggedIn())` | 未登录用户不能写 |
| 学生不能写 | `if (session.isStudent())` | 学生不能给自己或别人改成绩 |
| 主键字段不能为空 | `studentId/courseId/semester` 检查 | 防止无效成绩记录落盘 |
| 分数范围限制 | `validateRange()` | 防止 `999`、`-1` 这类分数 |
| 学生必须存在 | `requireStudentExists()` | 防止成绩挂到不存在学生上 |
| 课程必须存在 | `requireCourse()` | 防止成绩挂到不存在课程上 |
| 教师白名单 | `course.getTeacherId() != session.getOwnerId()` | 教师不能改别人的课 |
| 同键更新 | `sameKey()` 查找已有成绩 | 避免同一学生同一课程同一学期重复多条 |
| 最终落盘 | `scoreRepo_.saveAll(scores)` | 写回 `data/scores.dat` |

删除成绩也走类似权限链：

```cpp
if (session.isStudent()) {
    throw PermissionException("Student is not allowed to delete scores");
}
if (session.isTeacher()) {
    Course course = requireCourse(courseRepo_, courseId);
    if (course.getTeacherId() != session.getOwnerId()) {
        throw PermissionException("Teacher can only delete scores of own courses");
    }
}
```

删除单条成绩的完整落盘链路是：

```text
AdminMenu::scoreMenu() / TeacherMenu::deleteScore()
  -> ScoreService::remove(session, studentId, courseId, semester)
  -> scoreRepo_.loadAll()
  -> 删除匹配 studentId + courseId + semester 的那一条
  -> scoreRepo_.saveAll(scores)
  -> data/scores.dat
```

这说明教师端的权限控制不是只靠菜单隐藏选项。如果不经过菜单而直接调用服务层，只要 `Session` 是教师，`ScoreService` 仍然会重新检查课程归属。

### 2.11 统计分析功能链路

统计功能集中在 [`src/service/StatsService.cpp`](src/service/StatsService.cpp)。它不写 `cout`，不写文件，只返回结构化结果。这样 CLI、GUI、CSV 导出都能复用同一套统计口径。

GPA 计算的关键是先把总评分映射成绩点，再按课程学分加权：

```cpp
double gpaPointFor(double total) {
    if (total >= 90.0) return 4.0;
    if (total >= 85.0) return 3.7;
    if (total >= 80.0) return 3.3;
    if (total >= 75.0) return 3.0;
    if (total >= 70.0) return 2.7;
    if (total >= 65.0) return 2.3;
    if (total >= 60.0) return 2.0;
    return 0.0;
}
```

`computeGpaFor()` 会同时读取学生、课程和成绩：

```cpp
auto students = studentRepo_.loadAll();
auto courses  = courseRepo_.loadAll();
auto scores   = scoreRepo_.loadAll();

double weightedSum = 0.0;
for (const auto& s : scores) {
    if (s.getStudentId() != studentId) continue;
    const double credit = findCourseCredit(courses, s.getCourseId());
    if (credit <= 0.0) continue;
    weightedSum += gpaPointFor(s.getTotalScore()) * credit;
    result.totalCredit += credit;
    ++result.courseCount;
}
result.gpa = (result.totalCredit > 0.0) ? (weightedSum / result.totalCredit) : 0.0;
```

课程统计 `computeCourseStats()` 会计算：

- `count`：课程成绩条数
- `avg`：平均总评
- `max` / `min`：最高分和最低分
- `passRate`：总评大于等于 `SCORE_PASS = 60.0` 的比例
- `excellentRate`：总评大于等于 `SCORE_EXCELLENT = 90.0` 的比例

课程排名 `rankByCourse()` 的核心是按总评降序排序：

```cpp
std::sort(out.begin(), out.end(),
    [](const RankEntry& a, const RankEntry& b) { return a.totalScore > b.totalScore; });
```

预警报告的数据来源是 `computeAllWarnings()`。它按学生聚合 GPA 和挂科数，只把达到阈值的学生放进结果：

```cpp
const bool warn = (gpa < kWarnGpaThreshold) || (it->second.failCount >= kWarnFailThreshold);
if (!warn) continue;
```

这条规则的阈值来自 [`include/EduSys/service/StatsService.hpp`](include/EduSys/service/StatsService.hpp) 里的 `kWarnGpaThreshold = 2.0` 和 `kWarnFailThreshold = 2`。两个条件是 OR 关系：GPA 低于 2.0，或者挂科门数达到 2 门及以上，都会进入预警结果。

### 2.12 报告和 CSV 导出功能链路

报告导出集中在 [`src/report/ReportExporter.cpp`](src/report/ReportExporter.cpp)。它的定位是“把 `StatsService` 的结构化结果写成文件”，不是重新实现统计。

预警报告的链路是：

```text
AdminMenu::generateWarningReport()
  -> ReportExporter::exportWarningReport(session)
  -> StatsService::computeAllWarnings(session)
  -> data/warning_report.txt
```

对应代码：

```cpp
std::string ReportExporter::exportWarningReport(const Session& session) {
    auto warnings = stats_.computeAllWarnings(session);

    std::ofstream out(kWarningReportPath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        throw StorageException(std::string("Failed to open warning report: ") + kWarningReportPath);
    }
    ...
    Logger::instance().info("Warning report exported: hits=" + std::to_string(warnings.size())
                            + " path=" + kWarningReportPath);
    return kWarningReportPath;
}
```

报告文件里的内容大致长这样：

```text
EduSys Academic Warning Report
Generated at : 2026-06-06 10:30:00
Operator     : admin (Admin)
Threshold    : GPA < 2.0  OR  failCount >= 2
Hits         : 1

StuId   Name             GPA  Fail  FailedCourses
S004    Demo Student     1.70     2  C001,C002
```

真实文件会根据当前 `data/*.dat` 里的成绩变化而变化。生成逻辑是：先由 `StatsService::computeAllWarnings()` 得到预警学生列表，再由 `ReportExporter` 把生成时间、操作者、阈值和每个学生的预警信息写入 `data/warning_report.txt`。

课程统计 CSV 的链路是：

```text
AdminMenu::exportCsv()
  -> ReportExporter::exportCourseStatsCsv(session, courseId)
  -> StatsService::computeCourseStats(session, courseId)
  -> data/course_stats_<courseId>.csv
```

排名 CSV 的链路是：

```text
AdminMenu::exportCsv()
  -> ReportExporter::exportRankingCsv(session, courseId)
  -> StatsService::rankByCourse(session, courseId)
  -> data/ranking_<courseId>.csv
```

CSV 导出还多做了两层防护。第一层是权限，只有管理员能导出：

```cpp
if (!session.isAdmin()) {
    throw PermissionException("Admin role required for CSV export");
}
```

第二层是文件名安全检查，避免课程号里带路径分隔符、Windows 文件名非法字符或 ASCII 控制字符：

```cpp
void requireSafeCourseIdForFilename(const std::string& courseId) {
    if (courseId.empty()) {
        throw ValidationException("CSV export: courseId must not be empty");
    }
    for (char c : courseId) {
        if (c == '/' || c == '\\' || c == '"' || c == '\'' ||
            c == ':' || c == '*'  || c == '?' || c == '<'  ||
            c == '>' || c == '|'  || c <  0x20) {
            throw ValidationException(
                "CSV export: courseId contains unsafe character: " + courseId);
        }
    }
}
```

所以这里不是直接使用用户输入生成文件名，而是先拒绝明显危险字符，再生成固定目录下的 CSV。

### 2.13 二进制持久化功能链路

所有 `.dat` 文件都通过 [`include/EduSys/storage/BinaryRepository.hpp`](include/EduSys/storage/BinaryRepository.hpp) 统一读写。它是模板类，具体类型可以是 `Student`、`Course`、`Score` 等。

读取逻辑是：

```cpp
std::vector<T> loadAll() {
    if (!fileExists()) {
        return {};
    }
    BinaryReader r(path_);
    checkHeader(r);
    const std::uint32_t count = r.readUint32();
    std::vector<T> items;
    items.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        items.push_back(T::readFrom(r));
    }
    return items;
}
```

写入逻辑是：

```cpp
void saveAll(const std::vector<T>& items) {
    BinaryWriter w(path_);
    writeHeader(w, static_cast<std::uint32_t>(items.size()));
    for (const auto& item : items) {
        item.writeTo(w);
    }
}
```

这套设计有几个重要含义：

- 文件不存在时不会立即终止流程，而是返回空列表，交给 `AppContext::initializeData()` 判断是否 seed。
- 每个文件开头都有 `EDSY + version + count`，坏文件能在 `checkHeader()` 阶段尽早暴露。
- 仓储不知道实体字段细节，字段顺序由 `Student::writeTo()`、`Score::writeTo()` 这类实体方法决定。
- 写入采用整批覆写，不做复杂增量更新，代码更容易审查。

从业务功能到文件的落点可以这样看：

| 业务对象 | 仓储类型 | 实体类型 | 文件 |
| --- | --- | --- | --- |
| 账号 | `UserRepository` | `UserAccount` | `data/users.dat` |
| 学生 | `StudentRepository` | `Student` | `data/students.dat` |
| 教师 | `TeacherRepository` | `Teacher` | `data/teachers.dat` |
| 课程 | `CourseRepository` | `Course` | `data/courses.dat` |
| 成绩 | `ScoreRepository` | `Score` | `data/scores.dat` |

这也是 `tools/corrupt_check.bat` 能测试文件损坏的原因：只要破坏文件头或计数字段，仓储读取时就会抛 `StorageException`，损坏数据不会被按正常数据处理。

### 2.14 CLI 和 Qt 为什么能共存

CLI 和 GUI 共存不是靠复制业务代码，而是靠分层边界。白话一点说，就是界面层分开写，业务层和数据层共用。两条界面入口调用同一组服务和仓储：

```text
CLI: src/view/AdminMenu.cpp / TeacherMenu.cpp / StudentMenu.cpp
GUI: src/gui/AdminWindow.cpp / TeacherWindow.cpp / StudentWindow.cpp

共同调用:
AuthService / StudentService / CourseService / ScoreService / StatsService / ReportExporter

共同读写（路径相对当前运行目录）:
data/users.dat / data/students.dat / data/teachers.dat / data/courses.dat / data/scores.dat
```

Qt 主循环里的角色分发如下：

```cpp
std::unique_ptr<QMainWindow> createRoleWindow(EduSys::AppContext& appContext,
                                              const EduSys::Session& session) {
    using namespace EduSys;

    switch (session.getRole()) {
        case RoleType::Admin:
            return std::make_unique<AdminWindow>(appContext, session);
        case RoleType::Teacher:
            return std::make_unique<TeacherWindow>(appContext, session);
        case RoleType::Student:
            return std::make_unique<StudentWindow>(appContext, session);
    }

    throw EduSys::EduException("Unsupported session role.");
}
```

主窗口关闭后回到登录框的逻辑也在 `gui_main.cpp`：

```cpp
while (true) {
    EduSys::LoginDialog loginDialog(appContext);
    if (loginDialog.exec() != QDialog::Accepted) {
        logger.info("EduSys GUI shutdown normally (login cancelled).");
        break;
    }

    auto* mainWindow = createRoleWindow(appContext, loginDialog.session()).release();
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    mainWindow->show();

    QEventLoop eventLoop;
    QObject::connect(mainWindow, &QObject::destroyed, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
}
```

这段代码说明：

- GUI 登录成功后不是进入 CLI 菜单，而是创建 Qt 主窗口。
- GUI 的 `Session` 和 CLI 的 `Session` 是同一种对象。
- GUI 窗口关闭后不会直接结束程序，而是回到登录循环。
- 在相同运行目录下启动时，GUI 和 CLI 修改的是同一批 `data/*.dat` 文件，所以 GUI 改完后 CLI 能读到，CLI 改完后 GUI 也能读到。
- `createRoleWindow(...).release()` 把窗口指针交给 Qt 管理；`Qt::WA_DeleteOnClose` 表示窗口关闭时自动删除；`destroyed` 信号触发局部 `QEventLoop` 退出，所以这里不是让窗口对象无人管理。

如果老师问“Qt 版本是不是另写了一套演示数据”，可以按代码说明：`gui_main.cpp` 和 `main.cpp` 都创建同一个 `AppContext`，而 `AppContext` 装配的是同一批仓储和服务。通常从项目根目录启动时，两者最终读写同一个 `data/` 目录；如果人为从不同工作目录启动，就要注意 `data/` 会按各自运行目录解析。

## 3. 文档应该怎么读

如果你是第一次打开项目，先读本 README 的第 1-3 章，分清 CLI、GUI、默认账号和总体功能。读完这些之后，再按下面顺序深入：

| 顺序 | 文件 | 适合什么时候看 |
| --- | --- | --- |
| 1 | [`docs/architecture.md`](docs/architecture.md) | 先建立项目整体结构的认识 |
| 2 | [`docs/defense.md`](docs/defense.md) | 准备答辩，或者想知道“为什么这样设计” |
| 3 | [`docs/test-cases.md`](docs/test-cases.md) | 想看可验证性、边界测试与损坏恢复 |
| 4 | [`docs/cli-demo-guide.md`](docs/cli-demo-guide.md) | 想按 CLI 做主线演示，并查看可选补充功能 |
| 5 | [`docs/introduceQt-acceptance.md`](docs/introduceQt-acceptance.md) | 想查看 `introduceQt` GUI 的验收状态 |
| 6 | [`claude.md`](claude.md) | 想看最初设计意图、范围控制和架构红线 |

## 4. 仓库目录树总览

这棵树不是把工作区里每一个临时文件都列出来，而是列“理解项目需要看的核心阅读树”：核心源码、核心文档、核心脚本、主要运行产物和构建目录。当前工作区里的本地材料、未跟踪文件和异常残留会在树后单独说明，不代表都应该提交。

下面这棵树分成两类：

- **人工维护文件**：源码、文档、脚本、说明文件
- **运行期/构建期产物**：`.dat`、日志、CSV、`build/` 生成物

第一次看目录树时，可以按这个顺序理解：

1. 先看 `include/` 和 `src/`：这是系统代码本体，前者偏“声明有什么”，后者偏“具体怎么做”。
2. 再看 `data/`：这是程序运行后保存状态和导出结果的地方。
3. 再看 `docs/` 和 `tools/`：一个负责解释项目，一个负责辅助验证。
4. 最后看 `build/` 和 `build-qt-*`：它们是构建工具生成的内容，不是理解业务功能的起点。

```text
Student_Score_Management_System/
├─ .gitignore
├─ CMakeLists.txt
├─ build.bat
├─ README.md
├─ claude.md
├─ demo_input.txt
├─ data/
│  ├─ users.dat
│  ├─ students.dat
│  ├─ teachers.dat
│  ├─ courses.dat
│  ├─ scores.dat
│  ├─ app.log
│  ├─ warning_report.txt
│  ├─ course_stats_<courseId>.csv
│  ├─ ranking_<courseId>.csv
│  ├─ __corrupt_out__/
│  │  ├─ F1.out
│  │  ├─ F2.out
│  │  ├─ F3.out
│  │  └─ sanity.out
│  ├─ __selftest_status__.txt
│  ├─ __selftest_capture__.txt
│  ├─ __selftest_after_patch__.txt
│  ├─ __selftest_after_patch_status__.txt
│  ├─ __root_selftest_after_patch__.txt
│  └─ __root_selftest_after_patch_status__.txt
├─ docs/
│  ├─ .gitkeep
│  ├─ architecture.md
│  ├─ defense.md
│  ├─ test-cases.md
│  ├─ cli-demo-guide.md
│  └─ introduceQt-acceptance.md
├─ include/
│  └─ EduSys/
│     ├─ app/
│     │  ├─ .gitkeep
│     │  └─ AppContext.hpp
│     ├─ common/
│     │  ├─ Constants.hpp
│     │  ├─ Exception.hpp
│     │  ├─ Logger.hpp
│     │  ├─ PasswordHasher.hpp
│     │  └─ Types.hpp
│     ├─ model/
│     │  ├─ Person.hpp
│     │  ├─ Student.hpp
│     │  ├─ Teacher.hpp
│     │  ├─ UserAccount.hpp
│     │  ├─ Course.hpp
│     │  └─ Score.hpp
│     ├─ report/
│     │  └─ ReportExporter.hpp
│     ├─ service/
│     │  ├─ .gitkeep
│     │  ├─ AuthService.hpp
│     │  ├─ CourseService.hpp
│     │  ├─ ScoreService.hpp
│     │  ├─ Session.hpp
│     │  ├─ StatsService.hpp
│     │  └─ StudentService.hpp
│     ├─ storage/
│     │  ├─ .gitkeep
│     │  ├─ BinaryReader.hpp
│     │  ├─ BinaryWriter.hpp
│     │  ├─ BinaryRepository.hpp
│     │  ├─ StudentRepository.hpp
│     │  ├─ TeacherRepository.hpp
│     │  ├─ UserRepository.hpp
│     │  ├─ CourseRepository.hpp
│     │  └─ ScoreRepository.hpp
│     ├─ gui/
│     │  ├─ AdminWindow.hpp
│     │  ├─ ChangePasswordDialog.hpp
│     │  ├─ CourseEditDialog.hpp
│     │  ├─ LoginDialog.hpp
│     │  ├─ ScoreEditDialog.hpp
│     │  ├─ StudentEditDialog.hpp
│     │  ├─ StudentWindow.hpp
│     │  └─ TeacherWindow.hpp
│     └─ view/
│        ├─ .gitkeep
│        ├─ BaseMenu.hpp
│        ├─ AdminMenu.hpp
│        ├─ TeacherMenu.hpp
│        └─ StudentMenu.hpp
├─ src/
│  ├─ app/
│  │  ├─ AppContext.cpp
│  │  ├─ gui_main.cpp
│  │  └─ main.cpp
│  ├─ common/
│  │  ├─ Logger.cpp
│  │  └─ PasswordHasher.cpp
│  ├─ model/
│  │  ├─ Student.cpp
│  │  ├─ Teacher.cpp
│  │  ├─ UserAccount.cpp
│  │  ├─ Course.cpp
│  │  └─ Score.cpp
│  ├─ report/
│  │  └─ ReportExporter.cpp
│  ├─ service/
│  │  ├─ .gitkeep
│  │  ├─ AuthService.cpp
│  │  ├─ StudentService.cpp
│  │  ├─ CourseService.cpp
│  │  ├─ ScoreService.cpp
│  │  └─ StatsService.cpp
│  ├─ storage/
│  │  ├─ .gitkeep
│  │  ├─ BinaryReader.cpp
│  │  └─ BinaryWriter.cpp
│  ├─ gui/
│  │  ├─ ChangePasswordDialog.cpp
│  │  ├─ CourseEditDialog.cpp
│  │  ├─ LoginDialog.cpp
│  │  ├─ ScoreEditDialog.cpp
│  │  ├─ StudentEditDialog.cpp
│  │  ├─ StudentWindow.cpp
│  │  ├─ TeacherWindow.cpp
│  │  └─ AdminWindow.cpp
│  └─ view/
│     ├─ .gitkeep
│     ├─ BaseMenu.cpp
│     ├─ AdminMenu.cpp
│     ├─ TeacherMenu.cpp
│     └─ StudentMenu.cpp
├─ tools/
│  └─ corrupt_check.bat
└─ build/
   ├─ EduSys.sln
   ├─ edusys.vcxproj
   ├─ ALL_BUILD.vcxproj
   ├─ ZERO_CHECK.vcxproj
   ├─ CMakeCache.txt
   ├─ cmake_install.cmake
   ├─ CMakeFiles/...
   ├─ edusys.dir/...
   └─ x64/...
```

另外还有 `build-qt-introduceQt/`、`build-qt-mingw2/`、`build-qt-mingw/`、`build-qt/`、`build-qt-gui-check/`、`build-qt-verify/` 这些 Qt 构建目录。它们和 `build/` 一样属于构建生成内容，这里不把每个中间文件逐个展开。

当前工作区还可能存在一些本地材料或未跟踪文件。它们不属于学生成绩管理系统的核心源码；如果后续要提交，需要先确认它们是否应该纳入版本库。

| 文件 / 目录 | 当前性质 | 是否影响系统运行 | 建议 |
| --- | --- | --- | --- |
| `.claude/settings.json` | 本地 AI 工具配置，当前未跟踪 | 不被 CMake、`build.bat` 或程序运行读取 | 默认不作为项目交付内容提交。 |
| `.claude/settings.local.json` | 本地 AI 工具配置，当前已跟踪 | 不影响学生成绩管理系统本身 | 保留或清理前先确认课程提交要求。 |
| `weeklog.xlsx`、`weekly_report.xlsx`、`大作业周记（详细认真版）.xlsx` | 课程过程材料 / Excel 产物 | 不被 `edusys.exe` 读取 | 可作为课程材料另行管理，不算系统源码。 |
| `tools/build_detailed_weekly_journal_xlsx.py` | 生成周记 Excel 的辅助脚本，当前未跟踪 | 不参与系统构建和运行 | 如需提交周记生成过程，再考虑纳入版本库。 |
| `(unspecified file_path)` | 异常命名残留文件，当前未跟踪 | 不被源码、CMake 或脚本引用 | 建议确认来源；无用途时可清理。 |

下面开始逐级展开。

## 5. 根目录逐文件说明

| 文件 / 目录 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`.gitignore`](.gitignore) | 当前主要忽略根目录 `edusys.exe` 与 `build/Release/edusys.exe` 这类容易造成运行产物混淆的文件。 | 防止把本地编译结果误提交进仓库，避免测试脚本和真实源码状态错位。 |
| [`CMakeLists.txt`](CMakeLists.txt) | 定义项目名 `EduSys`、C++17 标准、MSVC / 非 MSVC 编译选项，以及 `edusys` / `edusys_gui` 两个目标的源文件列表。 | 面向 CMake/Visual Studio/Qt 的正式构建入口。 |
| [`build.bat`](build.bat) | 一个简洁的 `g++` 构建脚本，按脚本列出的 CLI/core/view 源文件编译为根目录 `edusys.exe`，不包含 Qt GUI 源文件。 | 课程环境备用构建方案；适合“没有完整 CMake 工具链，但能用 g++”的情况。 |
| [`README.md`](README.md) | 也就是你正在看的这份文档。 | 仓库导航、运行说明、目录树总解说。 |
| [`claude.md`](claude.md) | 开题报告修订版，包含任务分析、目标范围、架构图、风险评估、Qt 预留思路、开发红线。 | 这是项目“为什么这样设计”的源头文档。 |
| [`docs/cli-demo-guide.md`](docs/cli-demo-guide.md) | 当前工作区新增的 CLI 端演示手册，按输入输出手把手演示 `edusys.exe`。 | 给课堂演示准备的纯 CLI 教程；提交前需要确认是否纳入版本库。 |
| [`docs/introduceQt-acceptance.md`](docs/introduceQt-acceptance.md) | 当前工作区新增的 `introduceQt` 分支 GUI 验收记录。 | 记录 Qt 版当前做到哪一步、哪些项还需要手工勾验；提交前需要确认是否纳入版本库。 |
| [`demo_input.txt`](demo_input.txt) | 一份标准输入脚本，串起 Admin、Teacher、Student 三段演示路径。 | 用于通过标准输入复现一段交互流程，适合录屏、答辩彩排、回归展示；它不等同于完整手工验收表。 |
| [`.claude/`](.claude) | 本地 AI 工具权限配置目录，其中 `settings.local.json` 已在版本库中，`settings.json` 是当前工作区本地文件。 | 它不是学生成绩管理系统的业务代码，不参与编译、不参与运行、不影响 `edusys.exe` 或 `edusys_gui.exe`。 |
| [`data/`](data) | 所有运行期数据、日志、报表、CSV 与损坏测试输出都放在这里。 | 这是程序的“工作目录”；删掉其中的 `.dat` 会影响状态。 |
| [`docs/`](docs) | 架构图册、答辩稿、测试汇总文档。 | 面向老师、答辩和维护者。 |
| [`include/`](include) | 公开头文件树。 | 体现类型定义、服务接口、仓储接口与菜单类边界。 |
| [`src/`](src) | 具体实现。 | 所有业务行为都最终在这里落地。 |
| [`tools/`](tools) | 工程辅助脚本。 | 当前核心验证脚本是 F 组文件损坏脚本；周记生成脚本属于本地辅助材料。 |
| [`build/`](build) | CMake / MSVC 自动生成目录。 | 不是手写业务代码目录；通常可重新生成，但如果其中内容已被 Git 跟踪，清理前要先确认版本库状态。 |

## 6. `docs/` 目录逐文件说明

`docs/` 负责“把代码讲清楚”，不是“重新写一份代码”。

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`docs/.gitkeep`](docs/.gitkeep) | 占位文件。 | 最早用于保证空 `docs/` 目录也能被 Git 跟踪；即使后来目录不空，它保留也无害。 |
| [`docs/architecture.md`](docs/architecture.md) | 复用了 `claude.md` 里的 5 张 Mermaid 图：思维导图、分层图、类关系图、级联删除图、登录与权限流程图；每张图旁边都加了“当前仓库哪份文件对应哪条结构”的对证说明。末尾还有“Qt 适配讨论”。 | 当你要说明项目整体结构、Qt 会改哪一层时，先看它。 |
| [`docs/defense.md`](docs/defense.md) | 一份答辩问答稿，共 14 条核心 Q&A，围绕四层分层、显式序列化、权限矩阵、测试入口、Qt 复用边界等高频问题组织。每条都附当前文件与行号。 | 准备答辩表述时看它。 |
| [`docs/test-cases.md`](docs/test-cases.md) | Week 13 的测试总表，按 A-F 六组组织：认证与会话、字段校验、唯一性、级联删除残留、教师白名单、文件损坏。文末附日志证据与设计取舍备注。 | 当你要说明除正常路径外还覆盖了哪些边界和异常场景时，看它。 |
| [`docs/cli-demo-guide.md`](docs/cli-demo-guide.md) | CLI 演示的逐步操作说明，包含输入内容、预期输出、演示顺序和清理步骤。 | 课堂现场演示控制台版时可按步骤执行。 |
| [`docs/introduceQt-acceptance.md`](docs/introduceQt-acceptance.md) | `introduceQt` GUI 的当前验收记录。 | 用来查看当前 GUI 验收状态和待确认项。 |

说明：`docs/cli-demo-guide.md` 和 `docs/introduceQt-acceptance.md` 是当前工作区新增文档；如果要作为正式交付内容提交，需要确认它们已纳入版本库。

## 7. `include/EduSys/` 目录逐级说明

这一层代表**接口面**。它告诉你系统有哪些概念、有哪些服务、有哪些菜单，但不负责把所有细节写出来。

对 C++ 初学者可以这样理解：多数 `.hpp` 文件负责声明类、函数和依赖关系，真正执行逻辑通常在 `src/` 目录里的同名或对应 `.cpp` 文件中。例外是模板类，例如 `BinaryRepository.hpp`，模板代码通常必须放在头文件里，编译器才能为不同实体类型生成对应实现。

另外，目录里的 `.gitkeep` 不是程序文件，不参与编译，也不会被 `#include`。它只是一个占位文件，用来让 Git 在目录还没有正式文件时也能保留这个目录；后来目录里有真实文件后，`.gitkeep` 保留也不会影响程序。

### 7.1 `include/EduSys/app/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/app/.gitkeep`](include/EduSys/app/.gitkeep) | 预留占位文件。 | 保留应用装配层目录。 |
| [`include/EduSys/app/AppContext.hpp`](include/EduSys/app/AppContext.hpp) | 声明共享运行时上下文：仓储对象、服务对象、报表导出器，以及 `initializeData()`。 | CLI 和 Qt 两条入口都依赖它做统一装配。 |

### 7.2 `include/EduSys/common/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/common/Constants.hpp`](include/EduSys/common/Constants.hpp) | 集中定义 `USERS_FILE`、`STUDENTS_FILE`、`DATA_DIR` 等路径常量，以及 `SCORE_MIN`、`SCORE_MAX`、`SCORE_PASS`、`USUAL_WEIGHT` 等业务常量。 | 防止路径、阈值、权重在多处重复写死。 |
| [`include/EduSys/common/Exception.hpp`](include/EduSys/common/Exception.hpp) | 定义异常层次：`EduException`、`StorageException`、`AuthException`、`ValidationException`、`PermissionException`。 | 给上层一个清晰的“错误语义分类”：是存储坏了、认证失败了、字段非法了、还是越权了。 |
| [`include/EduSys/common/Logger.hpp`](include/EduSys/common/Logger.hpp) | 定义 `Logger` 单例与 `LogLevel` 枚举，暴露 `info/warn/error` 接口。 | 给整个项目统一日志入口。 |
| [`include/EduSys/common/PasswordHasher.hpp`](include/EduSys/common/PasswordHasher.hpp) | 声明 `PasswordHasher::hash` 与 `PasswordHasher::verify` 两个静态方法。 | 对外表达“密码不以明文落盘”，同时明确这不是生产级密码学方案。 |
| [`include/EduSys/common/Types.hpp`](include/EduSys/common/Types.hpp) | 定义 `using Id = std::string;` 与 `enum class RoleType`。 | 统一角色枚举和对象编号语义。 |

### 7.3 `include/EduSys/model/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/model/Person.hpp`](include/EduSys/model/Person.hpp) | 抽象出 `id`、`name`、`contact` 三个公共人员字段，并声明纯虚函数 `roleLabel()`。 | 提供 `Student` 与 `Teacher` 的公共基类，但不把密码、菜单、统计等职责塞进去。 |
| [`include/EduSys/model/Student.hpp`](include/EduSys/model/Student.hpp) | 在 `Person` 基础上增加 `major`、`className`、`enrollYear`，并声明 `writeTo/readFrom`。 | 表达学生实体，同时约定它能被显式序列化。 |
| [`include/EduSys/model/Teacher.hpp`](include/EduSys/model/Teacher.hpp) | 在 `Person` 基础上增加 `department`、`title`，并声明 `writeTo/readFrom`。 | 表达教师实体。 |
| [`include/EduSys/model/UserAccount.hpp`](include/EduSys/model/UserAccount.hpp) | 定义登录账号对象，包含 `username`、`passwordHash`、`role`、`ownerId`、`enabled`。 | 把“登录账号”从 `Person` 体系中独立出来，解决管理员账号不对应具体人的问题。 |
| [`include/EduSys/model/Course.hpp`](include/EduSys/model/Course.hpp) | 定义 `courseId`、`courseName`、`credit`、`teacherId`、`semester`。 | 课程实体只通过 `teacherId` 关联教师，不直接持有教师对象。 |
| [`include/EduSys/model/Score.hpp`](include/EduSys/model/Score.hpp) | 定义 `studentId + courseId + semester` 三元组及三类分数。 | 成绩是独立关联实体，不嵌在 `Student` 里，便于统计、删除和持久化。 |

### 7.4 `include/EduSys/service/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/service/.gitkeep`](include/EduSys/service/.gitkeep) | 占位文件。 | 保留初始骨架结构。 |
| [`include/EduSys/service/AuthService.hpp`](include/EduSys/service/AuthService.hpp) | 暴露 `authenticate()` 与 `changePassword()`。 | 所有用户名/密码校验都必须走这里，避免各层自己处理认证。 |
| [`include/EduSys/service/CourseService.hpp`](include/EduSys/service/CourseService.hpp) | 暴露课程 `listAll/findById/create/update/remove`。 | 课程读写与级联删除规则的唯一入口。 |
| [`include/EduSys/service/ScoreService.hpp`](include/EduSys/service/ScoreService.hpp) | 暴露成绩查询、按学生查、按课程查、`upsert`、删除单条成绩。 | 成绩权限矩阵、同键覆写策略、范围校验都在这一层统一定义。 |
| [`include/EduSys/service/Session.hpp`](include/EduSys/service/Session.hpp) | 定义轻量会话值对象：`loggedIn`、`username`、`role`、`ownerId`。 | 它不读写文件、不持有仓储，只描述“当前是谁”。 |
| [`include/EduSys/service/StatsService.hpp`](include/EduSys/service/StatsService.hpp) | 定义 `GpaResult`、`CourseStats`、`RankEntry`、`WarningEntry` 四种结构化结果，并暴露 GPA、课程统计、排名、预警计算接口。 | 这是统计层的核心接口，重要特点是“只返回结构化结果，不负责排版和写文件”。 |
| [`include/EduSys/service/StudentService.hpp`](include/EduSys/service/StudentService.hpp) | 暴露学生的读写、查询、删除。 | 学生删除时的级联清理范围在这里被正式定义。 |

### 7.5 `include/EduSys/storage/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/storage/.gitkeep`](include/EduSys/storage/.gitkeep) | 占位文件。 | 保留目录结构。 |
| [`include/EduSys/storage/BinaryReader.hpp`](include/EduSys/storage/BinaryReader.hpp) | 声明 `readUint8/readInt32/readUint32/readDouble/readString/readBytes`。 | 二进制显式解码器。 |
| [`include/EduSys/storage/BinaryWriter.hpp`](include/EduSys/storage/BinaryWriter.hpp) | 声明 `writeUint8/writeInt32/writeUint32/writeDouble/writeString/writeBytes`。 | 二进制显式编码器。 |
| [`include/EduSys/storage/BinaryRepository.hpp`](include/EduSys/storage/BinaryRepository.hpp) | 模板仓储基类，内置统一文件头 `magic + version + count`，并封装 `loadAll/saveAll`。 | 这是所有 `.dat` 仓储的核心基础设施。 |
| [`include/EduSys/storage/StudentRepository.hpp`](include/EduSys/storage/StudentRepository.hpp) | 把 `BinaryRepository<Student>` 绑定到 `data/students.dat`。 | 学生表仓储。 |
| [`include/EduSys/storage/TeacherRepository.hpp`](include/EduSys/storage/TeacherRepository.hpp) | 把 `BinaryRepository<Teacher>` 绑定到 `data/teachers.dat`。 | 教师表仓储。 |
| [`include/EduSys/storage/UserRepository.hpp`](include/EduSys/storage/UserRepository.hpp) | 把 `BinaryRepository<UserAccount>` 绑定到 `data/users.dat`。 | 账号表仓储。 |
| [`include/EduSys/storage/CourseRepository.hpp`](include/EduSys/storage/CourseRepository.hpp) | 把 `BinaryRepository<Course>` 绑定到 `data/courses.dat`。 | 课程表仓储。 |
| [`include/EduSys/storage/ScoreRepository.hpp`](include/EduSys/storage/ScoreRepository.hpp) | 把 `BinaryRepository<Score>` 绑定到 `data/scores.dat`。 | 成绩表仓储。 |

这里五个具体仓储通常没有单独 `.cpp` 文件，是因为它们只是“把某种实体绑定到某个 `.dat` 路径”。真正的读写流程在 `BinaryRepository.hpp`：它负责统一文件头、`loadAll/saveAll`；读写字段时再调用实体自己的 `T::readFrom()` 和 `writeTo()`；底层字节读写由 `BinaryReader.cpp` 和 `BinaryWriter.cpp` 完成。

### 7.6 `include/EduSys/view/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/view/.gitkeep`](include/EduSys/view/.gitkeep) | 占位文件。 | 保留目录结构。 |
| [`include/EduSys/view/BaseMenu.hpp`](include/EduSys/view/BaseMenu.hpp) | 抽象菜单基类，声明 `readLine/readInt/readDouble/printTable/paginate/printError/printOk`。 | 明确规定：`std::cin/std::cout` 只允许集中出现在 View 层工具里。 |
| [`include/EduSys/view/AdminMenu.hpp`](include/EduSys/view/AdminMenu.hpp) | 管理员菜单类，持有 `Session` 与各类服务引用。 | 把 Admin 可见的所有操作编排成菜单流程。 |
| [`include/EduSys/view/TeacherMenu.hpp`](include/EduSys/view/TeacherMenu.hpp) | 教师菜单类，额外声明 `pickOwnCourseId()` 白名单选择器。 | 在 View 层先做一层“我的课程”过滤，再交给 Service 层权限校验。 |
| [`include/EduSys/view/StudentMenu.hpp`](include/EduSys/view/StudentMenu.hpp) | 学生菜单类。 | 负责“只读自己”的交互展示。 |

### 7.7 `include/EduSys/report/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/report/ReportExporter.hpp`](include/EduSys/report/ReportExporter.hpp) | 暴露 `exportWarningReport`、`exportCourseStatsCsv`、`exportRankingCsv` 三个导出接口。 | 这是一个独立于 `service/`、`view/`、`storage/` 的轻量适配器，只负责把统计结果变成文本或 CSV 文件。 |

### 7.8 `include/EduSys/gui/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/gui/LoginDialog.hpp`](include/EduSys/gui/LoginDialog.hpp) | Qt 登录对话框，持有 `AppContext`、`Session`、用户名/密码输入框与失败计数。 | GUI 版的第一个入口。 |
| [`include/EduSys/gui/AdminWindow.hpp`](include/EduSys/gui/AdminWindow.hpp) | 管理员主窗口，6 个页签：学生、课程、成绩、统计、报告、账户。 | 把 CLI 的 Admin 菜单换成 Qt 页签。 |
| [`include/EduSys/gui/TeacherWindow.hpp`](include/EduSys/gui/TeacherWindow.hpp) | 教师主窗口，4 个页签：我的课程、我的课程成绩、我的课程统计、账户。 | 对应 CLI 的 Teacher 菜单。 |
| [`include/EduSys/gui/StudentWindow.hpp`](include/EduSys/gui/StudentWindow.hpp) | 学生主窗口，4 个页签：我的资料、我的成绩、我的 GPA、账户。 | 对应 CLI 的 Student 菜单。 |
| [`include/EduSys/gui/StudentEditDialog.hpp`](include/EduSys/gui/StudentEditDialog.hpp) | 学生新增/编辑对话框。 | 管理员修改学生信息时使用。 |
| [`include/EduSys/gui/CourseEditDialog.hpp`](include/EduSys/gui/CourseEditDialog.hpp) | 课程新增/编辑对话框。 | 管理员修改课程信息时使用。 |
| [`include/EduSys/gui/ScoreEditDialog.hpp`](include/EduSys/gui/ScoreEditDialog.hpp) | 成绩录入/编辑对话框，带 `setCourseIdLocked()`。 | 教师/管理员录分时使用。 |
| [`include/EduSys/gui/ChangePasswordDialog.hpp`](include/EduSys/gui/ChangePasswordDialog.hpp) | 改密码对话框。 | 三类角色共用。 |

说明：`ChangePasswordDialog.hpp`、`CourseEditDialog.hpp`、`ScoreEditDialog.hpp` 是当前工作区新增的 Qt GUI 头文件，当前未跟踪，属于 `introduceQt` 分支待确认提交内容。它们已被 `CMakeLists.txt` 的 `edusys_gui` 目标引用；如果不提交，别人拉取分支后可能无法完整构建 GUI。

## 8. `src/` 目录逐级说明

`src/` 存放具体实现，包含业务逻辑、交互流程、文件读写和报表导出。

### 8.1 `src/app/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/app/AppContext.cpp`](src/app/AppContext.cpp) | 统一构造仓储、服务和导出器；`initializeData()` 会检查 `data/` 是否存在并决定是否 seed。 | CLI 和 GUI 两条入口都复用这一层装配。 |
| [`src/app/gui_main.cpp`](src/app/gui_main.cpp) | Qt 版入口，初始化 `QApplication`，弹出 `LoginDialog`，按角色创建 `AdminWindow` / `TeacherWindow` / `StudentWindow`，并在主窗口关闭后回到登录框。 | `introduceQt` 分支的 GUI 主循环。 |
| [`src/app/main.cpp`](src/app/main.cpp) | CLI 入口，负责 `--self-test`、登录循环、角色分发，以及调用 `AdminMenu` / `TeacherMenu` / `StudentMenu`。 | `edusys.exe` 的主入口。 |

### 8.2 `src/common/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/common/Logger.cpp`](src/common/Logger.cpp) | 实现 `Logger` 单例，生成时间戳，按 `INFO/WARN/ERROR` 级别同步追加写入 `data/app.log`。 | 让所有关键事件都有落盘证据，尤其适合自检与损坏恢复场景。 |
| [`src/common/PasswordHasher.cpp`](src/common/PasswordHasher.cpp) | 用固定盐 + `FNV-1a 64-bit` 生成 16 字符十六进制摘要，并实现密码比对。 | 这是课程环境下“避免明文落盘”的轻量方案，不追求生产级密码学安全。 |

### 8.3 `src/model/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/model/Student.cpp`](src/model/Student.cpp) | 实现学生构造、字段 setter、`roleLabel()`，以及学生对象的显式 `writeTo/readFrom`。 | 让 `Student` 可以通过仓储持久化。 |
| [`src/model/Teacher.cpp`](src/model/Teacher.cpp) | 实现教师构造、字段 setter、`roleLabel()`，以及显式序列化。 | 与 `Student` 形成对称实现。 |
| [`src/model/UserAccount.cpp`](src/model/UserAccount.cpp) | 实现账号对象构造、密码哈希更新、启停状态、`ownerId` 写入与反序列化。 | 登录账号的实际落盘格式在这里定义。 |
| [`src/model/Course.cpp`](src/model/Course.cpp) | 实现课程构造、字段修改与显式序列化。 | 课程记录的 `.dat` 布局在这里确定。 |
| [`src/model/Score.cpp`](src/model/Score.cpp) | 实现成绩构造与显式序列化。 | 定义 `(studentId, courseId, semester, usual, final, total)` 如何存盘。 |

### 8.4 `src/service/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/service/.gitkeep`](src/service/.gitkeep) | 占位文件。 | 历史骨架保留。 |
| [`src/service/AuthService.cpp`](src/service/AuthService.cpp) | 实现账号查找、启用状态检查、密码验证、认证失败日志、修改密码逻辑。 | 认证和密码修改统一经由此层处理。 |
| [`src/service/StudentService.cpp`](src/service/StudentService.cpp) | 实现学生字段校验、权限检查、创建/修改/查询；删除时按顺序清成绩、清学生账号、再删学生本体。 | 删除学生后是否会留下残留账号和成绩记录，相关处理主要在这里实现。 |
| [`src/service/CourseService.cpp`](src/service/CourseService.cpp) | 实现课程字段校验、教师存在性校验、创建/修改/查询；删除课程时级联清成绩。 | 管理课程数据的一致性。 |
| [`src/service/ScoreService.cpp`](src/service/ScoreService.cpp) | 实现成绩范围校验、学生/课程存在性校验、角色权限矩阵、教师仅能操作自己课程、同键 `upsert`、删除单条成绩。 | 成绩相关权限和业务规则主要集中在这里。 |
| [`src/service/StatsService.cpp`](src/service/StatsService.cpp) | 实现 GPA 分段映射、课程平均分、及格率、优秀率、排名、预警汇总。所有接口都只返回结构化数据，不做任何 `cout` 或文件写入。 | 这是“统计逻辑与表现层解耦”的关键文件。 |

### 8.5 `src/storage/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/storage/.gitkeep`](src/storage/.gitkeep) | 占位文件。 | 目录骨架保留。 |
| [`src/storage/BinaryReader.cpp`](src/storage/BinaryReader.cpp) | 实现显式解码，包含 `readString()` 的长度上限保护和 `Unexpected EOF` 检查。 | 负责从二进制文件安全读回结构化字段。 |
| [`src/storage/BinaryWriter.cpp`](src/storage/BinaryWriter.cpp) | 实现显式编码，失败时立即抛 `StorageException`。 | 负责把结构化字段写回二进制文件。 |

### 8.6 `src/view/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/view/.gitkeep`](src/view/.gitkeep) | 占位文件。 | 保留目录结构。 |
| [`src/view/BaseMenu.cpp`](src/view/BaseMenu.cpp) | 实现统一输入、数值解析、分页打印、表格输出、标题分隔线、成功/失败消息。 | 所有菜单都共享同一套 CLI 交互工具，避免每个菜单重复实现输入输出逻辑。 |
| [`src/view/AdminMenu.cpp`](src/view/AdminMenu.cpp) | 实现管理员主菜单及其子菜单：学生管理、课程管理、成绩管理、统计、预警报告、CSV 导出、改密码。 | Admin 的交互编排集中在这里。 |
| [`src/view/TeacherMenu.cpp`](src/view/TeacherMenu.cpp) | 实现教师菜单，并通过 `pickOwnCourseId()` 先过滤“我的课”，再允许录分、删分、查统计。 | View 层白名单防线所在文件。 |
| [`src/view/StudentMenu.cpp`](src/view/StudentMenu.cpp) | 实现学生个人资料、个人成绩、个人 GPA、修改密码四类只读/自助操作。 | 学生端交互入口。 |

### 8.7 `src/report/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/report/ReportExporter.cpp`](src/report/ReportExporter.cpp) | 实现三类导出：`warning_report.txt`、课程统计 CSV、课程排名 CSV。这里包含当前时间格式化、CSV 转义、课程编号安全性检查、输出文件落盘与日志记录。 | 把 `StatsService` 的结构化结果转换成最终文件，是一个适配器式实现。 |

### 8.8 `src/gui/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/gui/LoginDialog.cpp`](src/gui/LoginDialog.cpp) | Qt 登录对话框，三次失败后退出，用户名留空直接结束启动。 | GUI 版的登录入口。 |
| [`src/gui/AdminWindow.cpp`](src/gui/AdminWindow.cpp) | 管理员主窗口，6 个页签，内部用 `QTableWidget`、编辑对话框和 `QMessageBox` 串起学生/课程/成绩/统计/报告/账户功能。 | CLI Admin 菜单的桌面版。 |
| [`src/gui/TeacherWindow.cpp`](src/gui/TeacherWindow.cpp) | 教师主窗口，4 个页签，课程选择先过白名单，再调用成绩与统计服务。 | CLI Teacher 菜单的桌面版。 |
| [`src/gui/StudentWindow.cpp`](src/gui/StudentWindow.cpp) | 学生主窗口，4 个页签，展示个人资料、成绩、GPA 和账户操作。 | CLI Student 菜单的桌面版。 |
| [`src/gui/StudentEditDialog.cpp`](src/gui/StudentEditDialog.cpp) | 学生新增/编辑窗口，负责表单校验和对象回填。 | 管理员编辑学生时复用。 |
| [`src/gui/CourseEditDialog.cpp`](src/gui/CourseEditDialog.cpp) | 课程新增/编辑窗口，负责课程表单校验和对象回填。 | 管理员编辑课程时复用。 |
| [`src/gui/ScoreEditDialog.cpp`](src/gui/ScoreEditDialog.cpp) | 成绩新增/编辑窗口，支持锁定课程号并校验分数字段。 | 管理员/教师录分时复用。 |
| [`src/gui/ChangePasswordDialog.cpp`](src/gui/ChangePasswordDialog.cpp) | 改密码窗口，先做三项表单校验，再调用 `AuthService::changePassword()`。 | 三类角色共用的账户操作窗口。 |

说明：`ChangePasswordDialog.cpp`、`CourseEditDialog.cpp`、`ScoreEditDialog.cpp` 是当前工作区新增的 Qt GUI 实现文件，当前未跟踪，属于 `introduceQt` 分支待确认提交内容。它们已被 `CMakeLists.txt` 的 `edusys_gui` 目标引用；如果不提交，别人拉取分支后可能无法完整构建 GUI。

## 9. `tools/` 目录逐文件说明

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`tools/corrupt_check.bat`](tools/corrupt_check.bat) | 这是 Week 13 F 组脚本。它会先检查根目录 `edusys.exe` 是否存在且是否可能过旧，再验证 `data/*.dat` 是否齐全；之后做 `.dat` 快照备份，依次制造 3 类文件损坏场景：删除 `scores.dat`、把 `users.dat` 头的 `EDSY` 改成 `XXXX`、把 `scores.dat` 计数字段伪造为 `255`；每次都运行 `edusys.exe --self-test` 捕获输出并恢复现场；最后再跑一次 sanity 自检。 | 损坏数据会被识别，脚本会恢复原文件并记录输出。 |

当前工作区里的 `tools/build_detailed_weekly_journal_xlsx.py` 是周记 Excel 生成脚本，属于课程过程材料辅助工具，不是学生成绩管理系统的核心验证脚本；如果要提交，需要单独确认。

这个脚本生成的是 `weeklog.xlsx`、`weekly_report.xlsx` 或 `大作业周记（详细认真版）.xlsx` 这类过程材料。它们不是系统输入文件，`edusys.exe` 不会读取这些 Excel，也不会因为它们存在或不存在而改变功能。

## 10. `data/` 目录逐文件说明

`data/` 不是源码目录，但它是程序运行时的主要状态目录。

先按用途分一下：

| 类别 | 文件 | 是否需要手工维护 |
| --- | --- | --- |
| 业务状态 | `users.dat`、`students.dat`、`teachers.dat`、`courses.dat`、`scores.dat` | 通常不手工改，由程序读写。 |
| 运行日志 | `app.log` | 程序自动追加，答辩或排查时可查看。 |
| 导出结果 | `warning_report.txt`、`course_stats_<courseId>.csv`、`ranking_<courseId>.csv` | 由 Admin 菜单导出，重复导出会覆盖同名文件。 |
| 损坏测试输出 | `__corrupt_out__/F1.out` 等 | 由 `tools/corrupt_check.bat` 生成，用来留存 F 组测试输出。 |
| 自检捕获留痕 | `__selftest_*.txt`、`__root_selftest_*.txt` | 由外部脚本或人工重定向留下，不是每次运行程序都会自动生成。 |

### 10.1 五个核心 `.dat` 文件

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`data/users.dat`](data/users.dat) | 存放登录账号：用户名、密码哈希、角色、`ownerId`、启用状态。 | 没有它就无法认证登录。 |
| [`data/students.dat`](data/students.dat) | 存放学生实体：学号、姓名、联系方式、专业、班级、入学年份。 | 学生管理的持久化载体。 |
| [`data/teachers.dat`](data/teachers.dat) | 存放教师实体：工号、姓名、联系方式、院系、职称。 | 教师存在性校验依赖它。 |
| [`data/courses.dat`](data/courses.dat) | 存放课程实体：课程号、课程名、学分、授课教师编号、学期。 | 课程管理和成绩外键校验依赖它。 |
| [`data/scores.dat`](data/scores.dat) | 存放成绩记录：学生编号、课程编号、学期、平时分、期末分、总评。 | GPA、统计、排名、预警的底层数据来源。 |

### 10.2 运行日志与导出结果

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`data/app.log`](data/app.log) | 记录程序启动、认证成功/失败、创建学生、删除课程、导出报告、自检成功、Fatal 错误等事件。 | 自检、答辩、排障三用。 |
| [`data/warning_report.txt`](data/warning_report.txt) | 学业预警文本报告，包括生成时间、操作人、阈值、命中学生列表。 | Admin 菜单第 5 项产物。 |
| `data/course_stats_<courseId>.csv` | 某门课程的统计 CSV，例如当前工作区可能存在 `course_stats_C001.csv` 或 `course_stats_C900.csv`。 | Admin 菜单第 7 项产物之一，具体文件名由导出时输入的课程号决定。 |
| `data/ranking_<courseId>.csv` | 某门课程的排名 CSV，例如当前工作区可能存在 `ranking_C001.csv` 或 `ranking_C900.csv`。 | Admin 菜单第 7 项产物之一，具体文件名由导出时输入的课程号决定。 |

### 10.3 `__corrupt_out__/` 文件损坏测试输出

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`data/__corrupt_out__/F1.out`](data/__corrupt_out__/F1.out) | 删除 `scores.dat` 后运行 `--self-test` 的捕获输出。 | 对应 F1 用例。 |
| [`data/__corrupt_out__/F2.out`](data/__corrupt_out__/F2.out) | 篡改 `users.dat` magic 后运行 `--self-test` 的捕获输出。 | 对应 F2 用例。 |
| [`data/__corrupt_out__/F3.out`](data/__corrupt_out__/F3.out) | 伪造 `scores.dat` count 字段后的捕获输出。 | 对应 F3 用例。 |
| [`data/__corrupt_out__/sanity.out`](data/__corrupt_out__/sanity.out) | 三个损坏场景恢复后再次跑 `--self-test` 的输出。 | 记录恢复后的自检结果。 |

补充说明：

- `data/__corrupt_backup__/` 是 `corrupt_check.bat` 运行时临时创建的备份目录，脚本结束后会删除
- `.dat` 文件都是二进制格式，不建议手工修改

### 10.4 自检捕获与状态文件

这些文件是 `--self-test` 及其补丁后重跑的捕获结果和退出状态记录，主要用来留痕，不是业务数据，也不是手工维护内容。需要注意：`edusys.exe --self-test` 本身主要打印到控制台并写 `data/app.log`；下面这些 `.txt` 多数是外部脚本或人工重定向保存下来的结果，不是每次运行自检都必然生成。

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`data/__selftest_status__.txt`](data/__selftest_status__.txt) | 记录一次自检运行的退出码状态。 | 最小化的自检结果标记。 |
| [`data/__selftest_capture__.txt`](data/__selftest_capture__.txt) | 记录一次自检运行的捕获输出。 | 自检正文留痕。 |
| [`data/__selftest_after_patch__.txt`](data/__selftest_after_patch__.txt) | 记录补丁后的自检捕获输出。 | 用来确认打补丁后自检仍然通过。 |
| [`data/__selftest_after_patch_status__.txt`](data/__selftest_after_patch_status__.txt) | 记录补丁后的自检退出码。 | 与上一个文件配对使用。 |
| [`data/__root_selftest_after_patch__.txt`](data/__root_selftest_after_patch__.txt) | 记录仓库根目录执行自检时的捕获输出。 | 证明从根目录调用也能正常回归。 |
| [`data/__root_selftest_after_patch_status__.txt`](data/__root_selftest_after_patch_status__.txt) | 记录仓库根目录执行自检时的退出码。 | 与上一个文件配对使用。 |

## 11. `build/` 目录说明

`build/` 目录是 **CMake / Visual Studio 自动生成目录**，不属于手写业务逻辑。它通常可以重新生成；但如果其中部分文件已经被 Git 跟踪，直接删除会表现为大量删除记录，清理前要先看 `git status`。

考虑到里面文件很多，而且大多数只是 IDE / 编译系统的中间产物，这里按类别说明，而不是把每个 `.obj` 和 `.tlog` 当成业务文件来解释。

| 文件 / 子目录 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`build/EduSys.sln`](build/EduSys.sln) | Visual Studio 解决方案文件。 | 用 VS 打开整个项目时的入口。 |
| [`build/edusys.vcxproj`](build/edusys.vcxproj) | 主可执行项目的 MSVC 工程文件。 | 告诉 VS 如何编译 `edusys`。 |
| [`build/edusys.vcxproj.filters`](build/edusys.vcxproj.filters) | VS 过滤器定义。 | 控制 Solution Explorer 里的显示分组。 |
| [`build/ALL_BUILD.vcxproj`](build/ALL_BUILD.vcxproj) | CMake 自动生成的聚合目标。 | 一次性构建整个解决方案。 |
| [`build/ALL_BUILD.vcxproj.filters`](build/ALL_BUILD.vcxproj.filters) | 上述聚合目标的 VS 过滤器。 | IDE 辅助文件。 |
| [`build/ZERO_CHECK.vcxproj`](build/ZERO_CHECK.vcxproj) | CMake 自动生成的检查目标。 | 重新判断 `CMakeLists.txt` 是否变化、是否需要重配。 |
| [`build/ZERO_CHECK.vcxproj.filters`](build/ZERO_CHECK.vcxproj.filters) | 对应的过滤器。 | IDE 辅助文件。 |
| [`build/CMakeCache.txt`](build/CMakeCache.txt) | CMake 配置缓存。 | 记录编译器路径、生成器选项、变量值。 |
| [`build/cmake_install.cmake`](build/cmake_install.cmake) | CMake 安装脚本。 | 只有在执行安装动作时才有意义。 |
| [`build/CMakeFiles/`](build/CMakeFiles) | 编译器探测、配置日志、规则文件、stamp 文件等。 | CMake 内部工作区。 |
| [`build/edusys.dir/`](build/edusys.dir) | 对象文件、链接日志、增量编译跟踪。 | 主项目的构建中间目录。 |
| [`build/x64/`](build/x64) | VS x64 配置下的中间产物与 recipe。 | 平台 / 配置级别的构建状态目录。 |

如果你是第一次读仓库，可以先把 `build/` 当成“自动生成、可以忽略细节”的目录；主要业务代码在 `src/` 和 `include/`。

### 11.1 Qt 构建目录

`build-qt-introduceQt/`、`build-qt-mingw/`、`build-qt-mingw2/`、`build-qt-gui-check/`、`build-qt-verify/` 等目录也属于构建生成目录。它们通常由 CMake、Ninja 和 Qt 工具链生成，里面会有 `.obj`、`.ninja_log`、`CMakeCache.txt`、自动生成的 Qt 部署脚本等。

需要注意：当前工作区里有一些 `build-qt-*` 文件已经被 Git 跟踪或处于修改状态。不要简单地整目录删除；清理前先运行：

```bash
git status --short
git ls-files build-qt-introduceQt
```

如果这些命令显示目录内文件已被跟踪，删除它们会在 Git 里表现为大量删除记录。除非明确要整理构建产物，否则不建议在交付前直接清空这些目录。

## 12. 代码结构与职责边界

仓库虽然文件不少，但架构逻辑可以概括为：

```text
用户 -> CLI View 菜单 / Qt GUI 窗口 -> Service 规则 -> Storage 仓储 -> .dat 文件
                                      |
                                      -> StatsService -> ReportExporter -> .txt/.csv
```

四条主要边界如下：

1. `view/` 和 `gui/` 只做交互，不直接读写 `.dat`
2. `service/` 集中权限、校验、级联和统计规则
3. `storage/` 只负责显式序列化，不做业务判断
4. `model/` 只描述数据结构，不混入菜单与算法

这四条边界正是 `introduceQt` 分支能把 GUI 和核心层顺利对接起来的关键。在保持这些边界的情况下，Qt 这条线可以把 `view/` 的控制台菜单换成 `src/gui/` 的窗口，并让 `src/app/gui_main.cpp` 接管主循环，而核心层继续原样复用。

## 13. 测试与验证入口

### 13.1 `edusys.exe --self-test`

这条入口覆盖两段内容：

- Week 11 端到端自检
  - 正确登录 `admin`
  - 错密码被拒绝
  - 学生角色越权创建学生被拒绝
  - 越界分数被拒绝
  - 首次 seed 时执行一次破坏性演化：新增 `S003`、更新 `S001`、删除 `S002`
- Week 13 A-E 只读边界验证
  - 认证 / 会话错误
  - 字段非法
  - 重复主键
  - 级联删除残留
  - 教师白名单双重防线

需要注意：`--self-test` 是自动回归入口，不是 CLI 菜单演示。它不会让你手工输入菜单编号。首次空数据运行时，它会触发 seed 后的样例演化，例如新增 `S003`、更新 `S001`、删除 `S002`；非首次运行时主要验证当前持久化数据是否符合预期。

补充一点：`main()` 对命令行参数还有严格检查，除了 `--self-test` 之外，其他参数都会直接报 `Unknown argument: ... (supported: --self-test)` 并退出。这个行为也是 CLI 参数校验的一部分。

### 13.2 `tools/corrupt_check.bat`

这条入口负责 F 组“坏文件”场景：

- F1：删除 `scores.dat`
- F2：把 `users.dat` 头四字节从 `EDSY` 改成 `XXXX`
- F3：把 `scores.dat` 的计数字段虚报为 `255`

这条脚本外置而不是内置进 `--self-test`，是为了让 `--self-test` 尽量保持只读，由外部脚本负责制造和恢复损坏场景。

### 13.3 `demo_input.txt`

它不是测试用例表，而是一条“演示脚本”：

- 先以 `admin` 登录，新增学生、录入成绩、查看统计、导出预警报告
- 再以 `t001` 登录，只查看并操作自己的课程
- 最后以 `s001` 登录，只看自己的资料、成绩、GPA

这条脚本可用于：

- 录屏
- 答辩前彩排
- 验证交互菜单是否仍然连贯

运行方式示例：

```bash
edusys.exe < demo_input.txt
```

这条命令会把 `demo_input.txt` 里的多行文本当作键盘输入喂给 `edusys.exe`。它走的是真实 CLI 交互，因此可能新增、修改或删除 `data/*.dat` 里的演示数据；它不是边界测试，也不等同于 `--self-test`。

这里需要把三个材料区分开：

| 材料 | 作用 |
| --- | --- |
| 本 README | 解释项目功能、代码链路和目录结构。 |
| `docs/cli-demo-guide.md` | 适合课堂手工演示，按步骤写了“输入什么、预计看到什么”。 |
| `demo_input.txt` | 适合把一串输入一次性喂给程序，检查主流程是否还能串起来。 |

所以如果老师要看“你怎么一步一步演示 CLI 主线功能和可选补充项”，优先打开 `docs/cli-demo-guide.md`；如果只是想快速复现一段流程，再考虑 `demo_input.txt`。

## 14. 数据格式与输出约定

### 14.1 `.dat` 文件头

所有 `.dat` 仓储都走统一头部格式：

```text
magic[4] = 'E' 'D' 'S' 'Y'
version  = uint32
count    = uint32
```

`uint32` 指 32 位无符号整数。当前仓储格式版本是 `1`，定义在 [`include/EduSys/storage/BinaryRepository.hpp`](include/EduSys/storage/BinaryRepository.hpp) 的 `kFormatVersion = 1u`。

这样做的原因有三个：

- 能尽早识别“这不是合法的 EduSys 数据文件”
- 如果后续升级格式，可以依赖 `version` 做兼容判断
- 所有实体都通过 `writeTo/readFrom` 显式逐字段编解码，不依赖对象内存布局

### 14.2 `warning_report.txt`

文本报告会输出：

- 生成时间
- 操作人
- 预警阈值
- 命中学生条目
- 每名学生的 GPA、挂科门数、挂科课程列表

### 14.3 CSV 导出

当前 CSV 导出有两类：

- `course_stats_<courseId>.csv`
- `ranking_<courseId>.csv`

课程统计 CSV 表头是：

```csv
CourseId,CourseName,Count,Avg,Max,Min,PassRatePct,ExcellentRatePct
C001,Data Structure,3,82.33,95.00,60.00,100.00,33.33
```

课程排名 CSV 表头是：

```csv
Rank,StudentId,StudentName,TotalScore
1,S001,Alice,95.00
```

两者都复用 `StatsService` 的结构化结果，不改任何统计口径。`ReportExporter.cpp` 里还额外做了：

- CSV 字段转义
- 文件名安全字符检查
- 导出日志记录

## 15. `introduceQt` 分支的目录边界现状

这里说明 GUI 与核心层的目录边界，也单独说明 Qt GUI 现在有哪些业务功能。前面的第 2 章已经把 CLI 和 Qt 的功能索引并列列出来；这一章再从 `introduceQt` 分支角度解释“这些 Qt 文件各自负责什么、为什么没有破坏原来的 CLI”。

### 15.1 当前界面层分工

CLI 侧：

- `src/view/*.cpp` 与 `include/EduSys/view/*.hpp` 仍然是 CLI 菜单层
- `src/app/main.cpp` 继续负责 CLI 主循环与菜单分派

GUI 侧：

- `src/app/gui_main.cpp` 负责 Qt 主循环
- `src/gui/*.cpp` 与 `include/EduSys/gui/*.hpp` 负责 Qt 窗口、对话框和控件逻辑

### 15.2 当前不需要动的部分

- `include/EduSys/model/*` 与 `src/model/*`
- `include/EduSys/storage/*` 与 `src/storage/*`
- `include/EduSys/service/*` 与 `src/service/*`
- `include/EduSys/report/ReportExporter.hpp`
- `src/report/ReportExporter.cpp`
- `Session`、异常体系、日志体系、常量定义

换句话说，当前仓库已经把“会跟界面变化的部分”和“不会跟界面变化的部分”分开了。`introduceQt` 分支就是沿着这条边界把控制台 View 换成 Qt Widgets，而不是去动核心业务层。

### 15.3 Qt GUI 的构建边界

CLI 备用构建可以走 `build.bat`。Qt GUI 不走 `build.bat`，而是由 CMake 目标 `edusys_gui` 构建。

这样分开是因为 Qt 需要 `find_package(Qt6 REQUIRED COMPONENTS Widgets)`、`Qt6::Widgets` 链接和 Qt 运行时环境，强行塞进 `build.bat` 会让原本简单的 CLI 构建路径变复杂。

`edusys_gui` 目标受 `EDUSYS_BUILD_GUI` 开关控制，默认是 `ON`。普通构建不用管这个开关；如果配置 CMake 时手动传过 `-DEDUSYS_BUILD_GUI=OFF`，就不会生成 GUI 目标。

`CMakeLists.txt` 的分工如下。第一次读这里时只需要看分组，不需要逐个记住每个文件名：

```cmake
set(EDUSYS_CORE_SOURCES
    src/app/AppContext.cpp
    src/common/Logger.cpp
    src/common/PasswordHasher.cpp
    src/model/Student.cpp
    src/model/Teacher.cpp
    src/model/UserAccount.cpp
    src/model/Course.cpp
    src/model/Score.cpp
    src/report/ReportExporter.cpp
    src/service/AuthService.cpp
    src/service/StudentService.cpp
    src/service/CourseService.cpp
    src/service/ScoreService.cpp
    src/service/StatsService.cpp
    src/storage/BinaryReader.cpp
    src/storage/BinaryWriter.cpp
)
```

这一组是共用核心层。CLI 和 GUI 都链接它，所以学生、课程、成绩、统计、报表、密码、文件读写不会分成两套。

```cmake
set(EDUSYS_CLI_SOURCES
    src/app/main.cpp
    src/view/BaseMenu.cpp
    src/view/AdminMenu.cpp
    src/view/TeacherMenu.cpp
    src/view/StudentMenu.cpp
)

add_executable(edusys ${EDUSYS_CLI_SOURCES})
target_link_libraries(edusys PRIVATE edusys_core)
```

这一组是 CLI 入口。它包含 `src/view/` 菜单文件，所以能运行控制台菜单和 `--self-test`。

```cmake
set(EDUSYS_GUI_SOURCES
    src/app/gui_main.cpp
    src/gui/LoginDialog.cpp
    src/gui/AdminWindow.cpp
    src/gui/StudentEditDialog.cpp
    src/gui/CourseEditDialog.cpp
    src/gui/ScoreEditDialog.cpp
    src/gui/ChangePasswordDialog.cpp
    src/gui/TeacherWindow.cpp
    src/gui/StudentWindow.cpp
)

if(EDUSYS_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(edusys_gui WIN32 ${EDUSYS_GUI_SOURCES})
    target_link_libraries(edusys_gui PRIVATE edusys_core Qt6::Widgets)
endif()
```

这一组是 GUI 入口。它包含 `src/gui/` 窗口文件，链接 `Qt6::Widgets`。这里没有把 `src/view/*.cpp` 加进来，说明 GUI 不依赖 CLI 菜单；同样，CLI 目标也没有把 `src/gui/*.cpp` 加进去，说明 CLI 不依赖 Qt。

实际构建时，在已经配置好 Qt MinGW 环境的前提下，可以使用下面这种命令。下面路径是本机示例，不要求每台电脑完全一样：

```bash
D:\Qt\Tools\CMake_64\bin\cmake.exe -S . -B build-qt-introduceQt -G Ninja -DCMAKE_PREFIX_PATH=D:\Qt\6.11.0\mingw_64 -DCMAKE_MAKE_PROGRAM=D:\Qt\Tools\Ninja\ninja.exe -DCMAKE_CXX_COMPILER=D:\Qt\Tools\mingw1310_64\bin\g++.exe
D:\Qt\Tools\Ninja\ninja.exe -C build-qt-introduceQt edusys edusys_gui
```

路径可以按本机 Qt 安装位置调整。关键不是固定某一个盘符，而是保证 `CMAKE_PREFIX_PATH` 指向 Qt 的 `mingw_64` 目录，`CMAKE_CXX_COMPILER` 指向同一套 Qt MinGW 的 `g++.exe`。

### 15.4 Qt GUI 的启动链路

GUI 的入口在 `src/app/gui_main.cpp`。启动顺序是：

1. 创建 `QApplication`。
2. 取得 `Logger`，开始写 `data/app.log`。
3. 创建 `AppContext`。
4. 调用 `appContext.initializeData()`，检查 `data/` 目录和数据文件状态，数据为空时写入默认示例数据。
5. 弹出 `LoginDialog`。
6. 登录成功后，根据 `Session` 角色创建 `AdminWindow`、`TeacherWindow` 或 `StudentWindow`。
7. 主窗口关闭后回到登录框。

有一个细节要注意：代码里先取得 `Logger`，而 `Logger` 会打开 `data/app.log`；之后才调用 `initializeData()`。仓库里保留了 `data/.gitkeep`，正常从项目目录运行时 `data/` 目录已经存在。如果手动把整个 `data/` 目录删掉，程序可能在打开日志时先失败，而不是等到 `initializeData()` 再创建目录。

对应代码片段是：

```cpp
EduSys::AppContext appContext;
const bool seededThisRun = appContext.initializeData();

if (seededThisRun) {
    QMessageBox::information(
        nullptr,
        QString::fromUtf8(u8"示例数据已初始化"),
        QString::fromUtf8(
            u8"检测到当前仓储为空，系统已写入默认示例账号：\n"
            u8"管理员：admin / admin123\n"
            u8"教师：t001 / t001pw\n"
            u8"学生：s001 / s001pw"));
}
```

这段说明 GUI 和 CLI 一样会走 `AppContext` 初始化。不同点是 GUI 用 `QMessageBox` 告诉用户样例数据已初始化，而 CLI 通常把信息打印在控制台或写日志。这里的初始化主要处理“数据文件为空时写入默认数据”，不是说它可以在任何情况下修复被删除的 `data/` 目录。

角色分发代码是：

```cpp
std::unique_ptr<QMainWindow> createRoleWindow(EduSys::AppContext& appContext,
                                              const EduSys::Session& session) {
    using namespace EduSys;

    switch (session.getRole()) {
        case RoleType::Admin:
            return std::make_unique<AdminWindow>(appContext, session);
        case RoleType::Teacher:
            return std::make_unique<TeacherWindow>(appContext, session);
        case RoleType::Student:
            return std::make_unique<StudentWindow>(appContext, session);
    }

    throw EduSys::EduException("Unsupported session role.");
}
```

这里没有重新判断用户名，也没有重新查账号。登录阶段已经生成 `Session`，角色窗口直接使用这个会话对象，也就是不再重新登录一次。CLI 也是一样的思路：认证完成后，后续菜单拿着 `Session` 调服务。

登录循环代码是：

```cpp
while (true) {
    EduSys::LoginDialog loginDialog(appContext);
    if (loginDialog.exec() != QDialog::Accepted) {
        logger.info("EduSys GUI shutdown normally (login cancelled).");
        break;
    }

    auto* mainWindow = createRoleWindow(appContext, loginDialog.session()).release();
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    mainWindow->show();

    QEventLoop eventLoop;
    QObject::connect(mainWindow, &QObject::destroyed, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
}
```

这里要注意两点：

- 登录框取消、空用户名退出或三次失败，会让 `loginDialog.exec()` 返回非 `Accepted`，然后 GUI 程序结束。
- 角色窗口关闭后，局部 `QEventLoop` 退出，但外层 `while` 没有结束，所以会再次出现登录框。

### 15.5 Qt 登录框 `LoginDialog`

`LoginDialog` 对应 CLI 的登录入口，但它不是把 CLI 输入函数包一层，而是自己使用 Qt 控件收集用户名和密码。它的声明在 `include/EduSys/gui/LoginDialog.hpp`：

```cpp
class LoginDialog : public QDialog {
public:
    explicit LoginDialog(AppContext& appContext, QWidget* parent = nullptr);

    const Session& session() const noexcept { return session_; }

private:
    void tryLogin();

    AppContext& appContext_;
    Session     session_;
    QLineEdit*  usernameEdit_ = nullptr;
    QLineEdit*  passwordEdit_ = nullptr;
    int         failedAttempts_ = 0;
};
```

这几个成员的含义是：

- `AppContext& appContext_` 用来拿到 `AuthService`。
- `Session session_` 用来保存登录成功后的用户、角色和 `ownerId`。
- `usernameEdit_` 和 `passwordEdit_` 是两个输入框。
- `failedAttempts_` 记录本次登录框里已经失败几次。

真实认证发生在 `src/gui/LoginDialog.cpp` 的 `tryLogin()`：

```cpp
try {
    const UserAccount account =
        appContext_.authService.authenticate(username.toStdString(), password.toStdString());
    session_.login(account.getUsername(), account.getRole(), account.getOwnerId());
    accept();
} catch (const AuthException& e) {
    ++failedAttempts_;
    QMessageBox::warning(
        this,
        QString::fromUtf8(u8"登录失败"),
        QString::fromUtf8(u8"%1\n\n当前失败次数：%2 / 3")
            .arg(errorText(e))
            .arg(failedAttempts_));

    passwordEdit_->clear();
    passwordEdit_->setFocus();

    if (failedAttempts_ >= 3) {
        QMessageBox::critical(
            this,
            QString::fromUtf8(u8"登录终止"),
            QString::fromUtf8(u8"连续 3 次认证失败，程序将退出。"));
        reject();
    }
}
```

这里可以看出，GUI 仍然调用 `AuthService::authenticate()`，不直接打开 `users.dat`。成功后用账号里的角色信息登录 `Session`；失败后只增加 GUI 自己的失败计数，并用弹窗显示错误。

空用户名退出的处理是：

```cpp
if (username.isEmpty()) {
    QMessageBox::information(
        this,
        QString::fromUtf8(u8"结束登录"),
        QString::fromUtf8(u8"用户名为空，本次启动将直接退出。"));
    reject();
    return;
}
```

这个行为和 CLI 的“登录界面输入空用户名退出程序”保持一致。

### 15.6 Qt 管理员窗口 `AdminWindow`

`AdminWindow` 是 GUI 里功能最多的窗口。它的声明在 `include/EduSys/gui/AdminWindow.hpp`，从函数名就能看出 6 个页签的边界：

```cpp
class AdminWindow : public QMainWindow {
public:
    AdminWindow(AppContext& appContext, Session session, QWidget* parent = nullptr);

private:
    QWidget* createStudentPage();
    QWidget* createCoursePage();
    QWidget* createScorePage();

    void refreshStudentTable();
    void showStudentById();
    void createStudent();
    void editSelectedStudent();
    void removeSelectedStudent();

    void refreshCourseTable();
    void showCourseById();
    void createCourse();
    void editSelectedCourse();
    void removeSelectedCourse();

    void refreshScoreTable();
    void showScoresByStudent();
    void showScoresByCourse();
    void createScore();
    void editSelectedScore();
    void removeSelectedScore();

    QWidget* createStatsPage();
    void queryCourseStats();
    void queryCourseRanking();
    void queryStudentGpa();

    QWidget* createReportPage();
    void exportWarningReport();
    void exportCourseStatsCsv();
    void exportRankingCsv();

    QWidget* createAccountPage();
};
```

构造函数里把 6 个页签挂到 `QTabWidget` 上：

```cpp
auto* tabs = new QTabWidget(this);
tabs->addTab(createStudentPage(), QString::fromUtf8(u8"学生管理"));
tabs->addTab(createCoursePage(), QString::fromUtf8(u8"课程管理"));
tabs->addTab(createScorePage(), QString::fromUtf8(u8"成绩管理"));
tabs->addTab(createStatsPage(), QString::fromUtf8(u8"统计分析"));
tabs->addTab(createReportPage(), QString::fromUtf8(u8"报告导出"));
tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));
```

#### 15.6.1 管理员学生管理页签

学生管理页签提供：刷新列表、按学号查看、新增学生、编辑选中学生、删除选中学生。界面层按钮连接如下：

```cpp
connect(refreshButton, &QPushButton::clicked, this, [this] { refreshStudentTable(); });
connect(viewButton, &QPushButton::clicked, this, [this] { showStudentById(); });
connect(createButton, &QPushButton::clicked, this, [this] { createStudent(); });
connect(editButton, &QPushButton::clicked, this, [this] { editSelectedStudent(); });
connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedStudent(); });
connect(studentLookupEdit_, &QLineEdit::returnPressed, this, [this] { showStudentById(); });
```

刷新列表调用 `StudentService::listAll()`：

```cpp
const auto students = appContext_.studentService.listAll(session_);
studentTable_->setRowCount(static_cast<int>(students.size()));
```

按学号查看调用 `StudentService::findById()`：

```cpp
const Student student = appContext_.studentService.findById(session_, id.toStdString());
showStudentDetails(student);
```

新增学生先打开 `StudentEditDialog`，对话框只收集字段，真正保存仍回到窗口调用服务：

```cpp
StudentEditDialog dialog(this);
if (dialog.exec() != QDialog::Accepted) {
    return;
}

const Student student = dialog.student();
appContext_.studentService.create(session_, student);
```

编辑学生会先读出当前学生，再把对象放进对话框：

```cpp
const Student current = appContext_.studentService.findById(session_, studentId.toStdString());
StudentEditDialog dialog(current, this);
if (dialog.exec() != QDialog::Accepted) {
    return;
}

Student updated = dialog.student();
appContext_.studentService.update(session_, updated);
```

删除学生前 GUI 会弹确认框，提示会级联删除成绩和学生账号：

```cpp
const auto confirm = QMessageBox::warning(
    this,
    QString::fromUtf8(u8"确认删除学生"),
    QString::fromUtf8(u8"删除学生 %1 后，将同时删除该学生关联的成绩记录和学生账户。\n此操作不可撤销，是否继续？")
        .arg(studentId),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);
if (confirm != QMessageBox::Yes) {
    return;
}

appContext_.studentService.remove(session_, studentId.toStdString());
```

这里的“级联删除”不是 GUI 自己完成的。GUI 只是提醒用户，实际清理发生在 `StudentService::remove()`：它会清理 `scores.dat` 中该学生的成绩、清理 `users.dat` 中 `ownerId` 指向该学生的学生账号，最后删除 `students.dat` 里的学生记录。

#### 15.6.2 管理员课程管理页签

课程管理页签提供：刷新列表、按课程号查看、新增课程、编辑选中课程、删除选中课程。按钮连接结构和学生页签相同：

```cpp
connect(refreshButton, &QPushButton::clicked, this, [this] { refreshCourseTable(); });
connect(viewButton, &QPushButton::clicked, this, [this] { showCourseById(); });
connect(createButton, &QPushButton::clicked, this, [this] { createCourse(); });
connect(editButton, &QPushButton::clicked, this, [this] { editSelectedCourse(); });
connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedCourse(); });
```

课程列表和详情分别调用：

```cpp
const auto courses = appContext_.courseService.listAll(session_);
const Course course = appContext_.courseService.findById(session_, id.toStdString());
```

新增和编辑使用 `CourseEditDialog`：

```cpp
CourseEditDialog dialog(this);
if (dialog.exec() != QDialog::Accepted) {
    return;
}

const Course course = dialog.course();
appContext_.courseService.create(session_, course);
```

```cpp
const Course current = appContext_.courseService.findById(session_, courseId.toStdString());
CourseEditDialog dialog(current, this);
if (dialog.exec() != QDialog::Accepted) {
    return;
}

Course updated = dialog.course();
appContext_.courseService.update(session_, updated);
```

删除课程同样先弹确认框，再调用服务：

```cpp
const auto confirm = QMessageBox::warning(
    this,
    QString::fromUtf8(u8"确认删除课程"),
    QString::fromUtf8(u8"删除课程 %1 后，将同时删除该课程关联的所有成绩记录。\n此操作不可撤销，是否继续？")
        .arg(courseId),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);
if (confirm != QMessageBox::Yes) {
    return;
}

appContext_.courseService.remove(session_, courseId.toStdString());
```

课程删除的真实级联清理发生在 `CourseService::remove()`：它先从 `scores.dat` 删除该课程所有成绩，再从 `courses.dat` 删除课程记录。GUI 没有自己读写 `scores.dat`。

#### 15.6.3 管理员成绩管理页签

成绩管理页签提供：查看全部、按学生查、按课程查、录入成绩、编辑选中成绩、删除选中成绩。按钮连接如下：

```cpp
connect(refreshButton, &QPushButton::clicked, this, [this] { refreshScoreTable(); });
connect(byStudentButton, &QPushButton::clicked, this, [this] { showScoresByStudent(); });
connect(byCourseButton, &QPushButton::clicked, this, [this] { showScoresByCourse(); });
connect(createButton, &QPushButton::clicked, this, [this] { createScore(); });
connect(editButton, &QPushButton::clicked, this, [this] { editSelectedScore(); });
connect(removeButton, &QPushButton::clicked, this, [this] { removeSelectedScore(); });
```

三个查询入口分别调用：

```cpp
const auto scores = appContext_.scoreService.listAll(session_);
const auto scores = appContext_.scoreService.findByStudent(session_, studentId.toStdString());
const auto scores = appContext_.scoreService.findByCourse(session_, courseId.toStdString());
```

录入成绩使用 `ScoreEditDialog`：

```cpp
ScoreEditDialog dialog(this);
if (dialog.exec() != QDialog::Accepted) {
    return;
}

const Score score = dialog.score();
appContext_.scoreService.upsert(session_, score);
```

编辑成绩时，GUI 从表格选中行拿到 `studentId + courseId + semester` 三元组，再找出当前分数对象放进对话框：

```cpp
ScoreEditDialog dialog(current, this);
if (dialog.exec() != QDialog::Accepted) {
    return;
}

const Score updated = dialog.score();
appContext_.scoreService.upsert(session_, updated);
```

删除单条成绩调用：

```cpp
appContext_.scoreService.remove(
    session_,
    studentId.toStdString(),
    courseId.toStdString(),
    semester.toStdString());
```

成绩是否允许写入、学生是否存在、课程是否存在、分数是否在 `0..100`、同一个 `(studentId, courseId, semester)` 是否应该覆盖，全部由 `ScoreService` 决定。GUI 表单会做一些基础非空和数值范围限制，但它不是最终规则来源。

#### 15.6.4 管理员统计分析页签

统计分析页签提供三件事：课程统计、课程排名、学生 GPA。对应服务调用是：

```cpp
const CourseStats stats = appContext_.statsService.computeCourseStats(session_, courseId.toStdString());
const auto ranking = appContext_.statsService.rankByCourse(session_, courseId.toStdString());
const GpaResult gpa = appContext_.statsService.computeGpaFor(session_, studentId.toStdString());
```

课程统计结果用 `QMessageBox` 展示，课程排名放在 `rankingTable_` 表格里，学生 GPA 也用弹窗展示。统计页签不写 `.dat` 文件，只读取学生、课程、成绩数据。统计口径仍在 `StatsService.cpp`：平均分、最高分、最低分、及格率、优秀率、GPA 分段都在那里计算。

#### 15.6.5 管理员报告导出页签

报告导出页签提供三类文件输出：

```cpp
const std::string path = appContext_.reportExporter.exportWarningReport(session_);
const std::string path = appContext_.reportExporter.exportCourseStatsCsv(session_, courseId.toStdString());
const std::string path = appContext_.reportExporter.exportRankingCsv(session_, courseId.toStdString());
```

导出路径与 CLI 一致：

```text
data/warning_report.txt
data/course_stats_<courseId>.csv
data/ranking_<courseId>.csv
```

GUI 只是把成功路径用 `QMessageBox` 告诉用户。文件内容如何组织、CSV 如何转义、课程编号是否安全，仍然由 `ReportExporter.cpp` 负责。

#### 15.6.6 管理员账户页签

账户页签提供修改密码和退出登录。修改密码使用 `ChangePasswordDialog`：

```cpp
connect(changePwButton, &QPushButton::clicked, this, [this] {
    ChangePasswordDialog dialog(appContext_, session_, this);
    dialog.exec();
});
```

退出登录不是调用某个 AuthService logout，因为 `Session` 是窗口对象持有的值对象，关闭窗口即可结束当前会话：

```cpp
connect(logoutButton, &QPushButton::clicked, this, [this] {
    const auto confirm = QMessageBox::question(
        this,
        QString::fromUtf8(u8"确认退出"),
        QString::fromUtf8(u8"确定要退出登录吗？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (confirm == QMessageBox::Yes) {
        close();
    }
});
```

窗口关闭后，`gui_main.cpp` 的事件循环返回，程序重新打开登录框。

### 15.7 Qt 教师窗口 `TeacherWindow`

`TeacherWindow` 有 4 个页签：我的课程、我的课程成绩、我的课程统计、账户。构造函数如下：

```cpp
auto* tabs = new QTabWidget(this);
tabs->addTab(createMyCoursesPage(), QString::fromUtf8(u8"我的课程"));
tabs->addTab(createMyScoresPage(), QString::fromUtf8(u8"我的课程成绩"));
tabs->addTab(createMyStatsPage(), QString::fromUtf8(u8"我的课程统计"));
tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));
```

教师端最重要的点是白名单，也就是只能选择和操作自己的课程。GUI 只让教师从“我的课程”里选择课程，服务层也会继续拒绝越权课程。也就是说，教师侧有两道防线。

#### 15.7.1 教师“我的课程”

教师课程列表来自：

```cpp
std::vector<Course> TeacherWindow::myCourses() {
    return appContext_.courseService.listAll(session_);
}
```

而 `CourseService::listAll()` 对教师会过滤课程：

```cpp
if (session.isTeacher()) {
    all.erase(std::remove_if(all.begin(), all.end(),
        [&](const Course& c) { return c.getTeacherId() != session.getOwnerId(); }),
        all.end());
}
```

这说明 GUI 表格看到的课程已经是本人课程。教师看不到别的老师课程，不是因为前端临时隐藏，而是服务层返回结果本身已经过滤。

#### 15.7.2 教师“我的课程成绩”

教师先从下拉框选择课程，然后才能加载或录入成绩。加载成绩调用：

```cpp
const auto scores = appContext_.scoreService.findByCourse(session_, courseId.toStdString());
```

录入成绩时，GUI 会把课程号锁定：

```cpp
ScoreEditDialog dialog(this);
dialog.setCourseIdLocked(courseId.toStdString());
if (dialog.exec() != QDialog::Accepted) {
    return;
}

const Score score = dialog.score();
appContext_.scoreService.upsert(session_, score);
```

`ScoreEditDialog::setCourseIdLocked()` 的实现是：

```cpp
void ScoreEditDialog::setCourseIdLocked(const std::string& courseId) {
    courseIdEdit_->setText(QString::fromStdString(courseId));
    courseIdEdit_->setEnabled(false);
}
```

这解决了一个很实际的问题：教师在 GUI 里不能手动把课程号改成别人的课程号。即使有人绕过 GUI 构造请求，`ScoreService::upsert()` 仍会检查课程教师：

```cpp
if (session.isTeacher() && course.getTeacherId() != session.getOwnerId()) {
    throw PermissionException("Teacher can only write scores of own courses");
}
```

删除成绩也有服务层保护：

```cpp
if (session.isTeacher()) {
    Course course = requireCourse(courseRepo_, courseId);
    if (course.getTeacherId() != session.getOwnerId()) {
        throw PermissionException("Teacher can only delete scores of own courses");
    }
}
```

所以教师侧不是“相信下拉框一定不会出错”，而是 GUI 先限制，Service 再拒绝。

#### 15.7.3 教师“我的课程统计”

教师统计页签调用：

```cpp
const CourseStats stats = appContext_.statsService.computeCourseStats(session_, courseId.toStdString());
const auto ranking = appContext_.statsService.rankByCourse(session_, courseId.toStdString());
```

`StatsService` 对教师也会检查课程归属：

```cpp
if (session.isTeacher() && courseIt->getTeacherId() != session.getOwnerId()) {
    throw PermissionException("Teacher can only query own course stats");
}
```

排名接口也有类似检查：

```cpp
if (session.isTeacher() && courseIt->getTeacherId() != session.getOwnerId()) {
    throw PermissionException("Teacher can only rank own course");
}
```

因此教师端统计的权限边界和 CLI 保持一致。

#### 15.7.4 教师账户页签

教师账户页签和管理员、学生账户页签共用 `ChangePasswordDialog`。退出登录同样是关闭当前 `TeacherWindow`，再由 `gui_main.cpp` 回到登录框。

### 15.8 Qt 学生窗口 `StudentWindow`

`StudentWindow` 有 4 个页签：我的资料、我的成绩、我的 GPA、账户。构造函数如下：

```cpp
auto* tabs = new QTabWidget(this);
tabs->addTab(createProfilePage(), QString::fromUtf8(u8"我的资料"));
tabs->addTab(createMyScoresPage(), QString::fromUtf8(u8"我的成绩"));
tabs->addTab(createMyGpaPage(), QString::fromUtf8(u8"我的 GPA"));
tabs->addTab(createAccountPage(), QString::fromUtf8(u8"账户"));
```

学生端全部业务都是“看自己”和“改自己密码”，没有写成绩、改课程、改学生档案的入口。

#### 15.8.1 学生“我的资料”

学生资料页签直接使用当前会话的 `ownerId`：

```cpp
const Student me = appContext_.studentService.findById(session_, session_.getOwnerId());
```

这里没有输入框让学生填写别人学号，所以 GUI 层已经避免了常见误操作。即使有人从别处调用服务，`StudentService` 仍会做权限判断。

#### 15.8.2 学生“我的成绩”

学生成绩页签调用：

```cpp
const auto scores = appContext_.scoreService.findByStudent(session_, session_.getOwnerId());
```

`ScoreService::findByStudent()` 对学生角色也有保护：

```cpp
if (session.isStudent() && studentId != session.getOwnerId()) {
    throw PermissionException("Student can only read own scores");
}
```

所以学生端看到的成绩来自服务层过滤，不是 GUI 自己从全部成绩里随便筛一下。

#### 15.8.3 学生“我的 GPA”

学生 GPA 页签调用：

```cpp
const GpaResult gpa = appContext_.statsService.computeGpaFor(session_, session_.getOwnerId());
```

学生只能查自己的 GPA，这个限制由服务层再次检查：

```cpp
if (session.isStudent() && studentId != session.getOwnerId()) {
    throw PermissionException("Student can only query own GPA");
}
```

GPA 的分段、学分加权和课程数量统计都在 `StatsService.cpp`。GUI 只负责把 `GpaResult` 显示成文字。

#### 15.8.4 学生账户页签

学生账户页签仍然使用：

```cpp
ChangePasswordDialog dialog(appContext_, session_, this);
dialog.exec();
```

密码修改最终调用 `AuthService::changePassword()`，所以旧密码是否正确、新密码是否为空、账号是否存在，仍由认证服务保证。

### 15.9 Qt 表单对话框

Qt GUI 没有在主窗口里直接堆所有输入框，而是把新增/编辑动作放进独立对话框。这样每个对话框只负责一种对象，代码更容易读。

`StudentEditDialog` 用于学生新增和编辑。编辑已有学生时会禁用学号，因为学号是学生主键，不应该在编辑时随意改：

```cpp
void StudentEditDialog::loadStudent(const Student& student) {
    idEdit_->setText(QString::fromStdString(student.getId()));
    idEdit_->setEnabled(false);
    nameEdit_->setText(QString::fromStdString(student.getName()));
    majorEdit_->setText(QString::fromStdString(student.getMajor()));
    classEdit_->setText(QString::fromStdString(student.getClassName()));
    yearSpin_->setValue(student.getEnrollYear());
    contactEdit_->setText(QString::fromStdString(student.getContact()));
}
```

表单层做最基础的非空检查：

```cpp
if (idEdit_->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"学号不能为空。"));
    idEdit_->setFocus();
    return;
}
if (nameEdit_->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, QString::fromUtf8(u8"表单不完整"), QString::fromUtf8(u8"姓名不能为空。"));
    nameEdit_->setFocus();
    return;
}
```

学生入学年份在 GUI 里使用 `QSpinBox`，当前窗口限制为 `1900..2100`：

```cpp
yearSpin_ = new QSpinBox(this);
yearSpin_->setRange(1900, 2100);
yearSpin_->setValue(2025);
```

服务层的最终规则更抽象，只要求入学年份大于 0。也就是说，GUI 表单为了正常使用体验做了更窄的输入范围，Service 仍然是业务规则的最终入口。

`CourseEditDialog` 的结构类似，编辑已有课程时禁用课程号，并检查课程号、课程名、授课教师编号：

```cpp
void CourseEditDialog::loadCourse(const Course& course) {
    idEdit_->setText(QString::fromStdString(course.getCourseId()));
    idEdit_->setEnabled(false);
    nameEdit_->setText(QString::fromStdString(course.getCourseName()));
    creditSpin_->setValue(course.getCredit());
    teacherIdEdit_->setText(QString::fromStdString(course.getTeacherId()));
    semesterEdit_->setText(QString::fromStdString(course.getSemester()));
}
```

课程学分在 GUI 里使用 `QDoubleSpinBox`，当前窗口限制为 `0.5..20.0`：

```cpp
creditSpin_ = new QDoubleSpinBox(this);
creditSpin_->setRange(0.5, 20.0);
creditSpin_->setSingleStep(0.5);
creditSpin_->setDecimals(1);
creditSpin_->setValue(2.0);
```

服务层的最终规则是课程学分必须大于 0。这个差别不影响常规课程管理，但说明“GUI 与 CLI 功能等价”不是说每一个输入控件的边界都和 CLI 完全一样；它强调的是业务链路、权限规则和数据落点一致。

`ScoreEditDialog` 用于成绩新增和编辑。分数输入框用 `QDoubleSpinBox` 限制范围：

```cpp
for (QDoubleSpinBox* spin : {usualSpin_, finalSpin_, totalSpin_}) {
    spin->setRange(0.0, 100.0);
    spin->setDecimals(1);
    spin->setSingleStep(0.5);
}
```

编辑已有成绩时，三元主键字段会禁用。这里的三元主键字段指“学生号 + 课程号 + 学期”：

```cpp
studentIdEdit_->setEnabled(false);
courseIdEdit_->setEnabled(false);
semesterEdit_->setEnabled(false);
```

`ChangePasswordDialog` 是三类角色共用的改密码对话框。它先做旧密码、新密码、确认密码的表单校验，再调用认证服务：

```cpp
appContext_.authService.changePassword(session_, oldPw.toStdString(), newPw.toStdString());
```

这些对话框的定位要说清楚：它们只负责“把用户输入变成对象”或“把用户输入传给服务”。学生、课程、成绩编辑对话框本身不直接读写 `.dat`；`ChangePasswordDialog` 会在对话框内部调用 `AuthService::changePassword()`，但也仍然不直接改 `.dat` 文件。复杂权限和最终业务规则仍由 Service 层处理。

### 15.10 Qt GUI 和 CLI 的功能等价边界

Qt GUI 的业务功能目标是与 CLI 等价，但“等价”不是指所有入口都做成窗口按钮。这里分两类：

| 类别 | CLI | Qt GUI |
| --- | --- | --- |
| 登录 | 用户名/密码输入、空用户名退出、三次失败退出 | `LoginDialog` 等价实现 |
| 管理员业务 | 学生、课程、成绩、统计、报告、CSV、改密码 | `AdminWindow` 6 个页签等价实现 |
| 教师业务 | 我的课程、我的课程成绩、我的课程统计、改密码 | `TeacherWindow` 4 个页签等价实现 |
| 学生业务 | 我的资料、我的成绩、我的 GPA、改密码 | `StudentWindow` 4 个页签等价实现 |
| 数据持久化 | 按当前运行目录读写 `data/*.dat` | 在相同运行目录下读写同一批 `data/*.dat` |
| 导出文件 | `warning_report.txt`、CSV | 路径和语义一致 |
| 自动回归 | `edusys.exe --self-test` | 不做成 GUI 菜单项 |
| 文件损坏恢复验证 | `tools/corrupt_check.bat` | 不做成 GUI 菜单项 |
| CLI 手工演示 | `docs/cli-demo-guide.md`、`demo_input.txt` | 不属于 GUI 验收 |

补充一点：GUI 为了减少误输入，会在个别表单控件上设置更窄的常用范围，例如入学年份和课程学分；Service 层仍然是最终业务规则入口。所以这里的“等价”重点是业务功能、权限链路、统计/导出口径和数据落点一致，不是说每一个输入控件的可输入范围都和 CLI 完全相同。

所以在答辩或演示时可以这样解释：

- 如果老师问“GUI 有没有完整业务功能”，可以看 `AdminWindow`、`TeacherWindow`、`StudentWindow` 的页签和服务调用。
- 如果老师问“自动测试怎么保证”，仍然看 `edusys.exe --self-test` 和 `tools/corrupt_check.bat`。
- 如果老师问“GUI 改数据后 CLI 能不能读”，答案是：在相同运行目录下启动时可以，因为两边都通过 `AppContext` 连接同一批仓储和 `data/*.dat` 文件。
- 如果老师问“为什么不把 `--self-test` 做成 GUI 按钮”，答案是它本来就是自动回归入口，不是业务用户功能；保留在 CLI 更适合脚本化和课堂验证。

## 16. 开发阶段回顾

| 周次 | 完成内容 |
| --- | --- |
| Week 9 | 项目骨架、命名空间、核心实体模型、基本目录树 |
| Week 10 | 二进制读写器、模板仓储、`.dat` 文件格式、持久化设施 |
| Week 11 | 认证、会话、三大领域服务、级联删除、端到端自检 |
| Week 12 | 视图层菜单、统计分析、学业预警、报告导出 |
| Week 13 | 集成测试、边界测试、F 组文件损坏脚本 |
| Week 14 | 架构图册、答辩稿、CSV 导出、Qt GUI 接入与验收、README 总整理 |

## 17. 推荐阅读路径

如果你是不同角色，可以按下面方式看仓库：

| 你的身份 | 推荐先看什么 |
| --- | --- |
| 第一次接手这个项目的同学 | 本 README 的第 4-12 节 |
| 想直接改功能的开发者 | `include/` 对应接口头文件，再看 `src/` 实现 |
| 想准备答辩的人 | `docs/architecture.md` + `docs/defense.md` + 本 README 第 12-15 节 |
| 想验证项目可靠性的人 | `docs/test-cases.md` + `tools/corrupt_check.bat` |
| 想继续完善 Qt 分支的人 | `docs/introduceQt-acceptance.md` + `src/app/gui_main.cpp` + `src/gui/` 目录 + `docs/architecture.md` 末尾的 Qt 段 |

## 18. 一句话总结这棵目录树

如果只用一句话概括这整个仓库：

> `include/` 负责说明接口和边界，`src/` 负责实现业务流程，`docs/` 负责说明设计与答辩材料，`data/` 负责承载运行状态，`tools/` 负责验证文件损坏场景，`build/` 是自动生成的构建产物目录。

按这个划分阅读目录树，可以判断每类文件的作用和阅读顺序。
