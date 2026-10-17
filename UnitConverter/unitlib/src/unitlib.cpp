#include "unitlib.h"

#include <cmath>
#include <stdexcept>
#include <string>

// 如果没有经过 CMake 构建（例如用 Dev-C++ 直接编译本文件），
// 编译命令不会传入 UNITLIB_VERSION_STRING，这里给一个默认值，
// 保证源码在任何构建方式下都能编译通过。
#ifndef UNITLIB_VERSION_STRING
#define UNITLIB_VERSION_STRING "unknown"
#endif

namespace {

// 精确换算因子（constexpr 表示编译期常量，运行时没有额外开销）
constexpr double kMetersPerFoot = 0.3048;         // 1 ft = 0.3048 m（国际英尺）
constexpr double kKilogramsPerPound = 0.45359237; // 1 lb = 0.45359237 kg（国际磅）

// 下面三个函数只在本翻译单元内可见，放在匿名命名空间里等价于“内部链接”，
// 不会和其他文件里的同名函数冲突。

void requireFinite(double value, const char* name) {
    // NaN 和 ±inf 都参与不了有意义的换算，提前拦截，避免输出莫名其妙的数值
    if (!std::isfinite(value)) {
        throw std::invalid_argument(std::string(name) + "不是有效的有限数值");
    }
}

void requireCelsius(double celsius) {
    requireFinite(celsius, "摄氏度");
    if (celsius < unitlib::kAbsoluteZeroCelsius) {
        throw std::invalid_argument("摄氏度不能低于绝对零度 -273.15 ℃");
    }
}

void requireFahrenheit(double fahrenheit) {
    requireFinite(fahrenheit, "华氏度");
    if (fahrenheit < unitlib::kAbsoluteZeroFahrenheit) {
        throw std::invalid_argument("华氏度不能低于绝对零度 -459.67 ℉");
    }
}

void requireNonNegative(double value, const char* name) {
    requireFinite(value, name);
    if (value < 0.0) {
        throw std::invalid_argument(std::string(name) + "不能为负数");
    }
}

} // namespace

namespace unitlib {

double celsiusToFahrenheit(double celsius) {
    requireCelsius(celsius);
    return celsius * 9.0 / 5.0 + 32.0;
}

double fahrenheitToCelsius(double fahrenheit) {
    requireFahrenheit(fahrenheit);
    return (fahrenheit - 32.0) * 5.0 / 9.0;
}

double metersToFeet(double meters) {
    requireNonNegative(meters, "长度（米）");
    return meters / kMetersPerFoot;
}

double feetToMeters(double feet) {
    requireNonNegative(feet, "长度（英尺）");
    return feet * kMetersPerFoot;
}

double poundsToKilograms(double pounds) {
    requireNonNegative(pounds, "质量（磅）");
    return pounds * kKilogramsPerPound;
}

double kilogramsToPounds(double kilograms) {
    requireNonNegative(kilograms, "质量（千克）");
    return kilograms / kKilogramsPerPound;
}

const char* version() {
    return UNITLIB_VERSION_STRING;
}

} // namespace unitlib
