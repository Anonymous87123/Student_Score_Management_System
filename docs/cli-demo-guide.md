# 学生成绩管理系统 CLI 端功能演示教程

本文档用于课堂现场演示控制台版本功能。演示时只需要手动输入命令和菜单选项，不依赖 `bat` 脚本。

建议按本文档顺序演示。文档使用临时学生 `S900`、临时课程 `C900`、临时成绩 `S900 / C900 / 2025-2026-1`，最后会把临时数据删除，避免影响原有样例数据。

因为本系统使用 `data/*.dat` 做持久化保存，列表、统计、预警报告里的完整内容可能会随你电脑上的历史数据变化。现场演示时不用追求每一行都和本文档完全一样，重点看关键标记是否出现，例如 `S900`、`C900`、对应分数、成功提示和导出路径。

## 目录

1. 演示前确认
2. 启动 CLI 程序
3. 登录功能演示
4. Admin 端功能演示
5. Teacher 端功能演示
6. Student 端功能演示
7. 修改密码功能演示
8. 清理临时数据
9. 可选补充演示
10. CLI 功能覆盖清单

## 1. 演示前确认

### 1.0 当前样例数据说明

本项目的数据是持久化保存在 `data/*.dat` 中的。当前仓库常见状态不是最初 seed 后的纯初始状态，而是已经经过自检和前期演示后的状态。

通常可以稳定使用的数据是：

```text
账号：
admin / admin123
t001  / t001pw
s001  / s001pw

学生：
S001 Zhang San
S003 Wang Wu
S004 Li Qiang

课程：
C001 Advanced Programming (C++), teacherId = T001
```

注意：

```text
不要用 s002 或 s004 登录。当前通常没有这两个登录账号。
不要现场删除 C001、S001、S003、S004 这些样例主数据。
本文档使用 S900、C900 做临时演示数据，最后会清理。
```

其中 `S004 Li Qiang` 是当前仓库数据文件里常见的持久化数据，不是源码首次 seed 必然生成的数据。如果你的现场数据里没有 `S004`，不影响本文档主线演示。

### 1.1 打开终端并进入项目目录

输入：

```powershell
cd D:\Student_Score_Management_System
```

如果不确定当前目录，可以输入：

```powershell
pwd
```

预期路径包含：

```text
D:\Student_Score_Management_System
```

### 1.2 确认控制台程序存在

输入：

```powershell
dir edusys.exe
```

预期能看到：

```text
edusys.exe
```

如果没有 `edusys.exe`，先编译：

```powershell
cmd /c build.bat
```

预期关键输出：

```text
Build OK -> edusys.exe
```

再确认运行目录下有 `data` 目录：

```powershell
dir data
```

如果提示找不到 `data`，先创建：

```powershell
mkdir data
```

原因是程序启动时会写 `data/app.log`，后续学生、课程、成绩等 `.dat` 文件也会保存在这个目录下。平时在本项目目录运行一般已经有 `data`，这一步主要是防止把 `edusys.exe` 单独复制到别的目录时启动失败。

### 1.3 默认账号

本教程会用到这三个账号：

```text
admin / admin123
t001  / t001pw
s001  / s001pw
```

注意：管理员新增学生只新增学生资料，不会自动创建该学生的登录账号。所以后面学生端演示仍然使用默认账号 `s001 / s001pw`。

如果演示前发现 `admin / admin123` 登录失败，先不要继续主线。可以尝试是否是上次改密码演示后忘记改回：

```text
Username (empty = quit): admin
Password: admin123demo
```

如果 `admin123demo` 能登录，进入 Admin 主菜单后立刻改回默认密码：

```text
Select: 6
Old password: admin123demo
New password: admin123
```

预期关键输出：

```text
[OK]  Password changed.
```

如果 `admin123` 和 `admin123demo` 都不能登录，就先不要按本文档继续演示，需要先确认当前数据文件里的管理员密码状态，或者恢复到已知的演示数据。

### 1.4 临时编号冲突时怎么处理

本文档默认使用 `S900` 作为临时学生编号，使用 `C900` 作为临时课程编号。演示前如果发现 `S900` 或 `C900` 已经存在，优先先按第 8 节清理旧的临时数据；如果不方便清理，可以改用一组新的编号，例如 `S901` 和 `C901`。

建议演示前先在纸上或文档旁边记一下本次实际使用的编号：

```text
本次临时学生编号：S900
本次临时课程编号：C900
本次临时学期：2025-2026-1
```

如果想确认 `S900` 和 `C900` 是否已经残留，可以先单独启动程序并登录 Admin：

```powershell
.\edusys.exe
```

```text
Username (empty = quit): admin
Password: admin123
```

先查临时学生：

```text
Select: 1
Select: 2
Student id: S900
```

如果不存在，预期会看到：

```text
[ERR] Student not found: S900
```

如果能看到 `Id : S900` 之类的学生详情，说明旧临时学生还在。此时可以继续留在学生管理菜单删除它：

```text
Select: 5
Id to remove: S900
Type 'yes' to confirm cascade delete: yes
```

不管 `S900` 是否存在，查完或删完后都要先返回 Admin 主菜单：

```text
Select: 0
```

再查临时课程：

```text
Select: 2
Select: 2
Course id: C900
```

如果不存在，预期会看到：

```text
[ERR] Course not found: C900
```

如果能看到 `Id : C900` 之类的课程详情，说明旧临时课程还在。此时可以继续留在课程管理菜单删除它：

```text
Select: 5
Course id to remove: C900
Type 'yes' to confirm cascade delete: yes
```

如果只残留了学生，就只删学生；如果只残留了课程，就只删课程。确认完以后回到 Admin 主菜单并退出登录：

```text
Select: 0
Select: 0
```

回到登录界面后，可以直接在用户名处回车退出程序：

```text
Username (empty = quit):
```

如果改用备用编号，要整套替换，不要只改其中一步：

```text
把所有 S900 换成新的学生编号，例如 S901。
把所有 C900 换成新的课程编号，例如 C901。
成绩键里的 S900/C900/2025-2026-1 也要一起换。
Teacher 端 Pick course id from above 输入的课程号也要换。
Admin 导出的 CSV 文件名也会跟着变成 data/course_stats_C901.csv 和 data/ranking_C901.csv。
最后清理时删除的学生编号和课程编号也要换。
```

如果只换了学生编号但成绩录入里还输入旧的 `S900`，会出现 `Student not found`；如果只换了课程编号但 Teacher 端还输入旧的 `C900`，会出现找不到课程或课程不在本人授课列表里的错误。

## 2. 启动 CLI 程序

输入：

```powershell
.\edusys.exe
```

预期会看到类似输出：

```text
==============================================
 EduSys - Student Score Management System
 Week 12 build: interactive menus + stats
 Run mode        : LOADED (persisted)
==============================================

==============================================
 EduSys  -  Login
 Tip: leave username empty to quit program.
==============================================

-- Login --
Username (empty = quit):
```

`Run mode` 可能显示 `LOADED (persisted)`，也可能在第一次运行时显示 `SEEDED (first run)`。这不影响演示。

## 3. 登录功能演示

### 3.1 演示错误密码

在登录界面输入：

```text
Username (empty = quit): admin
Password: wrong
```

预期关键输出：

```text
[ERR] Invalid username or password  (attempt 1/3)
```

说明点：系统会识别错误密码，并记录连续失败次数。连续失败 3 次会退出程序。

### 3.2 演示 Admin 正常登录

继续输入：

```text
Username (empty = quit): admin
Password: admin123
```

预期关键输出：

```text
[OK]  Welcome, admin (role=Admin)
```

随后进入 Admin 主菜单：

```text
==============================================================================
 Admin Menu  [admin]
==============================================================================
 1. Student management
 2. Course  management
 3. Score   management
 4. Statistics
 5. Generate warning report
 6. Change my password
 7. Export course CSV (stats + ranking)
 0. Logout
Select:
```

## 4. Admin 端功能演示

Admin 端覆盖学生管理、课程管理、成绩管理、统计、预警报告、CSV 导出和修改密码。

本节先不演示修改密码，修改密码放到第 7 节统一演示。

### 4.1 学生管理：查看全部学生

在 Admin 主菜单输入：

```text
Select: 1
```

进入学生管理：

```text
==============================================================================
 Admin > Student Management
==============================================================================
 1. List all
 2. View by id
 3. Add
 4. Edit
 5. Remove (cascade scores + user account)
 0. Back
Select:
```

输入：

```text
Select: 1
```

预期关键输出类似：

```text
Id        Name            Class       Year    Contact
-----------------------------------------------------
S001      Zhang San       CS2501      2025    13800000001
S003      Wang Wu         CS2501      2025    13800000003
```

如果数据较多，程序会分页显示，并提示：

```text
Enter = next page, q = quit:
```

现场演示时可以输入 `q` 退出分页，也可以直接回车看下一页。无论哪种方式，都要等重新看到学生管理菜单的 `Select:` 后，再继续下一小节；不要把下一步的菜单编号输入到分页提示里。

### 4.2 学生管理：按学号查看

继续在学生管理菜单输入：

```text
Select: 2
Student id: S001
```

预期关键输出：

```text
Id      : S001
Name    : Zhang San
Major   : Computer Science
Class   : CS2501
Year    : 2025
Contact : 13800000001
```

### 4.3 学生管理：新增临时学生

输入：

```text
Select: 3
Id: S900
Name: Demo Student
Contact: 13900000900
Major: Computer Science
Class: CS-DEMO
Enroll year: 2025
```

预期关键输出：

```text
[OK]  Student created: S900
```

如果提示：

```text
[ERR] Student id already exists: S900
```

说明之前已经演示过但没有清理。可以改用 `S901`，后文涉及 `S900` 的地方同步改成 `S901`。

### 4.4 学生管理：查看刚新增的学生

输入：

```text
Select: 2
Student id: S900
```

预期关键输出：

```text
Id      : S900
Name    : Demo Student
Major   : Computer Science
Class   : CS-DEMO
Year    : 2025
Contact : 13900000900
```

### 4.5 学生管理：编辑学生

输入：

```text
Select: 4
Id to edit: S900
Name [Demo Student]: Demo Student Updated
Contact [13900000900]: 13900000900
Major [Computer Science]: Computer Science
Class [CS-DEMO]: CS-DEMO
Enroll year [2025]: 2025
```

说明：方括号里是旧值。程序也支持直接回车保留旧值，但现场演示时建议像上面这样把每个字段都输入一遍，避免漏按回车导致后续输入错位。

预期关键输出：

```text
[OK]  Student updated: S900
```

再次查看：

```text
Select: 2
Student id: S900
```

预期关键输出：

```text
Name    : Demo Student Updated
```

### 4.6 返回 Admin 主菜单

在学生管理菜单输入：

```text
Select: 0
```

回到 Admin 主菜单。

### 4.7 课程管理：查看全部课程

在 Admin 主菜单输入：

```text
Select: 2
```

进入课程管理：

```text
==============================================================================
 Admin > Course Management
==============================================================================
 1. List all
 2. View by id
 3. Add
 4. Edit
 5. Remove (cascade scores)
 0. Back
Select:
```

输入：

```text
Select: 1
```

预期关键输出类似：

```text
CId     CourseName                    Cred   TeaId   Semester
----------------------------------------------------------------
C001    Advanced Programming (C++)    4.0    T001    2025-2026-1
```

如果课程很多，这里也可能出现分页提示。先输入 `q` 或翻完页，等重新看到课程管理菜单的 `Select:` 后，再继续下一小节。

### 4.8 课程管理：按课程号查看

输入：

```text
Select: 2
Course id: C001
```

预期关键输出：

```text
Id       : C001
Name     : Advanced Programming (C++)
Credit   : 4
Teacher  : T001
Semester : 2025-2026-1
```

### 4.9 课程管理：新增临时课程

输入：

```text
Select: 3
Course id: C900
Course name: Demo Course
Credit: 3
Teacher id: T001
Semester: 2025-2026-1
```

预期关键输出：

```text
[OK]  Course created: C900
```

如果提示：

```text
[ERR] Course id already exists: C900
```

说明之前已经演示过但没有清理。可以改用 `C901`，后文涉及 `C900` 的地方同步改成 `C901`。

### 4.10 课程管理：查看刚新增的课程

输入：

```text
Select: 2
Course id: C900
```

预期关键输出：

```text
Id       : C900
Name     : Demo Course
Credit   : 3
Teacher  : T001
Semester : 2025-2026-1
```

### 4.11 课程管理：编辑课程

输入：

```text
Select: 4
Course id to edit: C900
Name [Demo Course]: Demo Course Updated
Credit [3.0]: 3
TeacherId [T001]: T001
Semester [2025-2026-1]: 2025-2026-1
```

预期关键输出：

```text
[OK]  Course updated: C900
```

再次查看：

```text
Select: 2
Course id: C900
```

预期关键输出：

```text
Name     : Demo Course Updated
```

### 4.12 返回 Admin 主菜单

在课程管理菜单输入：

```text
Select: 0
```

### 4.13 成绩管理：查看全部成绩

在 Admin 主菜单输入：

```text
Select: 3
```

进入成绩管理：

```text
==============================================================================
 Admin > Score Management
==============================================================================
 1. List all
 2. By student
 3. By course
 4. Upsert (add or update)
 5. Remove one score
 0. Back
Select:
```

输入：

```text
Select: 1
```

预期关键输出类似：

```text
StuId   CId     Semester      Usual   Final   Total
----------------------------------------------------
S001    C001    2025-2026-1   88.00   92.00   90.80
```

如果成绩很多，这里也可能出现分页提示。先输入 `q` 或翻完页，等重新看到成绩管理菜单的 `Select:` 后，再继续下一小节。

### 4.14 成绩管理：录入临时成绩

输入：

```text
Select: 4
Student id: S900
Course id: C900
Semester: 2025-2026-1
Usual (0-100): 86
Final (0-100): 90
Total (0-100): 88
```

预期关键输出：

```text
[OK]  Score upserted: S900/C900/2025-2026-1
```

说明点：`Upsert` 表示同一组学生、课程、学期不存在时新增，存在时更新。

### 4.15 成绩管理：按学生查询

输入：

```text
Select: 2
Student id: S900
```

预期关键输出：

```text
CId     Semester      Usual   Final   Total
--------------------------------------------
C900    2025-2026-1   86.00   90.00   88.00
```

### 4.16 成绩管理：按课程查询

输入：

```text
Select: 3
Course id: C900
```

预期关键输出：

```text
StuId   Semester      Usual   Final   Total
--------------------------------------------
S900    2025-2026-1   86.00   90.00   88.00
```

### 4.17 成绩管理：再次 Upsert 演示更新

输入同一组学生、课程、学期，但换一组分数：

```text
Select: 4
Student id: S900
Course id: C900
Semester: 2025-2026-1
Usual (0-100): 89
Final (0-100): 91
Total (0-100): 90
```

预期关键输出：

```text
[OK]  Score upserted: S900/C900/2025-2026-1
```

再次按学生查询：

```text
Select: 2
Student id: S900
```

预期分数已经变为：

```text
C900    2025-2026-1   89.00   91.00   90.00
```

### 4.18 成绩管理：删除单条成绩

此处先演示删除，再重新录入一次，方便后续 Teacher 端继续演示。

输入：

```text
Select: 5
Student id: S900
Course id: C900
Semester: 2025-2026-1
```

预期关键输出：

```text
[OK]  Score removed: S900/C900/2025-2026-1
```

重新录入一次同样成绩：

```text
Select: 4
Student id: S900
Course id: C900
Semester: 2025-2026-1
Usual (0-100): 89
Final (0-100): 91
Total (0-100): 90
```

预期关键输出：

```text
[OK]  Score upserted: S900/C900/2025-2026-1
```

### 4.19 返回 Admin 主菜单

在成绩管理菜单输入：

```text
Select: 0
```

### 4.20 统计分析：课程统计

在 Admin 主菜单输入：

```text
Select: 4
```

进入统计菜单：

```text
==============================================================================
 Admin > Statistics
==============================================================================
 1. Course stats (count/avg/max/min/pass/excellent)
 2. Ranking by course (total score desc)
 3. Student GPA
 0. Back
Select:
```

输入：

```text
Select: 1
Course id: C900
```

预期关键输出：

```text
CourseId    : C900
CourseName  : Demo Course Updated
Count       : 1
Avg         : 90.00
Max         : 90.00
Min         : 90.00
Pass rate   : 100.0%
Excellent   : 100.0%
```

### 4.21 统计分析：课程排名

输入：

```text
Select: 2
Course id: C900
```

预期关键输出：

```text
Rank   StuId     Name            Total
----------------------------------------
1      S900      Demo Student U  90.00
```

姓名列可能因为列宽显示为直接截断形式，程序不会自动加省略号。只要能看到 `S900` 和 `90.00` 即可。

### 4.22 统计分析：学生 GPA

输入：

```text
Select: 3
Student id: S900
```

预期关键输出：

```text
StudentId   : S900
StudentName : Demo Student Updated
Courses     : 1
Total credit: 3.0
GPA         : 4.00 / 4.0
```

### 4.23 返回 Admin 主菜单

在统计菜单输入：

```text
Select: 0
```

### 4.24 生成预警报告

在 Admin 主菜单输入：

```text
Select: 5
```

预期关键输出：

```text
==============================================================================
 Admin > Warning Report
==============================================================================
[OK]  Warning report written to: data/warning_report.txt
```

说明点：此功能会生成文本报告文件 `data/warning_report.txt`。

报告内容会根据当前所有学生的成绩变化。如果你的数据里有低分学生，例如当前仓库常见的 `S004`，预警报告里可能会出现这些学生；这不是错误，说明报告读取的是当前持久化数据。

### 4.25 导出课程 CSV

在 Admin 主菜单输入：

```text
Select: 7
Course id: C900
```

预期关键输出：

```text
[OK]  Course stats CSV : data/course_stats_C900.csv
[OK]  Ranking CSV      : data/ranking_C900.csv
```

说明点：此功能一次导出两个 CSV 文件：课程统计和课程排名。

这两个文件是演示输出文件，会保留在 `data/` 目录下。如果同名文件已经存在，本次导出会覆盖同名 CSV，而不是新建一个带时间戳的新文件。后面第 8 节清理的是 `.dat` 里的临时学生、课程和成绩，不会自动删除已经导出的 `data/course_stats_C900.csv` 和 `data/ranking_C900.csv`。如果演示结束后想让目录更干净，可以手动删除这两个 CSV；`data/warning_report.txt` 是固定报告文件，下次生成预警报告时会被覆盖。

### 4.26 暂时退出 Admin

在 Admin 主菜单输入：

```text
Select: 0
```

预期关键输出：

```text
[OK]  Logged out.
```

程序回到登录界面：

```text
-- Login --
Username (empty = quit):
```

## 5. Teacher 端功能演示

Teacher 端主要演示：查看本人课程、查看本人课程成绩、录入/更新成绩、删除成绩、查看本人课程统计、修改密码。

修改密码放在第 7 节统一演示。

### 5.1 登录 Teacher

输入：

```text
Username (empty = quit): t001
Password: t001pw
```

预期关键输出：

```text
[OK]  Welcome, t001 (role=Teacher)
```

进入 Teacher 菜单：

```text
==============================================================================
 Teacher Menu  [t001 / teacherId=T001]
==============================================================================
 1. List my courses
 2. View scores for my course
 3. Record / update one score
 4. Delete one score
 5. Course statistics (my course only)
 6. Change my password
 0. Logout
Select:
```

### 5.2 查看我的课程

输入：

```text
Select: 1
```

预期关键输出中应包含 `C001`，也应包含刚才由 Admin 新增、教师为 `T001` 的 `C900`：

```text
CId     CourseName                    Cred   Semester
---------------------------------------------------------
C001    Advanced Programming (C++)    4.0    2025-2026-1
C900    Demo Course Updated           3.0    2025-2026-1
```

说明点：Teacher 只能看到自己授课的课程。

如果这里看不到 `C900`，先不要继续 Teacher 端。通常是前面 Admin 新增课程时课程号没建成功，或者 `Teacher id` 没填成 `T001`。可以退出 Teacher，重新登录 Admin，到课程管理里按课程号查看 `C900`，确认 `Teacher : T001` 后再回来。

### 5.3 查看本人课程成绩

输入：

```text
Select: 2
Pick course id from above (empty = cancel): C900
```

预期关键输出：

```text
StuId     Semester      Usual   Final   Total
----------------------------------------------
S900      2025-2026-1   89.00   91.00   90.00
```

### 5.4 教师录入/更新成绩

输入：

```text
Select: 3
Pick course id from above (empty = cancel): C900
Student id: S900
Semester: 2025-2026-1
Usual (0-100): 92
Final (0-100): 94
Total (0-100): 93
```

预期关键输出：

```text
[OK]  Score upserted: S900/C900/2025-2026-1
```

说明点：Teacher 要先从本人课程列表里选择课程。这里体现教师只能操作自己课程的限制。

### 5.5 再次查看本人课程成绩

输入：

```text
Select: 2
Pick course id from above (empty = cancel): C900
```

预期关键输出变为：

```text
S900      2025-2026-1   92.00   94.00   93.00
```

### 5.6 查看本人课程统计

输入：

```text
Select: 5
Pick course id from above (empty = cancel): C900
```

预期关键输出：

```text
CourseId    : C900
CourseName  : Demo Course Updated
Count       : 1
Avg         : 93.00
Max         : 93.00
Min         : 93.00
Pass rate   : 100.0%
Excellent   : 100.0%
```

### 5.7 教师删除一条成绩

为了演示 Teacher 端的删除功能，输入：

```text
Select: 4
Pick course id from above (empty = cancel): C900
Student id: S900
Semester: 2025-2026-1
```

预期关键输出：

```text
[OK]  Score removed: S900/C900/2025-2026-1
```

为了后续清理时还能演示级联删除，马上重新录入一条成绩：

```text
Select: 3
Pick course id from above (empty = cancel): C900
Student id: S900
Semester: 2025-2026-1
Usual (0-100): 92
Final (0-100): 94
Total (0-100): 93
```

预期关键输出：

```text
[OK]  Score upserted: S900/C900/2025-2026-1
```

### 5.8 退出 Teacher

输入：

```text
Select: 0
```

预期关键输出：

```text
[OK]  Logged out.
```

## 6. Student 端功能演示

Student 端主要演示：查看个人资料、查看个人成绩、查看个人 GPA、修改密码。

修改密码放到第 7 节统一演示。

### 6.1 登录 Student

输入：

```text
Username (empty = quit): s001
Password: s001pw
```

预期关键输出：

```text
[OK]  Welcome, s001 (role=Student)
```

进入 Student 菜单：

```text
==============================================================================
 Student Menu  [s001 / studentId=S001]
==============================================================================
 1. View my profile
 2. View my scores
 3. View my GPA
 4. Change my password
 0. Logout
Select:
```

### 6.2 查看我的资料

输入：

```text
Select: 1
```

预期关键输出：

```text
Id      : S001
Name    : Zhang San
Major   : Computer Science
Class   : CS2501
Year    : 2025
Contact : 13800000001
```

### 6.3 查看我的成绩

输入：

```text
Select: 2
```

预期关键输出类似：

```text
CId     Semester      Usual   Final   Total
--------------------------------------------
C001    2025-2026-1   88.00   92.00   90.80
```

说明点：学生端不需要输入学号，系统自动使用当前登录学生 `S001`。

### 6.4 查看我的 GPA

输入：

```text
Select: 3
```

预期关键输出：

```text
StudentId   : S001
StudentName : Zhang San
Courses     : 1
Total credit: 4.0
GPA         : 4.00 / 4.0
```

### 6.5 退出 Student

输入：

```text
Select: 0
```

预期关键输出：

```text
[OK]  Logged out.
```

## 7. 修改密码功能演示

三类角色都有修改密码功能：

```text
Admin   主菜单 6. Change my password
Teacher 主菜单 6. Change my password
Student 主菜单 4. Change my password
```

现场建议只演示一个账号的改密码，并立刻改回，避免后续登录混乱。下面用 `admin` 演示。

如果时间紧，或者担心现场忘记改回密码，可以只口头说明这个菜单项存在，不实际执行。本节不是主线功能必须操作的步骤。只有在你确认自己能马上把密码改回 `admin123` 时，才建议现场实操；如果中途被打断、输错或忘记改回，后面的清理步骤会因为 Admin 登录失败而变麻烦。

### 7.1 登录 Admin

输入：

```text
Username (empty = quit): admin
Password: admin123
```

### 7.2 把 Admin 密码改成临时密码

在 Admin 主菜单输入：

```text
Select: 6
Old password: admin123
New password: admin123demo
```

预期关键输出：

```text
[OK]  Password changed.
```

### 7.3 退出并用新密码登录

输入：

```text
Select: 0
```

回到登录界面后输入：

```text
Username (empty = quit): admin
Password: admin123demo
```

预期关键输出：

```text
[OK]  Welcome, admin (role=Admin)
```

### 7.4 立刻把密码改回原密码

在 Admin 主菜单输入：

```text
Select: 6
Old password: admin123demo
New password: admin123
```

预期关键输出：

```text
[OK]  Password changed.
```

说明点：Teacher 和 Student 的改密码入口逻辑相同。如果老师要求看，也可以分别登录 `t001` 或 `s001`，进入对应菜单的修改密码项，用相同方式改临时密码再改回。本文主线只实际演示 Admin 改密码，避免三个账号都改一遍导致现场状态混乱。

## 8. 清理临时数据

演示结束前，建议删除临时课程和临时学生，让数据恢复到接近演示前的状态。

当前如果还在 Admin 主菜单，可以直接继续。如果不在，重新登录：

```text
Username (empty = quit): admin
Password: admin123
```

### 8.1 删除临时课程 C900

在 Admin 主菜单输入：

```text
Select: 2
Select: 5
Course id to remove: C900
Type 'yes' to confirm cascade delete: yes
```

预期关键输出：

```text
[OK]  Course removed (cascade): C900
```

说明点：删除课程会级联删除该课程关联的成绩。

本教程先删课程，是为了把临时课程和它关联的成绩一起清掉，适合作为收尾清理。这样做以后，再删除 `S900` 时通常已经没有 `S900/C900` 这条成绩可删了。

如果老师专门问“删除学生是否也会级联删除成绩”，可以说明：学生删除同样会级联删除该学生关联的成绩和学生账号，这个规则在 `StudentService` 里实现。若要现场单独证明，可以临时再建一组学生、课程和成绩，然后先删除学生；主线教程为了避免多造数据，没有把这个额外分支放进必走流程。

返回 Admin 主菜单：

```text
Select: 0
```

### 8.2 删除临时学生 S900

在 Admin 主菜单输入：

```text
Select: 1
Select: 5
Id to remove: S900
Type 'yes' to confirm cascade delete: yes
```

预期关键输出：

```text
[OK]  Student removed (cascade): S900
```

说明点：删除学生会级联删除该学生关联的成绩和学生账号。这里 `S900` 本身没有登录账号，但这个规则对已有学生账号同样生效。

返回 Admin 主菜单：

```text
Select: 0
```

### 8.3 演示输出文件怎么处理

第 8.1 和 8.2 清理的是持久化数据里的临时课程、临时学生和关联成绩，不会自动删除演示过程中导出的文件。

演示后可能留下或更新这些文件：

```text
data/warning_report.txt
data/course_stats_C900.csv
data/ranking_C900.csv
data/app.log
```

其中 `data/warning_report.txt` 是固定路径，下次生成预警报告时会被覆盖。`data/app.log` 是运行日志，正常保留即可。两个 `C900` CSV 是这次演示课程的导出结果，如果演示结束后想让目录更干净，可以在退出程序后手动删除：

```powershell
Remove-Item data\course_stats_C900.csv,data\ranking_C900.csv -ErrorAction SilentlyContinue
```

如果本次使用的是备用课程号，例如 `C901`，就把命令里的 `C900` 改成实际课程号。

### 8.4 退出登录和程序

退出登录：

```text
Select: 0
```

退出程序：

```text
Username (empty = quit):
```

在用户名处直接回车，预期输出：

```text
Bye.
```

## 9. 可选补充演示

这些内容不是主线必演示，但如果老师问到，可以现场补充。

### 9.1 空用户名退出

启动程序：

```powershell
.\edusys.exe
```

在登录界面用户名处直接回车：

```text
Username (empty = quit):
```

预期输出：

```text
Bye.
```

### 9.2 连续三次登录失败退出

启动程序后连续输入三次错误密码：

```text
Username (empty = quit): admin
Password: wrong1
Username (empty = quit): admin
Password: wrong2
Username (empty = quit): admin
Password: wrong3
```

预期关键输出：

```text
[ERR] Invalid username or password  (attempt 1/3)
[ERR] Invalid username or password  (attempt 2/3)
[ERR] Invalid username or password  (attempt 3/3)
Too many failed attempts. Exiting.
```

### 9.3 非法输入校验

如果程序已经退出，先重新启动：

```powershell
.\edusys.exe
```

然后登录 Admin：

```text
Username (empty = quit): admin
Password: admin123
```

进入 Admin 主菜单后，再执行下面步骤。

可以用 Admin 尝试新增非法成绩：

```text
Select: 3
Select: 4
Student id: S001
Course id: C001
Semester: 2025-2026-1
Usual (0-100): 101
Final (0-100): 90
Total (0-100): 95
```

预期关键输出：

```text
[ERR] usualScore out of range [0,100]: 101
```

说明点：成绩范围由服务层校验，不是只靠界面提示。

如果不继续演示成绩管理，可以输入：

```text
Select: 0
```

回到 Admin 主菜单。

### 9.4 Teacher 课程白名单校验

如果当前还在 Admin 主菜单，先退出登录：

```text
Select: 0
```

如果程序已经退出，先重新启动：

```powershell
.\edusys.exe
```

然后登录 Teacher：

```text
Username (empty = quit): t001
Password: t001pw
```

进入 Teacher 主菜单后，再执行下面步骤。

Teacher 菜单在查看成绩、录入成绩、删除成绩、课程统计时，都会先显示本人课程列表。这里可以用“查看成绩”入口演示：

```text
Select: 2
```

随后会出现课程选择提示：

```text
Pick course id from above (empty = cancel):
```

输入一个不在本人课程列表里的课程号，例如：

```text
Pick course id from above (empty = cancel): C999
```

预期关键输出：

```text
[ERR] Course id is not in your teaching list: C999
```

说明点：教师端先在菜单层限制课程选择，服务层也会继续做权限校验。

如果不继续演示 Teacher，可以输入：

```text
Select: 0
```

回到登录界面。

### 9.5 未知菜单项处理

如果当前在登录界面，直接输入 `admin / admin123` 登录。如果程序已经退出，先重新启动并登录 Admin：

```powershell
.\edusys.exe
```

```text
Username (empty = quit): admin
Password: admin123
```

在 Admin 主菜单随便输入一个不存在的选项，例如：

```text
Select: 99
```

预期关键输出：

```text
[ERR] Unknown choice
```

程序不会退出，也不会修改数据，会继续停留在当前菜单。

### 9.6 删除确认取消

这个演示用于说明删除操作不是输完编号就直接执行，必须输入 `yes` 才会真的删除。为了安全，可以拿样例课程 `C001` 做取消演示，但确认处一定不要输入 `yes`。

如果当前还在 Admin 主菜单，可以直接继续。如果当前在登录界面，输入 `admin / admin123` 登录；如果程序已经退出，先重新启动并登录 Admin。然后输入：

```text
Select: 2
Select: 5
Course id to remove: C001
Type 'yes' to confirm cascade delete: no
```

预期关键输出：

```text
[ERR] Aborted
```

这里不会删除 `C001`。演示完仍在课程管理菜单，可以输入：

```text
Select: 0
```

回到 Admin 主菜单。

### 9.7 修改密码失败校验

如果老师问“旧密码输错会不会也能改”，可以只演示失败路径。先确认自己在 Admin 主菜单；如果当前在登录界面，输入 `admin / admin123` 登录。然后输入：

```text
Select: 6
Old password: wrong
New password: admin123demo
```

预期关键输出：

```text
[ERR] Old password is incorrect
```

旧密码错误时不会修改密码，后面仍然使用 `admin / admin123` 登录。

如果想演示新密码不能为空，可以输入：

```text
Select: 6
Old password: admin123
New password:
```

预期关键输出：

```text
[ERR] New password must not be empty
```

这里也不会修改密码。

### 9.8 未知命令行参数

这个演示不进入交互菜单，只验证程序入口会拒绝未知参数。先确保当前交互程序已经退出。

如果当前还在 Admin 主菜单，可以先退出登录：

```text
Select: 0
```

回到登录界面后，在用户名处直接回车退出程序：

```text
Username (empty = quit):
```

回到 PowerShell 后输入：

```powershell
.\edusys.exe --bad-arg
```

预期关键输出：

```text
Unknown argument: --bad-arg  (supported: --self-test)
```

这说明当前 CLI 只支持无参数交互模式和 `--self-test` 自检模式。

### 9.9 命令行自检入口

虽然课堂演示主线不依赖自动脚本，但如果老师问“怎么证明边界测试跑过”，可以展示。运行前同样要先确认当前交互程序已经退出，回到 PowerShell 后再输入：

```powershell
.\edusys.exe --self-test
```

预期关键输出：

```text
Week 11         : self-check PASSED
Week 13         : boundary-check PASSED
```

这不是交互菜单功能，而是回归验证入口。

注意：只建议在确认默认 Admin 密码已经恢复为 `admin123`、并且当前数据已经是平时自检通过后的持久化状态时运行。`--self-test` 会写运行日志；如果 `data/*.dat` 处于全空的第一次运行状态，它还会执行 Week 11 的初始化演化流程，例如创建 `S003`、更新 `S001` 成绩、删除 `S002`。如果当前数据不是这个项目平时的演示数据，也可能因为持久化状态不符合自检预期而失败。现场如果不确定数据状态，可以只说明这是回归验证入口，不必当场运行。

## 10. CLI 功能覆盖清单

### 10.1 程序入口

已覆盖：

```text
无参数启动交互式 CLI
空用户名退出
错误密码提示
连续 3 次失败退出
未知命令行参数拒绝
--self-test 回归验证入口
```

### 10.2 Admin

已覆盖：

```text
学生列表
按学号查看学生
新增学生
编辑学生
删除学生，级联删除成绩和学生账号

课程列表
按课程号查看课程
新增课程
编辑课程
删除课程，级联删除成绩

成绩列表
按学生查询成绩
按课程查询成绩
录入或更新成绩
删除单条成绩

课程统计
课程排名
学生 GPA
生成 warning_report.txt
导出 course_stats_<courseId>.csv
导出 ranking_<courseId>.csv
修改密码
未知菜单项提示
删除确认取消
改密旧密码错误拒绝
改密新密码为空拒绝
退出登录
```

### 10.3 Teacher

已覆盖：

```text
查看我的课程
查看我的课程成绩
录入或更新我课程的成绩
删除我课程的一条成绩
查看我课程的统计
修改密码入口已说明；主线未逐项实操 Teacher 改密
退出登录
```

### 10.4 Student

已覆盖：

```text
查看我的资料
查看我的成绩
查看我的 GPA
修改密码入口已说明；主线未逐项实操 Student 改密
退出登录
```

### 10.5 文件输出

已覆盖：

```text
data/warning_report.txt
data/course_stats_C900.csv
data/ranking_C900.csv
```

其中 `C900` 是本文档中的临时课程号。实际文件名会随输入的课程号变化。
