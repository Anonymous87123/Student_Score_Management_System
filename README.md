# EduSys - 学生成绩管理系统

EduSys 是一个基于 `C++17` 的控制台学生成绩管理系统，目标不是做一个“概念很大、实现很空”的样板，而是完成一个真正可编译、可运行、可保存、可回归测试、可用于课程答辩展示的小型软件系统。

当前仓库已经完成首版交付闭环：

- 三类角色登录与权限控制：`Admin`、`Teacher`、`Student`
- 学生、课程、成绩三大核心对象的 CRUD 与查询
- GPA、课程统计、课程排名、学业预警
- `.dat` 二进制持久化与显式序列化
- `warning_report.txt` 文本报告与课程 CSV 导出
- `--self-test` 自检入口与 `tools/corrupt_check.bat` 文件损坏脚本
- 架构图册、答辩问答稿、测试用例文档三套文档

这份 `README.md` 的定位不再只是“运行说明”，而是**仓库导航手册**。下面会按目录树逐级展开，把主要文件的具体内容、职责边界、使用时机都讲清楚。

## 1. 快速开始

### 1.1 最短路径

```bash
build.bat
edusys.exe --self-test
edusys.exe
tools\corrupt_check.bat
```

四条命令分别对应：

- `build.bat`：用 `g++` 在仓库根目录构建 `edusys.exe`
- `edusys.exe --self-test`：执行 Week 11 + Week 13 A-E 组内建自检
- `edusys.exe`：进入正常登录与菜单交互
- `tools\corrupt_check.bat`：执行 Week 13 F 组文件损坏测试并自动恢复

### 1.2 两种构建方式

#### 方式 A：CMake / MSVC

```bash
cmake -S . -B build
cmake --build build --config Release
```

说明：

- 这条路径适合 Visual Studio / CMake 用户
- 产物通常位于 `build/Release/edusys.exe` 或 CMake 指定输出目录
- `build/` 目录里的内容大多数是自动生成文件，不是手写业务代码

#### 方式 B：`g++` 兜底脚本

```bash
build.bat
```

说明：

- 这条路径适合课程环境中只装了 MinGW / g++ 的情况
- `build.bat` 会把所有 `src/*.cpp` 一次性编译到根目录的 `edusys.exe`
- `tools/corrupt_check.bat` 默认就假定你运行的是根目录下这个 `edusys.exe`

## 2. 系统功能概览

### 2.1 三类角色

| 角色 | 能做什么 | 不能做什么 |
| --- | --- | --- |
| `Admin` | 学生/课程/成绩的完整 CRUD、统计、预警报告、CSV 导出、修改自己的密码 | 无业务级限制 |
| `Teacher` | 只查看并维护自己授课课程的成绩、查看自己课程的统计、修改自己的密码 | 不能改学生与课程基础数据，不能动别人的课程 |
| `Student` | 只查看自己的资料、成绩、GPA，修改自己的密码 | 不能写成绩，不能看别人数据 |

### 2.2 菜单总览

```text
Login loop
  -> username / password
  -> AuthService.authenticate()

Admin Menu
  1. Student management
  2. Course management
  3. Score management
  4. Statistics
  5. Generate warning report
  6. Change my password
  7. Export course CSV
  0. Logout

Teacher Menu
  1. List my courses
  2. View scores for my course
  3. Record / update one score
  4. Delete one score
  5. Course statistics
  6. Change my password
  0. Logout

Student Menu
  1. View my profile
  2. View my scores
  3. View my GPA
  4. Change my password
  0. Logout
```

### 2.3 默认样例账号

| 用户名 | 密码 | 角色 | 说明 |
| --- | --- | --- | --- |
| `admin` | `admin123` | `Admin` | 管理员账号，不关联任何 `Person` 实体 |
| `t001` | `t001pw` | `Teacher` | 关联教师 `T001` |
| `s001` | `s001pw` | `Student` | 关联学生 `S001` |

补充说明：

- 仓库第一次运行且五个 `.dat` 文件都为空时，会自动 seed 初始数据
- Week 11 自检会在首次 `SEEDED` 运行里做一次破坏性演化：新增 `S003`、删除 `S002`、更新 `S001` 的分数
- 因此“当前数据状态”和“刚 seed 完的初始状态”可能不同，这是设计的一部分，不是数据漂移

## 3. 文档应该怎么读

如果你不是来改代码，而是想最快理解项目，请按下面顺序看：

| 顺序 | 文件 | 适合什么时候看 |
| --- | --- | --- |
| 1 | [`docs/architecture.md`](docs/architecture.md) | 先建立“这项目长什么样”的全景图 |
| 2 | [`docs/defense.md`](docs/defense.md) | 准备答辩，或者想知道“为什么这样设计” |
| 3 | [`docs/test-cases.md`](docs/test-cases.md) | 想看可验证性、边界测试与损坏恢复 |
| 4 | [`claude.md`](claude.md) | 想看最初设计意图、范围控制和架构红线 |
| 5 | [`README.md`](README.md) | 想把“怎么运行 + 目录树 + 文件功能”一次看全 |

## 4. 仓库目录树总览

下面这棵树分成两类：

- **人工维护文件**：源码、文档、脚本、说明文件
- **运行期/构建期产物**：`.dat`、日志、CSV、`build/` 生成物

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
│  ├─ course_stats_C001.csv
│  ├─ ranking_C001.csv
│  └─ __corrupt_out__/
│     ├─ F1.out
│     ├─ F2.out
│     ├─ F3.out
│     └─ sanity.out
├─ docs/
│  ├─ .gitkeep
│  ├─ architecture.md
│  ├─ defense.md
│  └─ test-cases.md
├─ include/
│  └─ EduSys/
│     ├─ app/
│     │  └─ .gitkeep
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
│     └─ view/
│        ├─ .gitkeep
│        ├─ BaseMenu.hpp
│        ├─ AdminMenu.hpp
│        ├─ TeacherMenu.hpp
│        └─ StudentMenu.hpp
├─ src/
│  ├─ app/
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

下面开始逐级展开。

## 5. 根目录逐文件说明

| 文件 / 目录 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`.gitignore`](.gitignore) | 当前主要忽略根目录 `edusys.exe` 与 `build/Release/edusys.exe` 这类“容易造成我到底在跑哪个二进制”的产物。 | 防止把本地编译结果误提交进仓库，避免测试脚本和真实源码状态错位。 |
| [`CMakeLists.txt`](CMakeLists.txt) | 定义项目名 `EduSys`、C++17 标准、MSVC / 非 MSVC 编译选项，以及所有 `.cpp` 源文件列表。 | 面向 CMake/Visual Studio 的正式构建入口。 |
| [`build.bat`](build.bat) | 一个简洁的 `g++` 构建脚本，把所有 `src/*.cpp` 直接编成根目录 `edusys.exe`。 | 课程环境兜底构建方案；适合“没有完整 CMake 工具链，但能用 g++”的情况。 |
| [`README.md`](README.md) | 也就是你正在看的这份文档。 | 仓库导航、运行说明、目录树总解说。 |
| [`claude.md`](claude.md) | 开题报告修订版，包含任务分析、目标范围、架构图、风险评估、Qt 预留思路、开发红线。 | 这是项目“为什么这样设计”的源头文档。 |
| [`demo_input.txt`](demo_input.txt) | 一份标准输入脚本，串起 Admin、Teacher、Student 三段演示路径。 | 用于一键复现完整交互流程，适合录屏、答辩彩排、回归展示。 |
| [`data/`](data) | 所有运行期数据、日志、报表、CSV 与损坏测试输出都放在这里。 | 这是程序的“工作目录”；删掉其中的 `.dat` 会影响状态。 |
| [`docs/`](docs) | 架构图册、答辩稿、测试汇总文档。 | 面向老师、答辩和维护者。 |
| [`include/`](include) | 公开头文件树。 | 体现类型定义、服务接口、仓储接口与菜单类边界。 |
| [`src/`](src) | 具体实现。 | 所有业务行为都最终在这里落地。 |
| [`tools/`](tools) | 工程辅助脚本。 | 当前最关键的是 F 组文件损坏脚本。 |
| [`build/`](build) | CMake / MSVC 自动生成目录。 | 不是手写业务代码目录，可随时删除并重新生成。 |

## 6. `docs/` 目录逐文件说明

`docs/` 负责“把代码讲清楚”，不是“重新写一份代码”。

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`docs/.gitkeep`](docs/.gitkeep) | 占位文件。 | 最早用于保证空 `docs/` 目录也能被 Git 跟踪；即使后来目录不空，它保留也无害。 |
| [`docs/architecture.md`](docs/architecture.md) | 复用了 `claude.md` 里的 5 张 Mermaid 图：思维导图、分层图、类关系图、级联删除图、登录与权限流程图；每张图旁边都加了“当前仓库哪份文件对应哪条结构”的对证说明。末尾还有“Qt 适配讨论”。 | 当你要回答“项目整体长什么样”“换 Qt 会改哪一层”时，先看它。 |
| [`docs/defense.md`](docs/defense.md) | 一份答辩问答稿，共 14 条核心 Q&A，围绕四层分层、显式序列化、权限矩阵、测试入口、Qt 复用边界等高频问题组织。每条都附当前文件与行号。 | 当你要准备口头表达，或者不想临场组织语言时，看它。 |
| [`docs/test-cases.md`](docs/test-cases.md) | Week 13 的测试总表，按 A-F 六组组织：认证与会话、字段校验、唯一性、级联删除残留、教师白名单、文件损坏。文末附日志证据与设计取舍备注。 | 当你要证明“项目不是只会跑 happy path”时，看它。 |

## 7. `include/EduSys/` 目录逐级说明

这一层代表**接口面**。它告诉你系统有哪些概念、有哪些服务、有哪些菜单，但不负责把所有细节写出来。

### 7.1 `include/EduSys/app/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/app/.gitkeep`](include/EduSys/app/.gitkeep) | 预留占位文件。 | 最初设计里曾打算单独引入 `Application.hpp` 一类的应用装配入口；目前真正的入口逻辑集中在 `src/app/main.cpp`。这个目录相当于保留了未来重构空间。 |

### 7.2 `include/EduSys/common/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/common/Constants.hpp`](include/EduSys/common/Constants.hpp) | 集中定义 `USERS_FILE`、`STUDENTS_FILE`、`DATA_DIR` 等路径常量，以及 `SCORE_MIN`、`SCORE_MAX`、`SCORE_PASS`、`USUAL_WEIGHT` 等业务常量。 | 防止路径、阈值、权重散落在实现中形成魔法数字。 |
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

### 7.6 `include/EduSys/view/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/view/.gitkeep`](include/EduSys/view/.gitkeep) | 占位文件。 | 保留目录结构。 |
| [`include/EduSys/view/BaseMenu.hpp`](include/EduSys/view/BaseMenu.hpp) | 抽象菜单基类，声明 `readLine/readInt/readDouble/printTable/paginate/printError/printOk`。 | 明确规定：`std::cin/std::cout` 只允许集中出现在 View 层工具里。 |
| [`include/EduSys/view/AdminMenu.hpp`](include/EduSys/view/AdminMenu.hpp) | 管理员菜单类，持有 `Session` 与各类服务引用。 | 把 Admin 可见的所有操作编排成菜单流程。 |
| [`include/EduSys/view/TeacherMenu.hpp`](include/EduSys/view/TeacherMenu.hpp) | 教师菜单类，额外声明 `pickOwnCourseId()` 白名单选择器。 | 在 View 层先做一层“我的课程”过滤，再交给 Service 硬校验。 |
| [`include/EduSys/view/StudentMenu.hpp`](include/EduSys/view/StudentMenu.hpp) | 学生菜单类。 | 负责“只读自己”的交互展示。 |

### 7.7 `include/EduSys/report/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`include/EduSys/report/ReportExporter.hpp`](include/EduSys/report/ReportExporter.hpp) | 暴露 `exportWarningReport`、`exportCourseStatsCsv`、`exportRankingCsv` 三个导出接口。 | 这是一个独立于 `service/`、`view/`、`storage/` 的轻量适配器，只负责把统计结果变成文本或 CSV 文件。 |

## 8. `src/` 目录逐级说明

`src/` 是真正发生行为的地方。和 `include/` 不同，它不是只告诉你“有什么接口”，而是把逻辑一步一步跑起来。

### 8.1 `src/app/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/app/main.cpp`](src/app/main.cpp) | 这是项目最核心的装配文件。它做了五件大事：`ensureDirectoryExists()` 保证 `data/` 存在；`seedSampleData()` 在空仓库时注入样例数据；`runWeek11SelfCheck()` 执行端到端自检；`runWeek13BoundaryCheck()` 执行 A-E 只读边界验证；`runInteractiveLoop()` 进入正常登录循环；最后由 `main()` 根据参数决定走交互模式还是自检模式。 | 入口、依赖组装、模式切换、自检主控都在这里。后续如果接 Qt，这个文件会是最先被重构的地方之一。 |

### 8.2 `src/common/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/common/Logger.cpp`](src/common/Logger.cpp) | 实现 `Logger` 单例，生成时间戳，按 `INFO/WARN/ERROR` 级别同步追加写入 `data/app.log`。 | 让所有关键事件都有落盘证据，尤其适合自检与损坏恢复场景。 |
| [`src/common/PasswordHasher.cpp`](src/common/PasswordHasher.cpp) | 用固定盐 + `FNV-1a 64-bit` 生成 16 字符十六进制摘要，并实现密码比对。 | 这是课程环境下“避免明文落盘”的轻量方案，不追求生产级密码学安全。 |

### 8.3 `src/model/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/model/Student.cpp`](src/model/Student.cpp) | 实现学生构造、字段 setter、`roleLabel()`，以及学生对象的显式 `writeTo/readFrom`。 | 让 `Student` 真正可持久化。 |
| [`src/model/Teacher.cpp`](src/model/Teacher.cpp) | 实现教师构造、字段 setter、`roleLabel()`，以及显式序列化。 | 与 `Student` 形成对称实现。 |
| [`src/model/UserAccount.cpp`](src/model/UserAccount.cpp) | 实现账号对象构造、密码哈希更新、启停状态、`ownerId` 写入与反序列化。 | 登录账号的实际落盘格式在这里定义。 |
| [`src/model/Course.cpp`](src/model/Course.cpp) | 实现课程构造、字段修改与显式序列化。 | 课程记录的 `.dat` 布局在这里确定。 |
| [`src/model/Score.cpp`](src/model/Score.cpp) | 实现成绩构造与显式序列化。 | 定义 `(studentId, courseId, semester, usual, final, total)` 如何存盘。 |

### 8.4 `src/service/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/service/.gitkeep`](src/service/.gitkeep) | 占位文件。 | 历史骨架保留。 |
| [`src/service/AuthService.cpp`](src/service/AuthService.cpp) | 实现账号查找、启用状态检查、密码验证、认证失败日志、修改密码逻辑。 | 所有“身份可信度”都经由此层统一判断。 |
| [`src/service/StudentService.cpp`](src/service/StudentService.cpp) | 实现学生字段校验、权限检查、创建/修改/查询；删除时按顺序清成绩、清学生账号、再删学生本体。 | “删学生是否会留下幽灵账号和幽灵成绩”这个问题的答案完全在这里。 |
| [`src/service/CourseService.cpp`](src/service/CourseService.cpp) | 实现课程字段校验、教师存在性校验、创建/修改/查询；删除课程时级联清成绩。 | 管理课程数据的一致性。 |
| [`src/service/ScoreService.cpp`](src/service/ScoreService.cpp) | 实现成绩范围校验、学生/课程存在性校验、角色权限矩阵、教师仅能操作自己课程、同键 `upsert`、删除单条成绩。 | 权限和业务规则最密集的文件之一。 |
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
| [`src/view/BaseMenu.cpp`](src/view/BaseMenu.cpp) | 实现统一输入、数值解析、分页打印、表格输出、标题分隔线、成功/失败消息。 | 所有菜单都共享同一套 CLI 交互工具，避免每个菜单自己乱写 I/O。 |
| [`src/view/AdminMenu.cpp`](src/view/AdminMenu.cpp) | 实现管理员主菜单及其子菜单：学生管理、课程管理、成绩管理、统计、预警报告、CSV 导出、改密码。 | Admin 的全部交互编排都在这里。 |
| [`src/view/TeacherMenu.cpp`](src/view/TeacherMenu.cpp) | 实现教师菜单，并通过 `pickOwnCourseId()` 先过滤“我的课”，再允许录分、删分、查统计。 | View 层白名单防线所在文件。 |
| [`src/view/StudentMenu.cpp`](src/view/StudentMenu.cpp) | 实现学生个人资料、个人成绩、个人 GPA、修改密码四类只读/自助操作。 | 学生端交互入口。 |

### 8.7 `src/report/`

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`src/report/ReportExporter.cpp`](src/report/ReportExporter.cpp) | 实现三类导出：`warning_report.txt`、课程统计 CSV、课程排名 CSV。这里包含当前时间格式化、CSV 转义、课程编号安全性检查、输出文件落盘与日志记录。 | 把 `StatsService` 的结构化结果转换成最终文件，是项目里最典型的“适配器”实现。 |

## 9. `tools/` 目录逐文件说明

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`tools/corrupt_check.bat`](tools/corrupt_check.bat) | 这是 Week 13 F 组脚本。它会先检查根目录 `edusys.exe` 是否存在且是否可能过旧，再验证 `data/*.dat` 是否齐全；之后做 `.dat` 快照备份，依次制造 3 类文件损坏场景：删除 `scores.dat`、把 `users.dat` 头的 `EDSY` 改成 `XXXX`、把 `scores.dat` 计数字段伪造为 `255`；每次都运行 `edusys.exe --self-test` 捕获输出并恢复现场；最后再跑一次 sanity 自检。 | 它证明“坏数据不是默默带病运行，而是能被尽早识别并恢复”。 |

## 10. `data/` 目录逐文件说明

`data/` 不是源码目录，但它是程序运行时最重要的状态目录。

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
| [`data/course_stats_C001.csv`](data/course_stats_C001.csv) | 某门课程的统计 CSV，目前样例是 `C001`。 | Admin 菜单第 7 项产物之一。 |
| [`data/ranking_C001.csv`](data/ranking_C001.csv) | 某门课程的排名 CSV，目前样例是 `C001`。 | Admin 菜单第 7 项产物之一。 |

### 10.3 `__corrupt_out__/` 文件损坏测试输出

| 文件 | 具体内容 | 功能与定位 |
| --- | --- | --- |
| [`data/__corrupt_out__/F1.out`](data/__corrupt_out__/F1.out) | 删除 `scores.dat` 后运行 `--self-test` 的捕获输出。 | 对应 F1 用例。 |
| [`data/__corrupt_out__/F2.out`](data/__corrupt_out__/F2.out) | 篡改 `users.dat` magic 后运行 `--self-test` 的捕获输出。 | 对应 F2 用例。 |
| [`data/__corrupt_out__/F3.out`](data/__corrupt_out__/F3.out) | 伪造 `scores.dat` count 字段后的捕获输出。 | 对应 F3 用例。 |
| [`data/__corrupt_out__/sanity.out`](data/__corrupt_out__/sanity.out) | 三个损坏场景恢复后再次跑 `--self-test` 的输出。 | 用来证明还原干净。 |

补充说明：

- `data/__corrupt_backup__/` 是 `corrupt_check.bat` 运行时临时创建的备份目录，脚本结束后会删除
- `.dat` 文件都是二进制格式，不建议手工修改

## 11. `build/` 目录说明

`build/` 目录是 **CMake / Visual Studio 自动生成目录**，不属于手写业务逻辑。它可以整体删除，再通过 `cmake -S . -B build` 重新生成。

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

如果你是第一次读仓库，可以把 `build/` 当成“自动生成、可以忽略细节”的目录；你真正需要理解的业务代码都在 `src/` 和 `include/`。

## 12. 代码结构与职责边界

仓库虽然文件不少，但架构逻辑其实很简单：

```text
用户 -> View 菜单 -> Service 规则 -> Storage 仓储 -> .dat 文件
                       |
                       -> StatsService -> ReportExporter -> .txt/.csv
```

四条最重要的边界如下：

1. `view/` 只做交互，不直接读写 `.dat`
2. `service/` 集中权限、校验、级联和统计规则
3. `storage/` 只负责显式序列化，不做业务判断
4. `model/` 只描述数据结构，不混入菜单与算法

这四条边界正是项目后续能否平滑接入 Qt 的关键。因为只要这四条不破，未来换 GUI 时基本只需要替换 `view/` 和 `src/app/main.cpp` 的主循环。

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

### 13.2 `tools/corrupt_check.bat`

这条入口负责 F 组“坏文件”场景：

- F1：删除 `scores.dat`
- F2：把 `users.dat` 头四字节从 `EDSY` 改成 `XXXX`
- F3：把 `scores.dat` 的计数字段虚报为 `255`

这条脚本外置而不是内置进 `--self-test`，是因为项目明确坚持：**自检程序本体尽量只读，坏数据由外部脚本制造、外部脚本恢复。**

### 13.3 `demo_input.txt`

它不是测试用例表，而是一条“演示脚本”：

- 先以 `admin` 登录，新增学生、录入成绩、查看统计、导出预警报告
- 再以 `t001` 登录，只查看并操作自己的课程
- 最后以 `s001` 登录，只看自己的资料、成绩、GPA

这条脚本非常适合：

- 录屏
- 答辩前彩排
- 验证交互菜单是否仍然连贯

## 14. 数据格式与输出约定

### 14.1 `.dat` 文件头

所有 `.dat` 仓储都走统一头部格式：

```text
magic[4] = 'E' 'D' 'S' 'Y'
version  = uint32
count    = uint32
```

这样做的原因有三个：

- 能尽早识别“这不是合法的 EduSys 数据文件”
- 未来如果要升级格式，可以依赖 `version` 做兼容判断
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

两者都复用 `StatsService` 的结构化结果，不改任何统计口径。`ReportExporter.cpp` 里还额外做了：

- CSV 字段转义
- 文件名安全字符检查
- 导出日志记录

## 15. 如果以后要接 Qt，这棵目录树会怎么变化

这也是很多人看目录树时最关心的问题。

### 15.1 该改的部分

- `src/view/*.cpp`
- `include/EduSys/view/*.hpp`
- `src/app/main.cpp` 的主循环与菜单分派方式

### 15.2 不该改的部分

- `include/EduSys/model/*` 与 `src/model/*`
- `include/EduSys/storage/*` 与 `src/storage/*`
- `include/EduSys/service/*` 与 `src/service/*`
- `include/EduSys/report/ReportExporter.hpp`
- `src/report/ReportExporter.cpp`
- `Session`、异常体系、日志体系、常量定义

换句话说，你现在看到的这棵树，其实已经把“未来 GUI 会变的部分”和“未来 GUI 不该动的部分”分开了。这正是 `claude.md` 一开始就强调的设计目标。

## 16. 开发阶段回顾

| 周次 | 完成内容 |
| --- | --- |
| Week 9 | 项目骨架、命名空间、核心实体模型、基本目录树 |
| Week 10 | 二进制读写器、模板仓储、`.dat` 文件格式、持久化设施 |
| Week 11 | 认证、会话、三大领域服务、级联删除、端到端自检 |
| Week 12 | 视图层菜单、统计分析、学业预警、报告导出 |
| Week 13 | 集成测试、边界测试、F 组文件损坏脚本 |
| Week 14 | 架构图册、答辩稿、CSV 导出、Qt 适配讨论、README 总整理 |

## 17. 推荐阅读路径

如果你是不同角色，可以按下面方式看仓库：

| 你的身份 | 推荐先看什么 |
| --- | --- |
| 第一次接手这个项目的同学 | 本 README 的第 4-12 节 |
| 想直接改功能的开发者 | `include/` 对应接口头文件，再看 `src/` 实现 |
| 想准备答辩的人 | `docs/architecture.md` + `docs/defense.md` + 本 README 第 12-15 节 |
| 想验证项目可靠性的人 | `docs/test-cases.md` + `tools/corrupt_check.bat` |
| 想未来做 Qt 原型的人 | `docs/architecture.md` 末尾的 Qt 段 + `src/app/main.cpp` + `view/` 目录 |

## 18. 一句话总结这棵目录树

如果只用一句话概括这整个仓库：

> `include/` 负责讲清楚接口和边界，`src/` 负责把业务跑起来，`docs/` 负责把设计与答辩讲明白，`data/` 负责承载运行状态，`tools/` 负责验证坏场景，`build/` 只是自动生成的构建现场。

读懂这一句，再回头看目录树，就不会觉得它只是“文件很多”，而会知道每个文件为什么在这里、该在什么时候看它。
