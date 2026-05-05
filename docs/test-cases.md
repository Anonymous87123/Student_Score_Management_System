# EduSys 第 13 周 集成测试与边界测试

> 一次性汇总 A-F 六组用例的输入 / 预期 / 实际 / 判定。
> A-E 由 `edusys.exe --self-test` 执行；F 由 `tools/corrupt_check.bat` 执行。
> 整篇按 `claude.md §2.6` 要求落盘，便于答辩对照。

## 0. 运行方式与产物

```bash
# 步骤 1：构建
build.bat

# 步骤 2：A-E 组（exe 内自检，不动 .dat）
edusys.exe --self-test

# 步骤 3：F 组（外部脚本：备份 → 篡改 → 跑 exe → 还原）
tools\corrupt_check.bat
```

**判定列约定**：`PASS` = 实际异常 / 退出码与预期一致；`FAIL` 则被对应自检立即抛出，外层捕获后退出码 1。

---

## A. 认证 / 会话（Auth & Session）

| 用例 ID | 输入 | 预期异常 / 行为 | 实际 | 判定 |
| --- | --- | --- | --- | --- |
| A1 | `authenticate("no_such_user", "x")` | `AuthException("Invalid username or password")` + WARN 日志 | 抛 `AuthException`，写 `WARN Auth failed: unknown user 'no_such_user'` | PASS |
| A2 | `authenticate("admin", "wrong-password")` | `AuthException` + WARN 日志 | 抛 `AuthException`，写 `WARN Auth failed: bad password for 'admin'` | PASS |
| A3 | `changePassword(emptySession, "x", "y")`（未登录） | `AuthException("Not logged in")` | 抛 `AuthException` | PASS |
| A4 | `changePassword(adminSession, kSeedAdminPlain, "")` | `ValidationException("New password must not be empty")` | 抛 `ValidationException` | PASS |
| A5 | `studentSvc.listAll(emptySession)` | `AuthException` | 抛 `AuthException` | PASS |

## B. 字段校验（Field Validation）

> 全部走 Service 入口；Service 在 `saveAll` 之前抛异常，因此对 `.dat` 完全无副作用。

| 用例 ID | 输入 | 预期异常 | 实际 | 判定 |
| --- | --- | --- | --- | --- |
| B1 | `Score(usual=101, ...)` upsert by admin | `ValidationException("usualScore out of range [0,100]: 101")` | 抛 `ValidationException` | PASS |
| B2 | `Score(final=-1, ...)` upsert by admin | `ValidationException("finalScore out of range")` | 抛 `ValidationException` | PASS |
| B3 | `Score(studentId="", ...)` upsert | `ValidationException("Score key fields ... must not be empty")` | 抛 `ValidationException` | PASS |
| B4 | `Student(id="", ...)` create by admin | `ValidationException("Student id must not be empty")` | 抛 `ValidationException` | PASS |
| B5 | `Student(enrollYear=0, ...)` create by admin | `ValidationException("Student enrollYear must be positive")` | 抛 `ValidationException` | PASS |
| B6 | `Course(courseId="", ...)` create by admin | `ValidationException("Course id must not be empty")` | 抛 `ValidationException` | PASS |
| B7 | `Course(credit=-3, ...)` create by admin | `ValidationException("Course credit must be positive")` | 抛 `ValidationException` | PASS |
| B8 | `Course(teacherId="T999", ...)` create by admin（T999 不存在） | `ValidationException("Course teacherId not found: T999")` | 抛 `ValidationException` | PASS |

## C. 唯一性（Uniqueness）

| 用例 ID | 输入 | 预期异常 | 实际 | 判定 |
| --- | --- | --- | --- | --- |
| C1 | `studentSvc.create(admin, Student("S001", ...))`（S001 已存在） | `ValidationException("Student id already exists: S001")` | 抛 `ValidationException` | PASS |
| C2 | `courseSvc.create(admin, Course("C001", ...))`（C001 已存在） | `ValidationException("Course id already exists: C001")` | 抛 `ValidationException` | PASS |

> 备注：Score 的同键 `(studentId, courseId, semester)` 业务语义是 **upsert（覆写）**，不是拒绝；故不在 C 组列单条用例。Week 11 mutation block 中的 `S001/C001/2025-2026-1` 写两次后文件仍为单条，已隐含验证唯一性约束。

## D. 级联删除残留检查（Cascade Residue）

> 验证 Week 11 mutation block 的最终状态：S002 在 `students.dat` / `users.dat` / `scores.dat` 三处都不应残留任何引用。

| 用例 ID | 检查位置 | 预期 | 实际 | 判定 |
| --- | --- | --- | --- | --- |
| D1 | `students.dat` 中是否有 `id=="S002"` | 不存在 | 不存在 | PASS |
| D2 | `scores.dat` 中是否有 `studentId=="S002"` | 不存在 | 不存在 | PASS |
| D3 | `users.dat` 中是否有 `role==Student && ownerId=="S002"` | 不存在 | 不存在 | PASS |

## E. 教师白名单双重防线（Teacher Whitelist）

> Service 层用 `course.teacherId == session.ownerId` 当硬拒绝，View 层另用 `pickOwnCourseId()` 做白名单。E 组直接打 Service 层验证后端硬防线，**绕开 View 也拦得住**。

| 用例 ID | 输入 | 预期异常 / 行为 | 实际 | 判定 |
| --- | --- | --- | --- | --- |
| E1 | ghost 教师 `(role=Teacher, ownerId=T999)` upsert C001 成绩 | `PermissionException("Teacher can only write scores of own courses")` | 抛 `PermissionException` | PASS |
| E2 | ghost 教师 findByCourse C001 | `PermissionException("Teacher can only read scores of own courses")` | 抛 `PermissionException` | PASS |
| E3 | t001 教师 `listAll()` 返回结果 | 全部记录的课程 teacherId 必须等于 T001 | n=3 条，全部满足 | PASS |
| E4 | s001 学生 `findByStudent("S004")` | `PermissionException("Student can only read own scores")` | 抛 `PermissionException` | PASS |

## F. 文件层异常（File Corruption，外置脚本）

> 由 `tools/corrupt_check.bat` 跑：先备份 `data/*.dat` → 篡改 → 跑 `edusys.exe --self-test` 收 rc 与 stderr → 还原。结束后再跑一次 `--self-test` 做 sanity，证明还原干净。

| 用例 ID | 篡改方式 | 预期 rc / 标记 | 实际 rc | 实际 stderr 关键行 | 判定 |
| --- | --- | --- | --- | --- | --- |
| F1 | `del data\scores.dat` | rc=1 + `score for S003` | 1 | `Fatal: Self-check: score for S003 should exist` | PASS |
| F2 | `users.dat` 头 4 字节 `EDSY` → `XXXX` | rc=1 + `Bad magic` | 1 | `Fatal: Bad magic in data/users.dat` | PASS |
| F3 | `scores.dat` count 字段（offset 8..11）→ `0xFF 00 00 00` | rc=1 + `Fatal:` | 1 | `Fatal: Unexpected EOF reading: data/scores.dat` | PASS |
| F-sanity | F1-F3 还原后再跑 `--self-test` | rc=0 全绿 | 0 | （无 Fatal） | PASS |

## 汇总

| 组 | 用例数 | PASS | FAIL | 入口 |
| --- | --- | --- | --- | --- |
| A 认证 / 会话 | 5 | 5 | 0 | `--self-test` |
| B 字段校验 | 8 | 8 | 0 | `--self-test` |
| C 唯一性 | 2 | 2 | 0 | `--self-test` |
| D 级联残留 | 3 | 3 | 0 | `--self-test` |
| E 教师白名单 | 4 | 4 | 0 | `--self-test` |
| F 文件损坏 | 3 + sanity | 3 + sanity | 0 | `corrupt_check.bat` |
| **合计** | **25 + sanity** | **25 + sanity** | **0** | — |

---

## 关键日志证据

以下片段截自 `data/app.log`，时间戳与本轮自检 / corrupt_check 一一对应。

**Week 13 A 组（认证拒绝写日志）**

```text
[2026-05-05 15:09:24] [WARN] Auth failed: unknown user 'no_such_user'
[2026-05-05 15:09:24] [WARN] Auth failed: bad password for 'admin'
[2026-05-05 15:09:24] [INFO] Week 13 boundary check PASSED.
```

**F1（scores.dat 缺失）**

```text
==============================================
 EduSys - Week 11 + Week 13 Self-Test (--self-test)
 Run mode        : LOADED (persisted)
 [Week11] AUTH OK            : admin (role=Admin)
 [Week11] AUTH FAIL expected : Invalid username or password
 [Week11] PERM REJECT ok     : student cannot create student
 [Week11] RANGE REJECT ok    : usual=999 rejected
 [Week11] MUTATIONS skipped  : LOADED run, verifying persisted state instead
Fatal: Self-check: score for S003 should exist
```

```text
[2026-05-05 15:09:23] [ERROR] Fatal: Self-check: score for S003 should exist
```

**F2（users.dat 头被改成 `XXXX`）**

```text
==============================================
 EduSys - Week 11 + Week 13 Self-Test (--self-test)
 Run mode        : LOADED (persisted)
Fatal: Bad magic in data/users.dat
```

```text
[2026-05-05 15:09:24] [ERROR] Fatal: Bad magic in data/users.dat
```

> 注意：F2 在 Week 11 的第一步 `authSvc.authenticate("admin", ...)` 即触发 `userRepo.loadAll()` → `BinaryRepository::checkHeader` → `StorageException`。因此后续 Week 11 / Week 13 断言全部不会执行；这正是文件层异常应有的"早抛早死"语义。

**F3（scores.dat count 虚报为 255）**

```text
 [Week11] MUTATIONS skipped  : LOADED run, verifying persisted state instead
Fatal: Unexpected EOF reading: data/scores.dat
```

```text
[2026-05-05 15:09:24] [ERROR] Fatal: Unexpected EOF reading: data/scores.dat
```

**F-sanity（还原后回归绿）**

```text
[2026-05-05 15:09:24] [INFO] Week 13 boundary check PASSED.
```

---

## 设计取舍备注

- **为什么 F 组外置而不放进 `--self-test`**：用户在 Week 13 选项里明确"exe 内部不能自己改 `.dat`"。文件损坏属于 Storage 层异常，必须由外部干净地造、干净地还原；exe 只在加载时被动撞上、抛异常、退出 1。这条边界跟 Week 11 "MUTATIONS 仅 SEEDED 一次跑一次" 同一逻辑：尽量让自检本身保持只读 / 幂等。
- **C 组只有 2 条**：因为 Score 的同键语义是 upsert，不属于"重复拒绝"。如果硬要拒绝，会和 Week 11 mutation block 中"S001/C001 update"路径自相矛盾。
- **E1/E2 用 ghost 教师 (T999)**：seed 数据里只有 T001 一位老师，要构造"非自己授课的课程"只能伪造 ownerId。这种构造方式在 View 层不可达（菜单只允许从已登录账号填 ownerId），但能直接打到 Service 层后端硬防线，正好验证"绕开 View 也拦得住"。
- **F-sanity 是必跑环节**：corrupt_check.bat 必须证明它的备份还原逻辑没漏文件、没改字节，否则后续所有功能都站不住。所以脚本结尾必跑 `--self-test`，rc 不为 0 直接标失败。
