#include <bits/stdc++.h>
using namespace std;
// 去除逗号 + 空格 + 前导零 + 提取符号
pair<string, int> normalize(string s) {
    s.erase(remove_if(s.begin(), s.end(), [](char c){ return c == ' ' || c == '\t' || c == ','; }), s.end());
    int sign = 1;
    if (!s.empty() && s[0] == '-') {
        sign = -1;
        s = s.substr(1);
    }
    size_t start = s.find_first_not_of('0');
    s = (start == string::npos) ? "0" : s.substr(start);
    return {s, sign};
}
// 比较两个非负字符串大小
int compare(const string& a, const string& b) {
    if (a.size() != b.size()) return a.size() > b.size() ? 1 : -1;
    return a > b ? 1 : (a < b ? -1 : 0);
}
 
// 非负加法
string add(const string& a, const string& b) {
    string res;
    int i = a.size() - 1, j = b.size() - 1, carry = 0;
    while (i >= 0 || j >= 0 || carry) {
        int sum = carry;
        if (i >= 0) sum += a[i--] - '0';
        if (j >= 0) sum += b[j--] - '0';
        res.push_back(sum % 10 + '0');
        carry = sum / 10;
    }
    reverse(res.begin(), res.end());
    return res;
}
 
// 非负减法 (a >= b)
string subtract(const string& a, const string& b) {
    string res;
    int i = a.size() - 1, j = b.size() - 1, borrow = 0;
    while (i >= 0 || j >= 0 || borrow) {
        int diff = borrow;
        if (i >= 0) diff += a[i--] - '0';
        if (j >= 0) diff -= b[j--] - '0';
        if (diff < 0) {
            diff += 10;
            borrow = -1;
        } else borrow = 0;
        res.push_back(diff + '0');
    }
    reverse(res.begin(), res.end());
    size_t start = res.find_first_not_of('0');
    return (start == string::npos) ? "0" : res.substr(start);
}
 
string multiply(const string& a, const string& b) {
    int n = a.size(), m = b.size();
    vector<int> res(n + m, 0);
    for (int i = n - 1; i >= 0; --i) {
        for (int j = m - 1; j >= 0; --j) {
            int prod = (a[i] - '0') * (b[j] - '0');
            int sum = prod + res[i + j + 1];
            res[i + j + 1] = sum % 10;
            res[i + j] += sum / 10;
        }
    }
    string result;
    bool leading = true;
    for (int digit : res) {
        if (leading && digit == 0) continue;
        leading = false;
        result += '0' + digit;
    }
    return result.empty() ? "0" : result;
}
 
// 小整数乘大数 (O(n))
string multiply_small(const string& a, int b) {
    if (b == 0) return "0";
    string res;
    int carry = 0;
    for (int i = a.size() - 1; i >= 0; --i) {
        int prod = (a[i] - '0') * b + carry;
        res = char(prod % 10 + '0') + res;
        carry = prod / 10;
    }
    if (carry) res = to_string(carry) + res;
    return res;
}
 
// 除你大坝
pair<string, string> divide_and_remainder(const string& num, const string& div) {
    if (div == "0") return {"", ""}; 
    if (num == "0") return {"0", "0"};
    string quotient = "";
    string remainder = "";
    for (char c : num) {
        remainder += c;
        size_t start = remainder.find_first_not_of('0');
        remainder = (start == string::npos) ? "0" : remainder.substr(start);
        if (compare(remainder, div) < 0) {
            if (!quotient.empty()) quotient += '0';
            continue;
        }
        // 二分找，麻烦死
        int low = 0, high = 9, best = 0;
        while (low <= high) {
            int mid = (low + high) / 2;
            string prod = multiply_small(div, mid);
            if (compare(prod, remainder) <= 0) {
                best = mid;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        quotient += '0' + best;
        string sub_val = multiply_small(div, best);
        remainder = subtract(remainder, sub_val);
    }
    size_t start = quotient.find_first_not_of('0');
    quotient = (start == string::npos) ? "0" : quotient.substr(start);
    return {quotient, remainder};
}
 
//先格
string format_integer(string num) {
    if (num == "0") return "0";
    string res;
    int cnt = 0;
    for (int i = num.size() - 1; i >= 0; --i) {
        if (cnt == 3) {
            res += ',';
            cnt = 0;
        }
        res += num[i];
        cnt++;
    }
    reverse(res.begin(), res.end());
    return res;
}
 
//你他妈还得写小数点后的
string compute_decimal(string rem, const string& div, int& carry_to_int) {
    string decimal = "";
    carry_to_int = 0;
    for (int i = 0; i < 4; ++i) { // 多一位用于四舍五入
        if (rem == "0") {
            decimal += '0';
            continue;
        }
        rem += '0';
        auto [q, r] = divide_and_remainder(rem, div);
        decimal += (q.empty() ? "0" : q);
        rem = r;
    }
    while (decimal.size() < 4) decimal += '0';
    // 四舍五入
    if (decimal[3] >= '5') {
        int carry = 1;
        for (int j = 2; j >= 0; --j) {
            int d = (decimal[j] - '0') + carry;
            decimal[j] = (d % 10) + '0';
            carry = d / 10;
        }
        carry_to_int = carry;
    }
    return decimal.substr(0, 3);
}
int main() {
    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;
        size_t pos = line.find_first_of("*/", 1);
        if (pos == string::npos) continue;
        char op = line[pos];
        string left = line.substr(0, pos);
        string right = line.substr(pos + 1);
        auto [a, sa] = normalize(left);
        auto [b, sb] = normalize(right);
        int sign = sa * sb;
        string result_int, result_dec = "000";
        if (op == '*') {
            result_int = multiply(a, b);
        } else {
            if (b == "0") {
                cout << "Error: division by zero" << endl;
                continue;
            }
            auto [q, r] = divide_and_remainder(a, b);
            result_int = q;
            int carry = 0;
            result_dec = compute_decimal(r, b, carry);
            if (carry) {
                result_int = add(result_int, "1");
            }
        }
        string output = format_integer(result_int);
        if (op == '/') {
            output += "." + result_dec;
        }
        if (sign < 0 && (result_int != "0" || (op == '/' && result_dec != "000"))) {
            output = "-" + output;
        }
        cout << output << endl;
    }
    return 0;
}