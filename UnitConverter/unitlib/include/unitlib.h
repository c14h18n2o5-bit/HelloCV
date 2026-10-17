#ifndef UNITLIB_H
#define UNITLIB_H

/**
 * @file unitlib.h
 * @brief 单位转换库的对外接口：温度（℃/℉）、长度（米/英尺）、质量（磅/千克）。
 *
 * 设计思路：
 *   - 头文件只放“函数声明”和常量，具体的换算公式写在 src/unitlib.cpp 里；
 *   - 主程序 #include "unitlib.h" 并链接 unitlib 静态库即可调用，
 *     不需要知道公式如何实现。这正是“把可复用的功能封装成库”的意义，
 *     以后其他项目也能直接复用这个库。
 *
 * 错误处理约定：
 *   当输入在物理上没有意义时（例如低于绝对零度的温度、负的长度或质量），
 *   函数抛出 std::invalid_argument，由调用方（main.cpp）捕获并提示用户，
 *   而不是返回一个“看起来正常”的假结果。
 */

namespace unitlib {

/// 绝对零度（摄氏度表示），低于该值的温度没有物理意义
constexpr double kAbsoluteZeroCelsius = -273.15;

/// 绝对零度（华氏度表示）
constexpr double kAbsoluteZeroFahrenheit = -459.67;

/**
 * @brief 摄氏度 -> 华氏度，公式：F = C * 9 / 5 + 32
 * @param celsius 摄氏度
 * @return 对应的华氏度
 * @throws std::invalid_argument 数值非有限值，或低于绝对零度
 */
double celsiusToFahrenheit(double celsius);

/**
 * @brief 华氏度 -> 摄氏度，公式：C = (F - 32) * 5 / 9
 * @param fahrenheit 华氏度
 * @return 对应的摄氏度
 * @throws std::invalid_argument 数值非有限值，或低于绝对零度
 */
double fahrenheitToCelsius(double fahrenheit);

/**
 * @brief 米 -> 英尺，1 英尺 = 0.3048 米（国际英尺定义）
 * @param meters 米
 * @return 对应的英尺
 * @throws std::invalid_argument 数值非有限值，或为负数
 */
double metersToFeet(double meters);

/**
 * @brief 英尺 -> 米
 * @param feet 英尺
 * @return 对应的米
 * @throws std::invalid_argument 数值非有限值，或为负数
 */
double feetToMeters(double feet);

/**
 * @brief 磅 -> 千克，1 磅 = 0.45359237 千克（国际磅定义）
 * @param pounds 磅
 * @return 对应的千克
 * @throws std::invalid_argument 数值非有限值，或为负数
 */
double poundsToKilograms(double pounds);

/**
 * @brief 千克 -> 磅
 * @param kilograms 千克
 * @return 对应的磅
 * @throws std::invalid_argument 数值非有限值，或为负数
 */
double kilogramsToPounds(double kilograms);

/**
 * @brief 返回 unitlib 的版本号，例如 "1.0.0"。
 *
 * 版本号并不是写死在源码里的，而是由 CMake 的
 * target_compile_definitions(unitlib PRIVATE UNITLIB_VERSION_STRING="...")
 * 在编译时传入；用普通编译器直接编译时会回退为 "unknown"。
 */
const char* version();

} // namespace unitlib

#endif // UNITLIB_H
