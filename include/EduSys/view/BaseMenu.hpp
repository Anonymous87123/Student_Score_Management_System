#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace EduSys {

// 视图层基类：唯一允许 std::cin / std::cout 的地方。
// 所有派生菜单（Admin/Teacher/Student）共享这里的输入采集 / 表格打印 / 分页工具，
// 派生菜单本身不再各自调原始 IO，避免 View 边界被悄悄破。
class BaseMenu {
public:
    virtual ~BaseMenu() = default;
    virtual void run() = 0;

protected:
    // ---- 输入采集 ----
    // 读一行（去末尾 \r\n）；EOF 视作 "0"，让所有循环自然退出。
    static std::string readLine(const std::string& prompt);
    // 读非负整数；解析失败返回 -1（菜单循环把 -1 当作非法选择）。
    static int         readInt(const std::string& prompt);
    static double      readDouble(const std::string& prompt);

    // ---- 输出与控制 ----
    static void pause();
    static void printHeading(const std::string& title);
    static void printRule(std::size_t width = 78, char ch = '-');

    // 行 = 字符串列；headers/widths/rows[i] 三者长度必须一致，否则只按 widths 截断。
    // 列宽中"负值"语义已不暴露 —— widths 全部按 std::size_t 处理；left-align by default。
    static void printTable(const std::vector<std::string>&              headers,
                           const std::vector<std::vector<std::string>>& rows,
                           const std::vector<std::size_t>&              widths);

    // 简单分页：每 pageSize 行画一次表，提示 Enter 翻页 / q 跳出。
    static void paginate(const std::vector<std::string>&              headers,
                         const std::vector<std::vector<std::string>>& rows,
                         const std::vector<std::size_t>&              widths,
                         std::size_t                                  pageSize = 10);

    // 业务异常一律丢给派生菜单 catch，再走这里转成"红色提示行"。
    static void printError(const std::string& msg);
    static void printOk(const std::string& msg);
};

} // namespace EduSys
