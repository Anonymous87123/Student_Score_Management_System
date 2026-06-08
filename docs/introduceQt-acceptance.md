# `introduceQt` 分支验收记录

## 1. 范围说明

本文只记录 `introduceQt` 分支当前这套 Qt Widgets GUI 的验收状态，不替代 [`claude.md`](../claude.md) §8.6 的计划清单。计划负责定义"应该验什么"，本文负责说明"现在已经验到哪一步"。

## 2. 自动化回归

截至当前工作树，本分支继续沿用 CLI 版本的自动化回归链路作为核心业务基线：

- `build-qt-introduceQt` 可继续构建 `edusys`
- [`tools/corrupt_check.bat`](../tools/corrupt_check.bat) 通过
- `corrupt_check.bat` 恢复数据后再次触发的 `--self-test` 通过

这样做的含义是：Qt GUI 已经接入，但 `model / storage / service / report` 的正确性仍优先由 CLI 自检和损坏恢复脚本兜底。

## 3. 已完成的代码接入

以下 GUI 结构已经落成真实代码，而不是 placeholder：

- 入口与登录：[`src/app/gui_main.cpp`](../src/app/gui_main.cpp)、[`src/gui/LoginDialog.cpp`](../src/gui/LoginDialog.cpp)
- 管理员主窗口：[`src/gui/AdminWindow.cpp`](../src/gui/AdminWindow.cpp)
- 教师主窗口：[`src/gui/TeacherWindow.cpp`](../src/gui/TeacherWindow.cpp)
- 学生主窗口：[`src/gui/StudentWindow.cpp`](../src/gui/StudentWindow.cpp)
- 编辑与改密对话框：
  [`src/gui/StudentEditDialog.cpp`](../src/gui/StudentEditDialog.cpp)、
  [`src/gui/CourseEditDialog.cpp`](../src/gui/CourseEditDialog.cpp)、
  [`src/gui/ScoreEditDialog.cpp`](../src/gui/ScoreEditDialog.cpp)、
  [`src/gui/ChangePasswordDialog.cpp`](../src/gui/ChangePasswordDialog.cpp)

对应功能覆盖如下：

- Admin：学生 / 课程 / 成绩 CRUD，课程统计，课程排名，学生 GPA，预警报告导出，CSV 导出，修改密码，退出登录
- Teacher：本人课程列表，本人课程成绩 CRUD，本人课程统计 / 排名，修改密码，退出登录
- Student：本人资料，本人成绩，本人 GPA，修改密码，退出登录

## 4. 权限边界现状

当前权限边界已经形成"GUI 白名单 + Service 硬拒绝"双层保护：

- GUI 层不向教师暴露他人课程入口
- [`ScoreService`](../src/service/ScoreService.cpp) 对教师越权读写成绩继续拒绝
- [`StatsService`](../src/service/StatsService.cpp) 对教师越权统计继续拒绝
- [`CourseService`](../src/service/CourseService.cpp) 当前也已补齐教师只读本人课程的限制

这意味着即使将来某个 GUI 过滤点漏判，后端业务层仍会拒绝越权操作。

## 5. 仍需手工勾验的项目

以下项目目前仍应视为"待手工验收"，不能仅凭代码存在就判定完工：

- 登录成功、错密码、连续 3 次失败退出、取消退出
- Admin / Teacher / Student 三种角色实际点击链路是否完整可用
- 登出后是否稳定回到登录框
- 重启 GUI 后是否从原有 `.dat` 正确加载
- GUI 改写数据后，CLI 读到的结果是否一致
- GUI 导出的 `.txt / .csv` 与 CLI 导出路径和内容语义是否一致
- GUI 中文文案在真实运行窗口里是否全部正常显示

## 6. 结论

`introduceQt` 当前更接近"功能主体已落地，正在做验收与收尾"，而不是"只完成了原型"。后续工作的重点不再是补大块界面骨架，而是：

- 把手工验收项逐条跑完
- 修正文案 / 编码显示等收尾问题
- 同步文档，使仓库对 Qt 分支状态的描述与现实一致
