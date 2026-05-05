#include "EduSys/view/BaseMenu.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace EduSys {

namespace {

std::string trimCR(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
    return s;
}

std::string clipPad(const std::string& src, std::size_t width) {
    if (src.size() >= width) {
        return src.substr(0, width);
    }
    return src + std::string(width - src.size(), ' ');
}

} // namespace

std::string BaseMenu::readLine(const std::string& prompt) {
    if (!prompt.empty()) {
        std::cout << prompt;
        std::cout.flush();
    }
    std::string line;
    if (!std::getline(std::cin, line)) {
        // EOF 或流坏 —— 返回 "0" 让上层循环按"返回 / 退出"分支自然走完。
        std::cin.clear();
        return "0";
    }
    return trimCR(std::move(line));
}

int BaseMenu::readInt(const std::string& prompt) {
    const std::string s = readLine(prompt);
    try {
        std::size_t pos = 0;
        const int v = std::stoi(s, &pos);
        if (pos != s.size()) return -1;
        return v;
    } catch (...) {
        return -1;
    }
}

double BaseMenu::readDouble(const std::string& prompt) {
    const std::string s = readLine(prompt);
    try {
        std::size_t pos = 0;
        const double v = std::stod(s, &pos);
        if (pos != s.size()) return -1.0;
        return v;
    } catch (...) {
        return -1.0;
    }
}

void BaseMenu::pause() {
    std::cout << "(press Enter to continue)";
    std::cout.flush();
    std::string sink;
    std::getline(std::cin, sink);
}

void BaseMenu::printHeading(const std::string& title) {
    std::cout << "\n";
    printRule(78, '=');
    std::cout << " " << title << "\n";
    printRule(78, '=');
}

void BaseMenu::printRule(std::size_t width, char ch) {
    std::cout << std::string(width, ch) << "\n";
}

void BaseMenu::printTable(const std::vector<std::string>&              headers,
                          const std::vector<std::vector<std::string>>& rows,
                          const std::vector<std::size_t>&              widths) {
    if (headers.empty() || widths.size() != headers.size()) return;

    std::ostringstream line;
    for (std::size_t i = 0; i < headers.size(); ++i) {
        line << clipPad(headers[i], widths[i]);
        if (i + 1 < headers.size()) line << "  ";
    }
    std::cout << line.str() << "\n";

    std::size_t total = 0;
    for (auto w : widths) total += w;
    total += (headers.size() > 0 ? (headers.size() - 1) * 2 : 0);
    printRule(total, '-');

    for (const auto& row : rows) {
        std::ostringstream rl;
        for (std::size_t i = 0; i < headers.size(); ++i) {
            const std::string cell = (i < row.size()) ? row[i] : std::string{};
            rl << clipPad(cell, widths[i]);
            if (i + 1 < headers.size()) rl << "  ";
        }
        std::cout << rl.str() << "\n";
    }
}

void BaseMenu::paginate(const std::vector<std::string>&              headers,
                        const std::vector<std::vector<std::string>>& rows,
                        const std::vector<std::size_t>&              widths,
                        std::size_t                                  pageSize) {
    if (rows.empty()) {
        printTable(headers, rows, widths);
        std::cout << "(no data)\n";
        return;
    }
    if (pageSize == 0) pageSize = 10;

    const std::size_t totalPages = (rows.size() + pageSize - 1) / pageSize;
    for (std::size_t page = 0; page < totalPages; ++page) {
        const std::size_t begin = page * pageSize;
        const std::size_t end   = std::min(begin + pageSize, rows.size());
        std::vector<std::vector<std::string>> slice(rows.begin() + begin, rows.begin() + end);
        printTable(headers, slice, widths);
        std::cout << "Page " << (page + 1) << "/" << totalPages
                  << "  rows " << (begin + 1) << "-" << end << "/" << rows.size() << "\n";
        if (page + 1 < totalPages) {
            const std::string s = readLine("Enter = next page, q = quit: ");
            if (s == "q" || s == "Q") return;
        }
    }
}

void BaseMenu::printError(const std::string& msg) {
    std::cout << "[ERR] " << msg << "\n";
}

void BaseMenu::printOk(const std::string& msg) {
    std::cout << "[OK]  " << msg << "\n";
}

} // namespace EduSys
