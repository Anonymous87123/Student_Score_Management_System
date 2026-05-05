# EduSys 答辩问答稿（Q&A）

> Week 14 文档收尾产物。每条 Q&A **30 秒口述版**：把"为什么这样写"加工成可背的答案，**每条都附当前仓库文件 + 行号**对证，避免空泛。术语首次出现时附英文（如 PermissionException）。
>
> 配合 [`docs/architecture.md`](architecture.md) 的图册一起用：图册回答"长什么样"，本文回答"老师追问为什么"。

## 目录

- [一、架构与分层（高频）](#一架构与分层高频)
- [二、持久化 / 二进制格式](#二持久化--二进制格式)
- [三、权限、认证与安全](#三权限认证与安全)
- [四、测试与边界（Week 13）](#四测试与边界week-13)
- [五、可扩展性](#五可扩展性)

---

## 一、架构与分层（高频）

### Q1 为什么要分 4 层？菜单直接读写文件不行吗？

**答**：分 View / Service / Storage / Model 四层是为了让"会变"和"不变"的代码隔开。控制台菜单（View）很可能将来换成 Qt，但学生 / 课程 / 成绩的数据模型（Model）和增删改查规则（Service）不会变。如果菜单直接 `ofstream` 写 `.dat`，那一旦换 Qt 就要把整套权限、级联、字段校验逻辑再抄一遍。现在分层之后，**Service 层一行不动就能换 GUI**。

**对证**：
- 视图层不 `#include` 任何 `storage/`：[`src/view/AdminMenu.cpp`](../src/view/AdminMenu.cpp) 顶部 include 列表里只有 `service/`、`report/`、`view/BaseMenu.hpp`。
- Service 引用在 [`src/app/main.cpp:397-402`](../src/app/main.cpp#L397-L402) 统一构造一次后注入到三个菜单。

---

### Q2 为什么 `ReportExporter`（报告导出器）单独放在 `report/` 子目录，不是 `service/` 或 `view/`？

**答**：因为它的职责跨在三层之间，归任何一层都会破坏边界。它**不是 Service**：不做权限分支，权限由它内部传入的 StatsService 检查；它**不是 View**：不读 `cin`、不打印到 `cout`；它**不是 Storage**：写的是文本 `.txt` / `.csv`，不是结构化的 `.dat`。所以专门给它一个独立的 `report/` 子目录，明确它就是一个**轻量"格式化 + 写文件"适配器（Adapter）**。Week 14 的 CSV 导出在同一个类里加方法，不破坏这条边界。

**对证**：
- 头文件注释正是这段话：[`include/EduSys/report/ReportExporter.hpp:10-16`](../include/EduSys/report/ReportExporter.hpp#L10-L16)。
- 实现里没有任何 `cin` / `RoleType` 检查：[`src/report/ReportExporter.cpp`](../src/report/ReportExporter.cpp)。
- 权限调用：[`src/report/ReportExporter.cpp:49`](../src/report/ReportExporter.cpp#L49) 直接 `stats_.computeAllWarnings(session)`，权限失败由 StatsService 抛 `PermissionException`。

---

### Q3 为什么 `UserAccount`（登录账号）不继承 `Person`（人员基类）？

**答**：登录账号和业务实体是**两种不同的事物**。管理员（admin）只是一个登录账号，不是任何具体的"人"，没有学号、班级、入学年。强行让 admin 也继承 Person 就要塞一堆空字段。所以把账号 / 密码 / 角色 / 关联实体编号放进独立的 `UserAccount`，用 `ownerId` 字符串关联到 Student 或 Teacher（如有）。这样 admin 的 `ownerId` 就是空字符串，含义清晰。

**对证**：
- `UserAccount` 是独立类，不继承 Person：[`include/EduSys/model/UserAccount.hpp`](../include/EduSys/model/UserAccount.hpp)。
- admin 账号 `ownerId` 留空：[`src/app/main.cpp:108`](../src/app/main.cpp#L108) `users.emplace_back("admin", adminHash, RoleType::Admin, "", true);`

---

### Q4 为什么 `Score`（成绩）做成独立实体，不嵌进 `Student`？

**答**：如果 Score 嵌在 Student 内部（比如 `Student::scores` 是个 vector），那删除学生时关联成绩自动消失，听起来很方便。但这会带来三个问题：(1) 二进制文件 Student 一行就要塞下变长成绩列表，序列化变复杂；(2) 跨学生查"某课程平均分"要遍历所有 Student；(3) 一旦想换数据库，关系映射就乱。所以 Score 独立成表，用 `(studentId, courseId, semester)` 三元组当主键 —— 这就是关系数据库的标准做法。

**对证**：
- Score 是独立实体：[`include/EduSys/model/Score.hpp`](../include/EduSys/model/Score.hpp)。
- 删学生时一行清掉所有关联成绩：[`src/service/StudentService.cpp:103-108`](../src/service/StudentService.cpp#L103-L108)。
- 查"某课程统计"也是一行：`StatsService::computeCourseStats` 里 `std::remove_if(... s.getCourseId() != courseId)`。

---

## 二、持久化 / 二进制格式

### Q5 为什么不用 `reinterpret_cast` 整块写对象？听起来更快。

**答**：因为含 `std::string` 的对象**根本不能整块写**。`std::string` 内部是指向堆上字符数据的指针 + size + capacity，整块写出去只会把指针值（地址）写进文件，下次启动时这些地址全是野指针，恢复不出来。整块写还会暴露内部布局，跨编译器 / 跨标准库实现都会失效。所以 EduSys 强制**逐字段显式编码**，字符串走 `length:u32 + bytes`，数值走主机字节序。

**对证**：
- 编码契约：[`include/EduSys/storage/BinaryRepository.hpp:18-25`](../include/EduSys/storage/BinaryRepository.hpp#L18-L25)（注释里明确"实体布局完全由 T::writeTo / T::readFrom 决定"）。
- 实体 `writeTo` 实现：[`src/model/Score.cpp`](../src/model/Score.cpp)、[`src/model/Student.cpp`](../src/model/Student.cpp)。
- README 有 32 字节实际十六进制对证：[`README.md` "实际字节对证"段](../README.md)。

---

### Q6 文件头为什么要 magic + version？项目这么小有必要吗？

**答**：magic 是**第一道防误读**：把任何 `.txt` 文件、随机文件改名成 `.dat` 都会立刻被识破。version 是**第二道防演进**：将来字段顺序如果要变（比如 Score 加一个学期权重字段），加载老 `.dat` 时就能根据 version 走旧解码路径，而不是默默读出乱数据。Week 13 F 组就专门测了 magic 损坏 → 立刻抛 `StorageException`，没让坏数据流进 Service 层。

**对证**：
- magic / version 定义：[`include/EduSys/storage/BinaryRepository.hpp:29-30`](../include/EduSys/storage/BinaryRepository.hpp#L29-L30)。
- magic 校验抛异常：[`include/EduSys/storage/BinaryRepository.hpp:88-100`](../include/EduSys/storage/BinaryRepository.hpp#L88-L100)。
- F2 自动化验证：[`tools/corrupt_check.bat`](../tools/corrupt_check.bat) 把 `EDSY` 改成 `XXXX`，期望 rc=1 + `Bad magic` —— 见 [`docs/test-cases.md` F2](test-cases.md)。

---

### Q7 删学生时怎么保证 `scores.dat` 不留幽灵？

**答**：删学生走 Service 层的**三步级联**：先按 `studentId` 清掉 `scores.dat` 里所有相关成绩 → 再清 `users.dat` 里 `role==Student && ownerId==id` 的登录账号 → 最后删 `students.dat` 里学生本人。**任一步抛异常立即终止**，不会留半成品。Week 11 选了 Option A（物理删除，不做软删），Week 13 D 组三条断言（D1/D2/D3）专门验证了删完后三个 `.dat` 都不残留 S002 的引用。

**对证**：
- 三步级联：[`src/service/StudentService.cpp:91-130`](../src/service/StudentService.cpp#L91-L130)。
- D 组断言：[`docs/test-cases.md` D 表](test-cases.md)、对应代码 [`src/app/main.cpp` runWeek13BoundaryCheck D 段](../src/app/main.cpp#L344-L356)。

---

## 三、权限、认证与安全

### Q8 密码不是明文存的，但 FNV-1a 不是密码学哈希啊？

**答**：**这是有意识的取舍，写在文档里的**。FNV-1a + 固定盐（fixed salt）的目的有两个：(1) 避免明文落盘，万一同学拷走 `users.dat` 不能直接读出密码；(2) 跨平台跨进程结果稳定，不依赖任何外部库。它**不能抵抗**有目标的暴力破解、彩虹表攻击；项目的 README 和 `PasswordHasher` 头文件都写明了"不宣称达到密码学强度"。如果要做生产级，应该换 bcrypt / Argon2，但课程作业不让引第三方库，且这不是本项目的考察点。

**对证**：
- 实现：[`src/common/PasswordHasher.cpp:12`](../src/common/PasswordHasher.cpp#L12)（固定盐）+ [`:18`](../src/common/PasswordHasher.cpp#L18)（FNV-1a 64-bit）+ [`:36`](../src/common/PasswordHasher.cpp#L36)（盐 + 明文）。
- 主动声明非密码学强度：[`README.md` "已完成（累积）"上方 Week 11 PasswordHasher 段](../README.md)。

---

### Q9 教师只能录自己授课课程的成绩，这个限制是怎么"双重防线"的？

**答**：双重防线 = **View 层白名单 + Service 层硬拒绝**，任一道独立有效。
- View 层：[`TeacherMenu::pickOwnCourseId`](../src/view/TeacherMenu.cpp#L65-L82) 先把所有课程过滤成"我授课的"白名单（`c.getTeacherId() == session_.getOwnerId()`），用户只能从白名单里选 courseId，连别的课的 id 都打不进来。
- Service 层：就算前端被绕过（比如未来换 GUI 漏判），[`ScoreService::upsert`](../src/service/ScoreService.cpp#L146-L148) 后端再拒一次：`session.isTeacher() && course.getTeacherId() != session.getOwnerId()` → 抛 `PermissionException`。Week 13 E1/E2 用伪造 `ownerId=T999` 的 ghost 教师专门验过 Service 层这道线。

**对证**：
- View 白名单：[`src/view/TeacherMenu.cpp:65-82`](../src/view/TeacherMenu.cpp#L65-L82) + 调用点 [`:113, :134, :149, :160`](../src/view/TeacherMenu.cpp#L113)。
- Service 硬拒：[`src/service/ScoreService.cpp:146-148`](../src/service/ScoreService.cpp#L146-L148)。
- E 组断言：[`docs/test-cases.md` E 表](test-cases.md)。

---

### Q10 三种异常 `AuthException` / `PermissionException` / `ValidationException` 是怎么区分使用的？

**答**：按错的"位置"分。
- `AuthException`：**没登录** 或 **认证失败**。比如未登录就调 Service、用户名 / 密码错。
- `PermissionException`：**登录了但角色不对**。比如学生想删别人的记录。
- `ValidationException`：**字段不合法 / 主键冲突 / 关联缺失**。比如 usual=101、courseId 已存在、teacherId 不在 teachers.dat 里。

每个都继承自项目根异常 `EduException`，main 的 try / catch 用基类一把兜住后退出码 1，但分类抛能让日志和测试用例精确定位错的层次。

**对证**：
- 异常族定义：[`include/EduSys/common/Exception.hpp`](../include/EduSys/common/Exception.hpp)（5 个类，全部继承 `EduException`）。
- 三类异常在 Service 中按上述规则使用：[`src/service/StudentService.cpp:14-21`](../src/service/StudentService.cpp#L14-L21)（`requireAdmin` 同时处理 AuthException 与 PermissionException 两种语义）。

---

## 四、测试与边界（Week 13）

### Q11 怎么证明这个项目"真的能用"？

**答**：两条入口，一键复跑。
1. **`edusys.exe --self-test`**：Week 11 端到端 6 条主路径 + Week 13 边界 22 条断言（A-E 五组），全部走 `assertThrowsWeek13<Exc>(label, lambda)` 集中模板，每条独立打 PASS。退出码 0 = 全绿。
2. **`tools/corrupt_check.bat`**：F 组 3 条文件损坏用例（删文件 / 坏 magic / 虚报 count），脚本备份 → 篡改 → 跑 exe → 还原 → 末尾再跑一次 `--self-test` 当 sanity，证明备份还原干净。

加上 Week 12 的 [`demo_input.txt`](../demo_input.txt) 三段演示路径（Admin / Teacher / Student 各一段），整个交付物是可验证的。

**对证**：
- A-E 22 条断言：[`src/app/main.cpp:269-385`](../src/app/main.cpp#L269-L385)。
- F 脚本：[`tools/corrupt_check.bat`](../tools/corrupt_check.bat)。
- 全部用例汇总：[`docs/test-cases.md`](test-cases.md)。

---

### Q12 为什么 F 组（文件损坏）要外置成 `.bat`，不能放进 `--self-test` 吗？

**答**：因为 Week 13 的边界守则是 **`--self-test` 全程只读、不动 `.dat`**。让 exe 自己改自己的数据文件再还原，不仅破坏只读语义，万一异常退出还会留半残文件。F 组本质是"在加载阶段被动撞上坏数据"，最干净的做法是**外部脚本造坏 → 跑 exe → 收 rc → 外部脚本还原**。脚本结尾必跑一次 `--self-test` 当 sanity，rc 不为 0 直接报失败。

**对证**：
- 脚本结构：[`tools/corrupt_check.bat`](../tools/corrupt_check.bat) 顶部注释段 "Why external" 已明确这条理由。
- 这条决策是用户在 Week 13 计划阶段拍的"路径 1 + F 组 β + 表格式 docs"。

---

## 五、可扩展性

### Q13 如果要换 Qt 图形界面，需要改多少代码？

**答**：**几乎只改 `view/` 与 `app/main.cpp` 主循环**。Service / Storage / Model / Session / ReportExporter 全部一行不动。原因是 Service 接口已经是 GUI 中立的：返回 `std::vector<Student>` / 自定义结构体 / 抛异常，没有任何 `cout` / `cin` 介入。Qt 主窗口直接持有一个 `Session` 实例 + 一组 Service 引用，slot 里调 Service 拿到 `vector` 就喂给 `QTableView`。详细的"该改 / 不该改"清单在 [`docs/architecture.md`](architecture.md) 末尾的 Qt 适配段。

**对证**：
- Service 接口的 GUI 中立性：[`include/EduSys/service/StatsService.hpp`](../include/EduSys/service/StatsService.hpp)（返回 `GpaResult` / `CourseStats` / `vector<RankEntry>` / `vector<WarningEntry>`，没有任何字符串排版逻辑）。
- 当前 View 层和 Service 层的耦合点：[`src/app/main.cpp:451-467`](../src/app/main.cpp#L451-L467) `switch (acc.getRole())` 实例化三个 Menu，换 Qt 时这一段换成 `QApplication::exec()` 与 QDialog 即可。

---

### Q14 Week 14 加的 CSV 导出，和原来的 `.txt` 预警报告是什么关系？

**答**：**复用同一个 ReportExporter，不改任何统计口径或菜单主流程**。ReportExporter 现在有三个方法：
- `exportWarningReport` → `data/warning_report.txt`（Week 12 既有）
- `exportCourseStatsCsv(courseId)` → `data/course_stats_<courseId>.csv`（Week 14 新增）
- `exportRankingCsv(courseId)` → `data/ranking_<courseId>.csv`（Week 14 新增）

数据全部走 `StatsService` 同一组结构化结果，CSV 只是换一种格式落盘。Admin 菜单加一项"7. Export to CSV"，触发后选课程 → 输出两个 CSV 文件。**Service / Storage / Model / 权限矩阵 / 既有菜单 1-6 完全没动**。

**对证**：
- ReportExporter 头文件：[`include/EduSys/report/ReportExporter.hpp`](../include/EduSys/report/ReportExporter.hpp)。
- AdminMenu 第 7 项：[`src/view/AdminMenu.cpp` run() switch 段](../src/view/AdminMenu.cpp#L37-L63)。

---

## 一句话总结（开场可用）

> "EduSys 是一个 4 层结构的 C++17 控制台学生成绩管理系统：View / Service / Storage / Model 单向依赖，Model 是纯数据，Storage 是逐字段二进制 + 文件头校验，Service 集中权限与级联，View 只做交互。整个项目用 `--self-test` + `corrupt_check.bat` 两条入口可一键回归 25 条用例 + sanity，文档分图册 / 答辩稿 / 测试汇总三份对证当前代码。"
