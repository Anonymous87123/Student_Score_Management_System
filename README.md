# EduSys - 学生成绩管理系统

## TL;DR（30 秒上手）

```bash
build.bat                       # 用 g++ 一键编译，产物 edusys.exe
edusys.exe --self-test          # Week 11 端到端 + Week 13 边界 22 条断言（rc=0 全绿）
tools\corrupt_check.bat         # Week 13 F 组：3 条文件损坏 + sanity（rc=0 全绿）
edusys.exe                      # 默认进入登录循环；空用户名直接退出
edusys.exe < demo_input.txt     # 一键复演 Week 12 三段 Admin/Teacher/Student 演示
```

**关键文档（按看图 → 看答辩 → 看测试 → 看周报顺序）**

| 文档 | 用途 |
| --- | --- |
| [`docs/architecture.md`](docs/architecture.md) | 5 张 Mermaid 架构图册 + 每张图对证当前文件 + Qt 适配讨论 |
| [`docs/defense.md`](docs/defense.md) | 答辩问答稿，14 条 Q&A 全部对证当前仓库文件 + 行号 |
| [`docs/test-cases.md`](docs/test-cases.md) | 集成 / 边界测试统一汇总（A-F 六组共 25 条 + sanity） |
| [`README.md`](README.md) | 本文件，按周累加的进度与运行证据 |
| [`claude.md`](claude.md) | 开题报告原文（设计理由的最终引用） |

## 怎么用这个系统（用户指南）

本节面向**第一次运行本项目的使用者**，只讲常规交互启动路径，不涉及 `--self-test` 自检模式（那是维护者用的，见后文"两条运行路径的通过证据"）。

### 入口与首次启动

```bash
build.bat                       # 首次编译：用 g++ 一键产出 edusys.exe
.\edusys.exe                    # Windows 下直接运行
```

- **当 `data/*.dat` 为空时，首次常规启动会自动创建目录并注入样例数据。** 控制台会显示 `Run mode : SEEDED (first run)`，之后再次启动即显示 `Run mode : LOADED (persisted)`，数据持久保留。
- 启动后进入登录循环：输入用户名 + 密码即登录；**连续 3 次认证失败**或**用户名处直接回车**，程序正常退出（rc=0）。

### 默认样例账号

> 以下均为课程项目内置样例账号；**密码仅用于首次演示**，登录后可通过菜单中的"修改密码"更新。

| 用户名 | 密码 | 角色 | 关联实体 |
| --- | --- | --- | --- |
| `admin` | `admin123` | Admin（管理员） | — |
| `t001` | `t001pw` | Teacher（教师） | 教师 T001（仅授课 C001） |
| `s001` | `s001pw` | Student（学生） | 学生 S001 |

### 三类角色分别能做什么

登录后按角色自动路由到三棵菜单树之一。具体菜单项编号见下方 [菜单结构与命令编号](#菜单结构与命令编号)，权限边界见 [权限矩阵（Week 11）](#权限矩阵week-11)。

- **Admin**：全量 CRUD（学生 / 课程 / 成绩）+ 全局统计 + 学业预警报告 + CSV 导出 + 改自身密码。
- **Teacher**：只能看 / 改**自己授课的课程**的成绩；课程统计也只限自己的课；可改自身密码。
- **Student**：只读**自己的**基本信息 / 成绩 / GPA；可改自身密码。

### 典型任务速查

| 想做什么 | 用哪个账号 | 菜单路径 | 产出 |
| --- | --- | --- | --- |
| 新增一名学生 | `admin` | `1 → 3` | 追加进 `data/students.dat` + 自动建 `users.dat` 账号 |
| 给学生录一门成绩 | `admin` / `t001` | Admin `3 → 4`；Teacher `3` | 追加或更新 `data/scores.dat` |
| 看某门课的平均分 / 及格率 | `admin` / `t001` | Admin `4 → 1`；Teacher `5` | 控制台直接打印 |
| 查某学生的 GPA | `admin` / `s001`（看自己） | Admin `4 → 3`；Student `3` | 控制台直接打印 |
| 生成学业预警报告 | `admin` | `5` | `data/warning_report.txt` |
| 导出某课程 CSV（统计 + 排名） | `admin` | `7` | `data/course_stats_<id>.csv` + `data/ranking_<id>.csv` |
| 改自己密码 | 任意角色 | Admin `6` / Teacher `6` / Student `4` | 新哈希写回 `users.dat` |
| 退出登录 | 任意角色 | 在角色主菜单输入 `0` | 回到登录提示符 |

菜单每一层都可以输入 `0` 返回上一级；在登录提示符按回车（空用户名）即安全退出程序。

### 遇到问题时

- **登录一直失败**：默认样例账号见上表；如果之前改过密码，可以删除 `data/users.dat`（**会丢所有账号**）后重启，让 seed 逻辑重新注入默认样例。
- **怀疑数据文件损坏**：跑 [`tools/corrupt_check.bat`](tools/corrupt_check.bat)，它会备份 → 故意制造 3 种坏数据 → 跑 exe → 还原 → 末尾再做一次 sanity 自检，rc=0 即一切正常。
- **想一键复演 Week 12 三段演示**：`edusys.exe < demo_input.txt`，stdin 脚本见 [`demo_input.txt`](demo_input.txt)。

## 当前进度

**第 14 周：文档收尾、CSV 导出与答辩准备**

- [`docs/architecture.md`](docs/architecture.md)：把 [`claude.md`](claude.md) 里 5 张 Mermaid 图（思维导图 / 分层 / 类关系 / 级联 / 登录）抽出来汇总成一份独立图册，每张图旁配"为什么这样画 + 当前文件:行号"对证；末尾追加 Qt 适配讨论段，列清"该改 / 不该改"清单。
- [`docs/defense.md`](docs/defense.md)：14 条 Q&A 答辩稿，覆盖架构 / 持久化 / 权限 / 测试 / 可扩展性五大类，**每条都点到具体文件 + 行号**，不空泛。
- **CSV 导出（α 选项落地）**：`ReportExporter` 新增两个方法 `exportCourseStatsCsv` / `exportRankingCsv`，复用 `StatsService` 同一组结构化结果，**不改任何统计口径**。Admin 菜单第 7 项触发，输入 courseId 后一次性导出 `data/course_stats_<id>.csv` 与 `data/ranking_<id>.csv`。
- README 中等整理：顶部加 TL;DR + 文档索引；Week 9-13 详细段落保留为历史；累积进度表升级到 Week 14（已完成 9-14、零待办）。

```text
edusys.exe --self-test                  -> Week 11+13 自检（rc=0）
tools\corrupt_check.bat                 -> Week 13 F 组（rc=0）
Admin Menu > 7. Export course CSV       -> Week 14 新增；输入 courseId 落盘两个 CSV
docs\architecture.md / defense.md       -> Week 14 答辩用图册 + 问答稿
```

**第 13 周：集成测试与边界测试**

- A-E 五组（认证 / 字段校验 / 唯一性 / 级联残留 / 教师白名单）通过 `edusys.exe --self-test` 一键回归，**全程只读，不动 `.dat`**。22 条断言走 `assertThrowsWeek13<Exc>(label, lambda)` 集中模板，每条独立打 PASS / 异常类型 / 错信息。
- F 组（文件层损坏：缺文件 / 坏 magic / 虚报 count）外置到 [`tools/corrupt_check.bat`](tools/corrupt_check.bat)：脚本先把 `data/*.dat` 备份到 `data/__corrupt_backup__/`，再用 PowerShell 单行改字节 → 跑 exe → 收 rc 与 stderr → 还原 → 末尾再跑一次 `--self-test` 当 sanity，证明备份还原是干净的。
- [`docs/test-cases.md`](docs/test-cases.md)：**统一汇总 A-F 全部 25 条用例 + sanity**，按"用例 ID / 输入 / 预期 / 实际 / 判定"五列展示，文末挂"关键日志证据"段，从 `data/app.log` 摘 WARN / ERROR 行做对证。
- 这一周的边界守则与之前一致：`--self-test` 内部坚决不改 `.dat`；要造坏数据只能走外部脚本；脚本造坏后必须自己还原；docs 是结论汇总，不是日志原文。

```text
edusys.exe --self-test       -> A-E 组（22 条断言，rc=0 全绿）
tools\corrupt_check.bat      -> F 组（3 条 + sanity，rc=0 全绿）
docs\test-cases.md           -> 全部 25 条 + sanity 的统一表格汇总
```

**第 12 周：视图层、统计分析与学业预警**

- `StatsService`（service）：**只返回结构化结果**，不承担任何文本排版 / 文件 I/O。四个入口：`computeGpaFor` / `computeCourseStats` / `rankByCourse` / `computeAllWarnings`；GPA 走 4.0 制分段加权，按课程学分加权平均。
- `ReportExporter`（**独立 report/ 子目录**）：唯一一个允许把 StatsService 结果落盘成 `.txt` 的组件。既不是 Service（不做业务规则），也不是 View（不读控制台），也不是 Storage（写的是文本而非 `.dat`）。Admin 点菜单第 5 项时才触发，生成 `data/warning_report.txt`。
- `BaseMenu`（view/，抽象基类）：**唯一允许 `std::cin / std::cout` 的地方**。派生菜单不再各自写 IO；共用 `readLine / readInt / readDouble / printTable / paginate / printError / printOk`。
- `AdminMenu / TeacherMenu / StudentMenu`：分别按角色暴露功能。**TeacherMenu 的录分 / 删分 / 统计 / 查成绩入口一律先调 `pickOwnCourseId()`，只允许用户在"我的授课课程"白名单里选**，双重防线（View 本地过滤 + Service 的 `courseIt->getTeacherId() == session.ownerId` 拒绝）。
- `main.cpp`：默认进入登录循环（3 次认证失败正常退出，不算 fatal）；`--self-test` 走 Week 11 回归自检。两条路径共享"`data/` 建目录 + 空仓注入样例"起步步骤。

**GPA 分段（4.0 制）**

| Total | GPA |
| --- | --- |
| ≥ 90 | 4.0 |
| 85-89 | 3.7 |
| 80-84 | 3.3 |
| 75-79 | 3.0 |
| 70-74 | 2.7 |
| 65-69 | 2.3 |
| 60-64 | 2.0 |
| < 60 | 0.0 |

**学业预警阈值**：`GPA < 2.0` **或** `failCount >= 2`（取并集）。

## 菜单结构与命令编号

登录后按角色路由到三棵菜单树之一。数字即为用户输入的选择。

```text
Login loop  (3 failures -> exit 0; empty username -> exit 0)
 └─ username / password → AuthService

Admin Menu
  1. Student management
     1. List all       2. View by id  3. Add        4. Edit
     5. Remove (cascade scores + user account)      0. Back
  2. Course management
     1. List all       2. View by id  3. Add        4. Edit
     5. Remove (cascade scores)                     0. Back
  3. Score  management
     1. List all       2. By student  3. By course
     4. Upsert (add/update)            5. Remove one score   0. Back
  4. Statistics
     1. Course stats   2. Ranking by course          3. Student GPA
     0. Back
  5. Generate warning report  ->  data/warning_report.txt
  6. Change my password
  7. Export course CSV (stats + ranking)  ->  data/course_stats_<id>.csv + data/ranking_<id>.csv
  0. Logout

Teacher Menu   (teacherId 严格过滤：操作一律走 pickOwnCourseId() 白名单)
  1. List my courses
  2. View scores for my course
  3. Record / update one score
  4. Delete one score
  5. Course statistics (my course only)
  6. Change my password
  0. Logout

Student Menu   (只读自己)
  1. View my profile
  2. View my scores
  3. View my GPA
  4. Change my password
  0. Logout
```

## 三条演示路径（与仓库里 [`demo_input.txt`](demo_input.txt) 对齐）

**管理员：增学生 → 给他录分 → 看课程统计 → 出预警报告**

```text
Login: admin / admin123
 3 → 1                      (Score management > List all)              ... 2 行
 0 → 1 → 3                  (Student management > Add)
 S004 / Li Qiang / 13800000004 / Computer Science / CS2501 / 2025
 0 → 3 → 4                  (Score management > Upsert)
 S004 / C001 / 2025-2026-1 / 45 / 50 / 47                            # 故意挂科
 0 → 4 → 1                  (Statistics > Course stats)
 C001                       → count=3 avg=77.30 max=94.10 min=47.00
                              pass=66.7% excellent=66.7%
 0 → 5                      (Generate warning report)
 → [OK] Warning report written to: data/warning_report.txt
 0                           (Logout)
```

**教师（t001，只能动 C001）：列我的课 → 看课程统计 → 看成绩**

```text
Login: t001 / t001pw
 1                           (List my courses)          → 仅 C001
 5 → C001                    (Course statistics)        → 同上一致的 3 门统计
 2 → C001                    (View scores for my course) → S001/S003/S004 三行
 0                           (Logout)
```

**学生（s001）：看基本信息 → 看成绩 → 看 GPA**

```text
Login: s001 / s001pw
 1                           (View my profile)          → Zhang San / CS2501 / 2025
 2                           (View my scores)           → C001 88/92/90.80
 3                           (View my GPA)              → Courses=1 Credit=4.0 GPA=4.00
 0                           (Logout)
```

随后在登录提示符上直接回车（空用户名）→ 程序正常退出。

## `warning_report.txt` 示例（Admin 路径执行完产出）

```text
============================================================
 EduSys Academic Warning Report
 Generated at : 2026-05-05 13:58:29
 Operator     : admin (Admin)
 Threshold    : GPA < 2.0  OR  failCount >= 2
 Hits         : 1
============================================================

StuId   Name               GPA  Fail  FailedCourses
------------------------------------------------------------
S004    Li Qiang          0.00     1  C001

--- end of report ---
```

命中逻辑对证：S001 总评 90.80 → GPA 4.00，S003 总评 94.10 → GPA 4.00，二者均不触发；S004 总评 47.00 → gpaPoint=0 → GPA 0.00 且挂 C001 → 命中。

## 两条运行路径的通过证据

**默认交互模式**（`edusys.exe < demo_input.txt` 一键复现）

```text
==============================================
 EduSys - Student Score Management System
 Week 12 build: interactive menus + stats
 Run mode        : LOADED (persisted)
==============================================
 ... (三次 [OK] Welcome / [OK] Logged out，见 demo_input.txt 对应顺序)
 [OK]  Warning report written to: data/warning_report.txt
 ... 最后
 Username (empty = quit): Bye.
```

**`--self-test` 回归模式**（Week 11 断言全绿，证明视图层改动没破坏 Service 边界）

```text
==============================================
 EduSys - Week 11 Self-Test (--self-test)
 Run mode        : LOADED (persisted)
 [Week11] AUTH OK            : admin (role=Admin)
 [Week11] AUTH FAIL expected : Invalid username or password
 [Week11] PERM REJECT ok     : student cannot create student
 [Week11] RANGE REJECT ok    : usual=999 rejected
 [Week11] MUTATIONS skipped  : LOADED run, verifying persisted state instead
 [Week11] FINAL ASSERT ok    : S002 absent, S003 present, scores consistent
 Week 11         : self-check PASSED
==============================================
```

对应 `data/app.log` 关键行：

```text
[INFO] EduSys starting up (mode=interactive).
[INFO] Auth OK: 'admin'
[INFO] Student created: S004
[INFO] Score created: S004/C001/2025-2026-1
[INFO] Warning report exported: hits=1 path=data/warning_report.txt
[INFO] Auth OK: 't001'
[INFO] Auth OK: 's001'
[INFO] EduSys shutdown normally (interactive).
[INFO] EduSys starting up (mode=self-test).
[INFO] Week 11 self-check PASSED.
```

**已完成（累积）**

- 第 9 周：工程骨架与核心实体模型
- 第 10 周：持久化基础设施与仓储层
- 第 11 周：认证、会话与三大领域服务 + 级联删除
- 第 12 周：视图层、统计分析与学业预警报告
- 第 13 周：集成测试与边界测试（A-E 内置 `--self-test`、F 外置 `tools/corrupt_check.bat`、统一汇总到 `docs/test-cases.md`）
- 第 14 周：文档收尾与答辩准备（`docs/architecture.md` + `docs/defense.md` + Admin CSV 导出 + Qt 适配讨论）

**尚未实现**

- （无）

**第 11 周：认证、会话与三大领域服务**

- `PasswordHasher`（common）：固定盐 + FNV-1a 64-bit -> 16 字符小写 hex 摘要；`hash` / `verify` 两个静态方法。仅为避免明文落盘 / 跨平台稳定，不宣称达到密码学强度。样例用户 `admin / admin123`、`t001 / t001pw`、`s001|s002 / s001pw` 初始化时统一走真实哈希。
- `Session`（纯值对象）：`loggedIn / username / role / ownerId`；提供 `login / logout / isAdmin / isTeacher / isStudent`，不持有任何仓储引用。
- `AuthService`：`authenticate(username, password) -> UserAccount`（失败一律抛 `AuthException` 并写 `WARN` 日志）；`changePassword(session, old, new)` 仅允许改自身。
- `StudentService / CourseService / ScoreService`：把权限检查、字段校验、级联删除集中收口；三者共享一条规则 —— **Storage 不碰业务，所有跨文件一致性都靠 Service 协调**。
- 级联删除语义（Option A：物理删除，不做软删）：
  - 删除学生 `id` → 先清 `scores.dat` 中 `studentId==id` → 再删 `users.dat` 中 `role==Student && ownerId==id` 的账号 → 最后删学生本人
  - 删除课程 `courseId` → 先清 `scores.dat` 中 `courseId==courseId` → 再删课程本人
- `main.cpp` 从 Week 10 自检升级为 Week 11 端到端自检；破坏性写路径仅在首次 seed 后执行一次（Option A：不复原），之后每次启动只跑只读 / 鉴权 / 权限拒绝 / 越界拒绝等可重入路径。

**已完成（累积）**

- 第 9 周：工程骨架与核心实体模型
- 第 10 周：持久化基础设施与仓储层
- 第 11 周：认证、会话与三大领域服务 + 级联删除

**尚未实现**

- 第 12 周：`BaseMenu / AdminMenu / TeacherMenu / StudentMenu` 与统计分析
- 第 13 周：集成测试
- 第 14 周：文档与答辩准备

## 权限矩阵（Week 11）

| 操作 | Admin | Teacher | Student |
| --- | --- | --- | --- |
| Student 读 | 全部 | 全部 | 仅自己 |
| Student 增 / 改 / 删 | 允许（删触发级联） | 拒绝 | 拒绝 |
| Course 读 | 全部 | 全部 | 全部 |
| Course 增 / 改 / 删 | 允许（删触发级联） | 拒绝 | 拒绝 |
| Score 读 | 全部 | 仅自己授课的课程 | 仅自己 |
| Score 写 | 全部 | 仅自己授课的课程 | 拒绝 |

拒绝一律抛 `PermissionException`；未登录抛 `AuthException`；字段越界或关联缺失抛 `ValidationException`。

## Week 11 自检证据（一次首启 + 一次持久化重启）

<details>
<summary>展开查看完整 SEEDED / LOADED 双跑输出 + app.log 片段</summary>

**Run 1：SEEDED（清空 `data/*.dat` 后首次启动，跑破坏性路径）**

```text
 Run mode        : SEEDED (first run)
 [Week11] AUTH OK            : admin (role=Admin)
 [Week11] AUTH FAIL expected : Invalid username or password
 [Week11] PERM REJECT ok     : student cannot create student
 [Week11] RANGE REJECT ok    : usual=999 rejected
 [Week11] MUTATIONS done     : +S003, S001 score updated, -S002 (cascade)
 [Week11] FINAL ASSERT ok    : S002 absent, S003 present, scores consistent
 users           : 3  (first=admin)
 students        : 2  (first=S001)
 scores          : 2  (first=S001/C001)
```

**Run 2：LOADED（紧接着不清数据再跑，跳过破坏性路径，只做回归断言）**

```text
 Run mode        : LOADED (persisted)
 [Week11] AUTH OK            : admin (role=Admin)
 [Week11] AUTH FAIL expected : Invalid username or password
 [Week11] PERM REJECT ok     : student cannot create student
 [Week11] RANGE REJECT ok    : usual=999 rejected
 [Week11] MUTATIONS skipped  : LOADED run, verifying persisted state instead
 [Week11] FINAL ASSERT ok    : S002 absent, S003 present, scores consistent
 users           : 3  (first=admin)
 students        : 2  (first=S001)
 scores          : 2  (first=S001/C001)
```

两次运行计数完全一致，证明：

1. 级联删除在 Run 1 中落盘（users 从 4 → 3、S002 相关 Score 被清）；
2. Run 2 从 `.dat` 直接加载即得到同样的最终状态；
3. Run 2 仍能用 `admin / admin123` 通过认证 —— 说明 FNV-1a 哈希跨进程稳定、`users.dat` 写入的摘要格式正确。

对应 `data/app.log` 片段：

```text
[INFO] All repositories empty -> seeding initial sample data.
[INFO] Auth OK: 'admin'
[WARN] Auth failed: bad password for 'admin'
[INFO] Student created: S003
[INFO] Score created: S003/C001/2025-2026-1
[INFO] Score updated: S001/C001/2025-2026-1
[INFO] Student removed (cascade): id=S002 scores=-1 users=-1
[INFO] Week 11 self-check PASSED.

[INFO] Existing data files detected -> skip seeding.
[INFO] Auth OK: 'admin'
[WARN] Auth failed: bad password for 'admin'
[INFO] Week 11 self-check PASSED.
```

</details>

**第 10 周：二进制读写与仓储层**

- `BinaryReader / BinaryWriter`：显式逐字段编码；字符串 = `u32 长度 + 字节`，数值 = 主机字节序原始字节（单平台课程项目约束）；禁用 `reinterpret_cast` 整块写对象。
- 各实体加 `writeTo(BinaryWriter&) const` / `static X readFrom(BinaryReader&)`；`Person` 作为抽象基类不直接参与序列化。
- `BinaryRepository<T>`（模板头文件）：只负责文件头、整批 `loadAll` / `saveAll`；**不含任何实体字段知识**，实体布局全部由 `T::writeTo / T::readFrom` 决定。
- 5 个具体仓储：`UserRepository / StudentRepository / TeacherRepository / CourseRepository / ScoreRepository`，各自绑定 `data/*.dat`。
- `main.cpp`：启动即做「保存→退出→重启→重新加载」自检闭环；**样例注入只在五个仓库全部为空时触发一次**，后续启动直接读取 `.dat`。
- MSVC `C4996 std::strerror` 告警一并清理，改用 `errno` 数值上报。

**已完成（累积）**

- 第 9 周：工程骨架与核心实体模型
- 第 10 周：持久化基础设施与仓储层

**尚未实现**

- 第 11 周：`AuthService / Session / StudentService / CourseService / ScoreService`
- 第 12 周：`BaseMenu / AdminMenu / TeacherMenu / StudentMenu` 与统计分析
- 第 13 周：集成测试
- 第 14 周：文档与答辩准备

## `.dat` 二进制格式规格

每个 `data/*.dat` 都采用同一布局：12 字节文件头 + N 条记录。

```text
File header (12 bytes)
  offset 0x00  magic[4]   = 'E' 'D' 'S' 'Y'   (0x45 0x44 0x53 0x59)
  offset 0x04  version    = uint32_t          (当前 = 1)
  offset 0x08  count      = uint32_t          (后续记录数 N)

String encoding
  length     = uint32_t             (字节数；上限 1 MiB，防御性校验)
  bytes[length]

Primitive encoding
  uint8_t    = 1 byte
  int32_t    = 4 bytes               (主机字节序)
  uint32_t   = 4 bytes               (主机字节序)
  double     = 8 bytes               (主机字节序 IEEE 754)
```

每类实体的字段顺序由各自的 `writeTo` 决定：

| 文件 | 记录字段（按写入顺序） |
| --- | --- |
| `users.dat` | `username:str, passwordHash:str, role:u8(0=Admin,1=Teacher,2=Student), ownerId:str, enabled:u8` |
| `students.dat` | `id:str, name:str, contact:str, major:str, className:str, enrollYear:i32` |
| `teachers.dat` | `id:str, name:str, contact:str, department:str, title:str` |
| `courses.dat` | `courseId:str, courseName:str, credit:f64, teacherId:str, semester:str` |
| `scores.dat` | `studentId:str, courseId:str, semester:str, usual:f64, final:f64, total:f64` |

### 实际字节对证（`users.dat` 头 32 字节）

```text
000000 45 44 53 59 01 00 00 00 04 00 00 00 05 00 00 00  EDSY............
000010 61 64 6d 69 6e 10 00 00 00 50 4c 41 43 45 48 4f  admin....PLACEHO
```

- `45 44 53 59` → magic `EDSY`
- `01 00 00 00` → version 1
- `04 00 00 00` → count 4（4 条 UserAccount 记录）
- `05 00 00 00` → 后续字符串长度 5
- `61 64 6d 69 6e` → `"admin"`
- `10 00 00 00` → 下一字段字符串长度 16
- `50 4c 41 43 45 48 4f ...` → `"PLACEHO..."`（占位哈希前缀）

## 构建：二选一硬规约（Week 14 α 收口）

> 历史上 README 同时给了 CMake 与 g++ 两套构建路径，目的是兼容"老师只装了 g++"和"老师只装了 MSVC"两种环境，**这条双轨承诺保留**。但落地约束变严：

- **二选一**：要么走 `build.bat`（产物 `./edusys.exe`），要么走 `cmake --build build --config Release`（产物 `build/Release/edusys.exe`）。**两条路径不要并存** —— 如果两个 exe 都在磁盘上，删掉其中一个，避免脚本认错。
- **canonical artifact = `./edusys.exe`**：[`tools/corrupt_check.bat`](tools/corrupt_check.bat) 与 [`demo_input.txt`](demo_input.txt) 默认认这一份。MSVC 用户跑完 `cmake --build` 后，请 `copy /y build\Release\edusys.exe .` 把它"提"到根目录再跑测试。
- **`.gitignore` 已忽略两个 exe**：[`/edusys.exe`](.gitignore) 与 [`/build/Release/edusys.exe`](.gitignore)；远端不留二进制，每个开发者本地都从源码重编。
- **mtime 防呆**：`tools/corrupt_check.bat` 启动时会用 PowerShell 检查
  - `src/` 或 `include/` 任何文件比 `./edusys.exe` 新 → **拒跑**（rc=2）
  - `build/Release/edusys.exe` 存在且比 `./edusys.exe` 新 → **拒跑**（rc=2，提示 CMake 产物更新）

  避免对着旧 exe 跑测试出假绿。区别于 F 组失败 rc=1，便于上层自动化分流。

## 构建（推荐：CMake）

要求：CMake ≥ 3.15，C++17 编译器（MSVC / MinGW-w64 g++ / clang）。

```bash
cmake -S . -B build
cmake --build build --config Release
```

产物位于 `build/edusys`（Linux/macOS）或 `build/Release/edusys.exe`（Windows / MSVC）。

## 构建（备用：g++ 兜底）

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude \
    src/app/main.cpp \
    src/common/Logger.cpp \
    src/model/Student.cpp src/model/Teacher.cpp src/model/UserAccount.cpp \
    src/model/Course.cpp  src/model/Score.cpp \
    src/storage/BinaryReader.cpp src/storage/BinaryWriter.cpp \
    -o edusys
```

## 运行与自检

```bash
build\Release\edusys.exe      # Windows / MSVC
./edusys                      # Linux/macOS / g++ 兜底
```

- 第一次启动：五个 `.dat` 不存在 → 注入样例 → 控制台显示 `Run mode: SEEDED (first run)`，日志记录 `All repositories empty -> seeding initial sample data.`
- 之后启动：读取已有 `.dat` → 控制台显示 `Run mode: LOADED (persisted)`，日志记录 `Existing data files detected -> skip seeding.`；计数 / 首条记录 ID 与上次一致即证明持久化闭环成立。

## 项目结构

```text
Student_Score_Management_System/
├── CMakeLists.txt
├── README.md
├── claude.md
├── build.bat
├── demo_input.txt                 # Week 12 三段演示路径的 stdin 脚本
├── data/                          # 运行时自动创建
│   ├── app.log
│   ├── users.dat / students.dat / teachers.dat
│   ├── courses.dat / scores.dat
│   ├── warning_report.txt          # Admin 第 5 项触发后产出
│   ├── course_stats_<id>.csv       # Week 14: Admin 第 7 项产出
│   └── ranking_<id>.csv            # Week 14: Admin 第 7 项产出
├── docs/
│   ├── architecture.md             # Week 14: 5 张图册 + Qt 适配讨论
│   ├── defense.md                  # Week 14: 14 条 Q&A 答辩稿
│   └── test-cases.md               # Week 13: 集成 / 边界测试统一汇总
├── tools/
│   └── corrupt_check.bat           # Week 13: F 组外置文件损坏脚本
├── include/EduSys/
│   ├── common/   Types / Constants / Exception / Logger / PasswordHasher
│   ├── model/    Person / Student / Teacher / UserAccount / Course / Score
│   ├── storage/  BinaryReader / BinaryWriter / BinaryRepository<T>
│   │             + UserRepository / StudentRepository / TeacherRepository
│   │             + CourseRepository / ScoreRepository
│   ├── service/  Session / AuthService / StudentService
│   │             / CourseService / ScoreService / StatsService
│   ├── report/   ReportExporter（warning .txt + course/ranking .csv）
│   └── view/     BaseMenu / AdminMenu / TeacherMenu / StudentMenu
└── src/
    ├── app/      main.cpp（含 runWeek11SelfCheck + runWeek13BoundaryCheck）
    ├── common/   Logger.cpp / PasswordHasher.cpp
    ├── model/    Student / Teacher / UserAccount / Course / Score .cpp
    ├── storage/  BinaryReader.cpp / BinaryWriter.cpp
    ├── service/  AuthService / StudentService / CourseService
    │             / ScoreService / StatsService .cpp
    ├── report/   ReportExporter.cpp
    └── view/     BaseMenu / AdminMenu / TeacherMenu / StudentMenu .cpp
```

## 设计约束（摘自 `claude.md`）

- Model 层只表达数据，**不得出现** `std::cin / std::cout / 菜单文本 / 文件 I/O`
- 实体关系一律通过 `id` 字符串关联，不互相持有裸指针
- 所有多态基类（`Person` 等）必须有虚析构函数
- 二进制序列化必须逐字段显式编码，禁止 `reinterpret_cast` 整块写对象
- Storage 层只负责文件读写，**不做**业务规则 / 权限判断；那些属于 Service 层（Week 11）
