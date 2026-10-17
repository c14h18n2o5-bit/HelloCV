/**
 * @file main.cpp
 * @brief 单位转换器主程序：交互式菜单 + 输入校验 + 调用 unitlib 完成换算。
 *
 * 程序结构：
 *   1. 启动时先修正 Windows 控制台编码，保证中文提示正常显示；
 *   2. 主菜单循环读取用户选择，分别进入温度 / 长度 / 质量三个子菜单；
 *   3. 所有“读取输入”的逻辑都做了校验与容错：输入字母、超出范围、
 *      直接按 Ctrl+Z（EOF）都不会让程序死循环或崩溃；
 *   4. 真正的换算公式全部交给 unitlib 静态库完成，主程序只负责界面。
 */

#include "unitlib.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

// Windows 控制台默认使用 GBK 等本地编码，直接输出 UTF-8 中文会乱码，
// 因此启动时把输入/输出代码页切换成 UTF-8（65001）。
// NOMINMAX 用来阻止 <windows.h> 定义 min/max 宏，否则会与标准库的
// std::numeric_limits<T>::max() 之类的写法冲突。
#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

// 这两个宏由 CMake 的 target_compile_definitions() 传入；
// 若没有用 CMake 构建，则回退为友好提示，保证仍可编译。
#ifndef APP_VERSION
#  define APP_VERSION "dev"
#endif
#ifndef APP_GIT_HASH
#  define APP_GIT_HASH "unknown"
#endif

namespace {

constexpr int kExitChoice = 0;

/// 把 Windows 控制台的输入/输出代码页切换为 UTF-8；其他平台什么都不做。
void enableUtf8Console() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

/// 用 8 位有效数字格式化结果，去掉多余的 0（212.000000 显示为 212）。
std::string formatNumber(double value) {
    std::ostringstream out;
    out << std::setprecision(8) << std::defaultfloat << value;
    return out.str();
}

/// 丢弃当前输入行剩下的字符（包括换行符），避免非法输入影响下一次读取。
void discardLine() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/**
 * @brief 从标准输入读取一个整数，并限制在 [minValue, maxValue] 区间内。
 * @return true 表示读到合法值；false 表示输入流已结束（例如 Ctrl+Z / 管道输入读完）。
 *
 * 处理了三种情况：
 *   - 正常整数且范围合法：直接返回；
 *   - 输入的不是整数（failbit）：清除错误状态、丢弃该行，提示后重试；
 *   - 超出范围：提示后重试（不返回非法值，交给界面的 0 表示“返回/退出”）。
 */
bool readInt(const std::string& prompt, int minValue, int maxValue, int& result) {
    while (true) {
        std::cout << prompt << std::flush;
        int value = 0;
        if (std::cin >> value) {
            discardLine();
            if (value >= minValue && value <= maxValue) {
                result = value;
                return true;
            }
            std::cout << "  [错误] 选项 " << value << " 超出范围，请输入 "
                      << minValue << " ~ " << maxValue << " 之间的整数。\n";
        } else if (std::cin.eof()) {
            return false; // 输入结束，让上层退出，避免死循环
        } else {
            std::cin.clear();  // 清除 failbit
            discardLine();
            std::cout << "  [错误] 输入的不是整数，请重新输入。\n";
        }
    }
}

/**
 * @brief 从标准输入读取一个浮点数。
 * @return true 表示读到合法数值；false 表示输入流已结束。
 */
bool readDouble(const std::string& prompt, double& result) {
    while (true) {
        std::cout << prompt << std::flush;
        double value = 0.0;
        if (std::cin >> value) {
            discardLine();
            result = value;
            return true;
        }
        if (std::cin.eof()) {
            return false;
        }
        std::cin.clear();
        discardLine();
        std::cout << "  [错误] 输入的不是数字，请重新输入。\n";
    }
}

/// 打印“值 + 单位 = 值 + 单位”形式的结果。
void printResult(double value, const char* fromUnit,
                 double converted, const char* toUnit) {
    std::cout << "\n  结果：" << formatNumber(value) << " " << fromUnit
              << " = " << formatNumber(converted) << " " << toUnit << "\n\n";
}

void printBanner() {
    std::cout << "============================================================\n"
              << "              单位转换器 UnitConverter v" << APP_VERSION << "\n"
              << "     程序版本：git " << APP_GIT_HASH
              << "   |   转换库版本：" << unitlib::version() << "\n"
              << "============================================================\n"
              << "支持三组单位互转：\n"
              << "  1) 温度：摄氏度 (℃)  <->  华氏度 (℉)\n"
              << "  2) 长度：米 (m)      <->  英尺 (ft)\n"
              << "  3) 质量：磅 (lb)     <->  千克 (kg)\n";
}

void printMainMenu() {
    std::cout << "------------------------------------------------------------\n"
              << "  1. 温度转换（℃ <-> ℉）\n"
              << "  2. 长度转换（m <-> ft）\n"
              << "  3. 质量转换（lb <-> kg）\n"
              << "  0. 退出程序\n"
              << "------------------------------------------------------------\n";
}

/// 温度转换子菜单：输入方向、数值，调用 unitlib，捕获并展示库的异常。
void convertTemperature() {
    std::cout << "\n[温度转换]\n"
              << "  1. 摄氏度 -> 华氏度\n"
              << "  2. 华氏度 -> 摄氏度\n"
              << "  0. 返回主菜单\n";

    int direction = 0;
    if (!readInt("请选择转换方向 [0-2]: ", 0, 2, direction) || direction == 0) {
        return;
    }

    double value = 0.0;
    const bool isCelsiusToFahrenheit = (direction == 1);
    const char* prompt = isCelsiusToFahrenheit ? "请输入摄氏度 (℃): " : "请输入华氏度 (℉): ";
    if (!readDouble(prompt, value)) {
        return;
    }

    try {
        if (isCelsiusToFahrenheit) {
            printResult(value, "℃", unitlib::celsiusToFahrenheit(value), "℉");
        } else {
            printResult(value, "℉", unitlib::fahrenheitToCelsius(value), "℃");
        }
    } catch (const std::exception& error) {
        std::cout << "\n  [错误] " << error.what() << "\n\n";
    }
}

/// 长度转换子菜单：米 <-> 英尺。
void convertLength() {
    std::cout << "\n[长度转换]\n"
              << "  1. 米 -> 英尺\n"
              << "  2. 英尺 -> 米\n"
              << "  0. 返回主菜单\n";

    int direction = 0;
    if (!readInt("请选择转换方向 [0-2]: ", 0, 2, direction) || direction == 0) {
        return;
    }

    double value = 0.0;
    const bool isMeterToFeet = (direction == 1);
    if (!readDouble(isMeterToFeet ? "请输入长度 (m): " : "请输入长度 (ft): ", value)) {
        return;
    }

    try {
        if (isMeterToFeet) {
            printResult(value, "m", unitlib::metersToFeet(value), "ft");
        } else {
            printResult(value, "ft", unitlib::feetToMeters(value), "m");
        }
    } catch (const std::exception& error) {
        std::cout << "\n  [错误] " << error.what() << "\n\n";
    }
}

/// 质量转换子菜单：磅 <-> 千克。
void convertMass() {
    std::cout << "\n[质量转换]\n"
              << "  1. 磅 -> 千克\n"
              << "  2. 千克 -> 磅\n"
              << "  0. 返回主菜单\n";

    int direction = 0;
    if (!readInt("请选择转换方向 [0-2]: ", 0, 2, direction) || direction == 0) {
        return;
    }

    double value = 0.0;
    const bool isPoundToKilogram = (direction == 1);
    if (!readDouble(isPoundToKilogram ? "请输入质量 (lb): " : "请输入质量 (kg): ", value)) {
        return;
    }

    try {
        if (isPoundToKilogram) {
            printResult(value, "lb", unitlib::poundsToKilograms(value), "kg");
        } else {
            printResult(value, "kg", unitlib::kilogramsToPounds(value), "lb");
        }
    } catch (const std::exception& error) {
        std::cout << "\n  [错误] " << error.what() << "\n\n";
    }
}

} // namespace

int main() {
    enableUtf8Console();
    printBanner();

    int choice = 0;
    while (true) {
        printMainMenu();
        if (!readInt("请输入选项 [0-3]: ", 0, 3, choice)) {
            // 管道输入结束 / Ctrl+Z：正常退出而不是反复打印菜单
            std::cout << "\n输入结束，程序退出。\n";
            break;
        }

        if (choice == kExitChoice) {
            std::cout << "\n程序结束，感谢使用！\n";
            break;
        }

        switch (choice) {
            case 1:
                convertTemperature();
                break;
            case 2:
                convertLength();
                break;
            case 3:
                convertMass();
                break;
            default:
                break; // readInt 已经保证 0~3，这里只是防御性写法
        }
    }

    return 0;
}
