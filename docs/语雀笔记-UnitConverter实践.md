# UnitConverter 实践任务笔记

> 培训项目：哈工大（威海）创新平台培训 · 第二次任务（CMake）
> 仓库：HelloCV（仓库链接：待填写）
> 运行环境：Windows 11 家庭版 中文版 + Visual Studio 2026 Community（MSVC 19.51.36257）+ CMake 4.3.1-msvc1 + C++17

## 一、任务要求回顾

本次任务要求用 CMake 构建单位转换器，严格按以下结构组织：

```text
UnitConverter/
├── CMakeLists.txt            # 根目录 CMake 构建文件
├── src/main.cpp              # 主程序（交互逻辑）
├── unitlib/                  # 转换库
│   ├── CMakeLists.txt        # 库的 CMake 构建文件
│   ├── include/unitlib.h     # 头文件（函数声明）
│   └── src/unitlib.cpp       # 转换函数具体实现
└── build/                    # 编译目录（自行创建，不提交）
```

功能要求：摄氏度 ↔ 华氏度、米 ↔ 英尺、磅 ↔ 千克三组互转，交互式选择并输出结果。提交要求：仓库含 README.md 和语雀笔记链接汇总文档；至少两篇语雀笔记（学习内容 + 实践任务），要有截图、代码块和问题分析，链接公开可访问。

我的做法：HelloCV 根目录放 `UnitConverter/`、`README.md`、`docs/` 和 `.gitignore`，构建方式与测试记录写进 README，两篇笔记源稿放 `docs/`，发布后在 README 汇总表填链接。

## 二、目录结构与职责

```text
HelloCV/
├── README.md
├── .gitignore
├── docs/
│   ├── 语雀笔记-CMake学习内容.md
│   └── 语雀笔记-UnitConverter实践.md
└── UnitConverter/
    ├── CMakeLists.txt         # 根构建脚本
    ├── src/main.cpp
    ├── unitlib/
    │   ├── CMakeLists.txt     # 库构建脚本
    │   ├── include/unitlib.h
    │   └── src/unitlib.cpp
    └── build/                 # 本地生成，不提交
```

职责划分遵循「谁产生产物，谁描述自己」：根脚本负责工程声明、全局配置、引入子目录、生成主程序与安装；`unitlib/CMakeLists.txt` 只描述库；`main.cpp` 只做界面与输入校验；`unitlib.cpp` 只做换算与参数检查。

## 三、库与程序分离的设计

### 3.1 为什么拆成静态库

1. **职责分离**：换算公式是业务逻辑，菜单是界面逻辑，混在一个文件里以后换界面也得动公式；
2. **可复用**：其他任务需要换算时可直接链接 `unitlib`；
3. **可独立编译安装**：库先编成 `.lib`，主程序只依赖头文件，改界面不用重编库；
4. **接口稳定**：主程序只依赖 `unitlib.h` 的声明，实现改动不影响调用方。

### 3.2 接口设计（unitlib.h 摘选）

```cpp
namespace unitlib {

constexpr double kAbsoluteZeroCelsius = -273.15;
constexpr double kAbsoluteZeroFahrenheit = -459.67;

double celsiusToFahrenheit(double celsius);
double fahrenheitToCelsius(double fahrenheit);
double metersToFeet(double meters);
double feetToMeters(double feet);
double poundsToKilograms(double pounds);
double kilogramsToPounds(double kilograms);

const char* version();

} // namespace unitlib
```

**分析**：头文件只放声明和常量，调用方不必知道公式实现。`namespace` 避免重名，`constexpr` 让常量在编译期确定；接口统一返回 `double`，输入无物理意义时抛 `std::invalid_argument`，而不是返回 `-1` 或 `NaN`——前者会和合法结果混淆，后者容易被直接打印出去。异常保证错误不被静默忽略，由主程序统一捕获转成提示。

### 3.3 转换实现（unitlib.cpp 摘选）

```cpp
constexpr double kMetersPerFoot = 0.3048;         // 1 ft = 0.3048 m
constexpr double kKilogramsPerPound = 0.45359237; // 1 lb = 0.45359237 kg

double metersToFeet(double meters) {
    requireNonNegative(meters, "长度（米）");
    return meters / kMetersPerFoot;
}

double poundsToKilograms(double pounds) {
    requireNonNegative(pounds, "质量（磅）");
    return pounds * kKilogramsPerPound;
}
```

**分析**：换算因子只定义一次、正反方向共用，避免一边写 `0.3048` 一边写 `3.28084` 造成精度不一致；检查函数放在匿名命名空间里（内部链接，只在本文件可见）。把校验下沉到库里，任何调用方都得到一致保护，而不是依赖每个主程序自觉检查。

### 3.4 输入校验（main.cpp 摘选）

```cpp
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
```

**分析**：三种情况分开处理——成功、流结束（EOF）、格式错误。EOF 返回 `false` 而不是继续循环；格式错误先 `clear()` 清 failbit，再 `ignore` 掉整行残留字符，否则下一次读取立刻失败、形成「刷屏」。返回 `bool` 让调用方决定怎么退出，比直接 `exit()` 灵活。

## 四、两个 CMakeLists 逐段解析

### 4.1 根目录 CMakeLists.txt

**(1) 最低版本与工程声明**

```cmake
cmake_minimum_required(VERSION 3.16)
project(UnitConverter
    VERSION 1.0.0
    DESCRIPTION "单位转换器：摄氏度/华氏度、米/英尺、磅/千克"
    LANGUAGES CXX)
```

`cmake_minimum_required` 必须放第一行，决定后续命令的可用性与策略；`project()` 一次定义工程名、版本、描述和语言，后面的 `install()`、子目录、`${PROJECT_VERSION}` 都依赖它。

**(2) 全局编译配置（set）**

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

要求 C++17 且「必须支持」，不支持直接报错，而不是悄悄降级；关闭 GNU 扩展让代码保持标准 C++。另外给单配置生成器补了默认的 `CMAKE_BUILD_TYPE=Release`，并用 `CMAKE_RUNTIME_OUTPUT_DIRECTORY` 等变量把产物统一到 `build/bin` 与 `build/lib`。

**(3) 安装目录与 Git 版本号**

```cmake
include(GNUInstallDirs)

find_package(Git QUIET)
set(UNITCONVERTER_GIT_HASH "unknown")
if(Git_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE UNITCONVERTER_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
endif()
message(STATUS "UnitConverter ${PROJECT_VERSION}（git: ${UNITCONVERTER_GIT_HASH}）")
```

`include(GNUInstallDirs)` 提供标准安装目录变量，避免写死 `bin/`、`lib/`；`find_package(Git)` 演示了查找系统工具（而非第三方库）的用法，读不到提交号就保持 `unknown`，不阻断构建。`message(STATUS)` 把版本打到配置日志，便于确认。

**(4) 子目录、可执行程序与链接**

```cmake
add_subdirectory(unitlib)

add_executable(unit_converter src/main.cpp)
target_link_libraries(unit_converter PRIVATE unitlib)
```

`add_subdirectory` 负责调到 `unitlib/CMakeLists.txt`；`add_executable` 定义主程序目标；`target_link_libraries` 写目标名 `unitlib` 后，CMake 会自动带上库的头文件路径和编译要求，`PRIVATE` 表示不向其他目标传播。

**(5) 编译宏、警告与安装**

```cmake
target_compile_definitions(unit_converter PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
    APP_GIT_HASH="${UNITCONVERTER_GIT_HASH}")

if(MSVC)
    target_compile_options(unit_converter PRIVATE /W4 /utf-8)
else()
    target_compile_options(unit_converter PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS unit_converter
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
```

编译宏把版本号、提交号传给 `main.cpp` 的启动横幅；MSVC 用 `/W4 /utf-8`（高警告 + UTF-8 源码与执行字符集，解决中文乱码），其他编译器用常规警告；`install()` 用 `GNUInstallDirs` 变量安装可执行文件。

### 4.2 unitlib/CMakeLists.txt

**(1) 生成静态库并声明头文件路径**

```cmake
add_library(unitlib STATIC
    src/unitlib.cpp
    include/unitlib.h)

target_include_directories(unitlib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
```

`STATIC` 表示静态库（MSVC 下 `unitlib.lib`），改成 `SHARED` 即为动态库；头文件列进源文件不是必须的，但 IDE 里能直接看到。`target_include_directories` 用 `PUBLIC` + 生成器表达式：构建时头文件在源码 `include/`，安装后在 `<prefix>/include`，同一份脚本兼顾两种场景，链接 `unitlib` 的目标自动继承该路径。

**(2) 版本宏、C++ 标准与警告**

```cmake
target_compile_definitions(unitlib PRIVATE
    UNITLIB_VERSION_STRING="${PROJECT_VERSION}")

target_compile_features(unitlib PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(unitlib PRIVATE /W4 /utf-8)
else()
    target_compile_options(unitlib PRIVATE -Wall -Wextra -Wpedantic)
endif()
```

库版本宏只在库内部实现（`unitlib::version()`）中使用，所以是 `PRIVATE`；`target_compile_features(... PUBLIC cxx_std_17)` 让「使用 unitlib 至少需要 C++17」自动传递给消费者。

**(3) 安装库与头文件**

```cmake
install(TARGETS unitlib
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES include/unitlib.h
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
```

静态库归 `ARCHIVE`（Windows 下 `.lib`，Linux 下 `.a`），头文件用 `install(FILES ...)` 装到 `<prefix>/include`。安装规则写在子目录里，体现「谁产生产物、谁描述安装」；子目录无需再写 `cmake_minimum_required` 和 `project`。

> 📷 截图占位 1：`unitlib/CMakeLists.txt` 中 `target_include_directories` 生成器表达式那几行，以及根脚本中 `find_package(Git)` 那一段（用于笔记中标注讲解位置）。

## 五、构建与运行实录

### 5.1 方式 A：Visual Studio 生成器（本机实测通过）

```bat
cd UnitConverter
cmake -S . -B build
cmake --build build --config Release
```

配置日志关键行：

```text
-- The CXX compiler identification is MSVC 19.51.36257.0
-- UnitConverter 1.0.0（git: unknown）
-- Build files have been written to: .../UnitConverter/build
```

构建日志关键行（路径做了省略）：

```text
unitlib.cpp -> ...\build\lib\Release\unitlib.lib
main.cpp -> ...\build\bin\Release\unit_converter.exe
```

构建过程无警告。产物：`build\bin\Release\unit_converter.exe` 与 `build\lib\Release\unitlib.lib`。

> 📷 截图占位 2：终端中首次执行 `cmake -S . -B build` 的日志，必须包含 `The CXX compiler identification is MSVC 19.51.36257.0` 和 `Build files have been written to` 两行。

> 📷 截图占位 3：`cmake --build build --config Release` 成功后的日志，需包含 `unit_converter.exe` 输出路径那一行。

### 5.2 方式 B：Ninja 生成器（本机实测通过）

在 Visual Studio 开发者环境中执行：

```bat
cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ninja
```

Ninja 是单配置生成器，构建类型由 `CMAKE_BUILD_TYPE` 决定；产物不带配置子目录，位于 `build-ninja\bin\unit_converter.exe`。

> 📷 截图占位 4：Ninja 构建成功的终端日志与 `build-ninja\bin\unit_converter.exe` 的路径。

### 5.3 安装验证（本机实测通过）

```bat
cmake --install build --prefix <安装路径> --config Release
```

安装后得到三个文件：`<安装路径>/bin/unit_converter.exe`、`<安装路径>/lib/unitlib.lib`、`<安装路径>/include/unitlib.h`，说明 `GNUInstallDirs` 与 `install()` 规则生效。

> 📷 截图占位 5：安装完成后 `<安装路径>` 下的 `bin/`、`lib/`、`include/` 目录结构截图。

### 5.4 运行效果（本机实测）

```text
============================================================
              单位转换器 UnitConverter v1.0.0
     程序版本：git unknown   |   转换库版本：1.0.0
============================================================
...
请输入选项 [0-3]: 
[温度转换]
  1. 摄氏度 -> 华氏度
  2. 华氏度 -> 摄氏度
  0. 返回主菜单
请选择转换方向 [0-2]: 请输入摄氏度 (℃): 
  结果：100 ℃ = 212 ℉
```

实测换算结果：`100 ℃ = 212 ℉`、`32 ℉ = 0 ℃`、`-273.15 ℃ = -459.67 ℉`、`-459.67 ℉ = -273.15 ℃`、`1 m = 3.2808399 ft`、`1 ft = 0.3048 m`、`1 lb = 0.45359237 kg`、`1 kg = 2.2046226 lb`。

> 📷 截图占位 6：程序运行界面（启动横幅 + 主菜单 + 「结果：100 ℃ = 212 ℉」）。

## 六、测试记录表

所有用例均在本机实际运行验证，全部通过；程序正常退出返回码为 0。

| 编号 | 类别 | 输入 | 预期结果 | 实际结果 |
| --- | --- | --- | --- | --- |
| N-1 | 温度 | 100 ℃ → 华氏度 | 212 ℉ | 通过 |
| N-2 | 温度 | 32 ℉ → 摄氏度 | 0 ℃ | 通过 |
| N-3 | 温度边界 | -273.15 ℃ → 华氏度 | -459.67 ℉ | 通过 |
| N-4 | 温度边界 | -459.67 ℉ → 摄氏度 | -273.15 ℃ | 通过 |
| N-5 | 长度 | 1 m → 英尺 | 3.2808399 ft | 通过 |
| N-6 | 长度 | 1 ft → 米 | 0.3048 m | 通过 |
| N-7 | 质量 | 1 lb → 千克 | 0.45359237 kg | 通过 |
| N-8 | 质量 | 1 kg → 磅 | 2.2046226 lb | 通过 |
| E-1 | 菜单越界 | `9` | 提示「选项 9 超出范围，请输入 0 ~ 3 之间的整数。」并继续等待输入 | 通过 |
| E-2 | 非法字符 | `abc` | 提示「输入的不是整数，请重新输入。」并继续等待输入 | 通过 |
| E-3 | 温度越界 | -300 ℃ | 提示「摄氏度不能低于绝对零度 -273.15 ℃」 | 通过 |
| E-4 | 长度负数 | -2 m | 提示「长度（米）不能为负数」 | 通过 |
| E-5 | 质量负数 | -1 kg | 提示「质量（千克）不能为负数」 | 通过 |
| E-6 | 输入结束 | 管道输入读完（EOF） | 输出「输入结束，程序退出。」，不崩溃、不死循环，返回码 0 | 通过 |

## 七、问题与解决（问题 — 分析 — 解决）

**问题 1：老版 GCC 编不过 C++17**
现象：本机 Dev-C++ 自带 TDM-GCC 4.9.2，`g++ -std=c++17` 报 `unrecognized command line option '-std=c++17'`。
分析：4.9.2 发布于 2014 年，C++17 尚未定稿，编译器不认识该选项，是工具链版本问题而非代码问题。
解决：改用 Visual Studio 2026 的 MSVC（19.51.36257），并在 README 写明要求 GCC ≥ 7 / MSVC ≥ 2019 / Clang ≥ 7、CMake ≥ 3.16。

**问题 2：Windows 控制台中文编码**
现象 / 风险：源码以 UTF-8 保存，而 Windows 控制台默认按本地代码页（如 GBK）解释输出字节，中文提示可能显示为乱码；MSVC 若按本地编码解析 UTF-8 源码，也可能报 C4819 一类警告。
分析：问题出在「源文件编码」「编译器执行字符集」「控制台代码页」三个环节不一致；只处理其中一个环节仍可能乱码。
解决：双向处理——CMake 对 MSVC 追加 `/utf-8`，保证源码与执行字符集都是 UTF-8；程序启动时调用 `SetConsoleOutputCP(CP_UTF8)` 与 `SetConsoleCP(CP_UTF8)` 切换控制台代码页。

**问题 3：`min` / `max` 宏冲突（编写时预防）**
编写时注意到：`<windows.h>` 会定义 `min`、`max` 宏，若不阻止，`std::numeric_limits<T>::max()` 一类写法会被宏展开，导致编译错误。
分析：这是 Windows 头文件与 C++ 标准库的宏冲突，属于常见坑。
解决：在 `#include <windows.h>` 之前 `#define NOMINMAX` 预防，从源头阻止这两个宏被定义。

**问题 4：两种生成器产物路径不同**
实测事实：VS 生成器下可执行文件在 `build/bin/Release/`，Ninja 下在 `build/bin/`，两种方式都构建成功。
分析：VS 是多配置生成器，输出目录后会自动追加配置名；Ninja、MinGW、Unix Makefiles 是单配置生成器。根脚本虽用 `CMAKE_RUNTIME_OUTPUT_DIRECTORY` 统一了根目录，但配置子目录仍由生成器决定。
解决：README 两套命令分别写清产物路径；脚本注释也标明「多配置生成器多一层 `Release/`」，避免照搬固定路径。

**问题 5：EOF 需要专门处理**
现象：用管道 / 重定向喂入输入时，输入流会读到 EOF（本次功能测试正是用管道输入，输入读完后触发）。
分析：EOF 后 `std::cin >> value` 一直失败；如果循环不检查 `eof()`，就会无限重试、反复打印菜单。
解决：`readInt` / `readDouble` 在读取失败时先判断 `std::cin.eof()` 并返回 `false`，主循环收到后打印「输入结束，程序退出。」并结束。实测 EOF 时程序正常退出，不崩溃、不死循环，返回码 0。

**问题 6：非法输入会置 failbit**
分析：`abc` 这类输入会置 `std::cin` 的 failbit，若不 `clear()` 并丢弃整行，后续读取会连续失败。
解决与验证：当前实现先 `std::cin.clear()` 清除错误标志，再用 `discardLine()`（`ignore(max, '\n')`）丢弃整行残留字符；实测输入 `abc` 后能继续正常输入。

**问题 7：`find_package(Git)` 读不到提交号**
现象：配置日志显示 `git: unknown`。
分析：源码目录没有提交（或未安装 Git）时 `git rev-parse` 失败，变量可能为空。
解决：变量先初始化为 `unknown`，失败后仍保持，并再用 `if(NOT ...)` 兜底；版本号不阻断构建。

**问题 8：构建产物与 IDE 缓存不应提交**
分析：如果缺少 `.gitignore` 规则，`build/`、`.vs/`、`*.exe`、`*.lib` 等本地生成物会污染仓库、增大体积。
解决与验证：`.gitignore` 已补齐 `build/`、`build-*/`、`build_*/`、`CMakeFiles/`、`*.exe`、`*.lib`、`*.sln`、`*.vcxproj`、`.vs/` 等规则；实测 `git status` 与 `git add -A --dry-run` 确认 `build/` 被忽略，暂存区只会加入源码与文档。

## 八、CMake 命令覆盖对照表

| 要求命令 | 使用位置 | 作用 |
| --- | --- | --- |
| `cmake_minimum_required` | 根 `CMakeLists.txt` | 声明最低 CMake 版本 3.16 |
| `project` | 根 `CMakeLists.txt` | 声明工程名、版本 1.0.0、描述、C++ 语言 |
| `set` | 根 `CMakeLists.txt` | 设置 C++17、标准必需、关闭扩展、默认构建类型、统一输出目录 |
| `add_library` | `unitlib/CMakeLists.txt` | 把 `unitlib.cpp` 编成静态库 `unitlib` |
| `add_executable` | 根 `CMakeLists.txt` | 生成主程序 `unit_converter` |
| `target_link_libraries` | 根 `CMakeLists.txt` | 主程序链接 `unitlib`（PRIVATE） |
| `target_include_directories` | `unitlib/CMakeLists.txt` | PUBLIC 头文件路径，生成器表达式区分构建/安装场景 |
| `target_compile_definitions` | 根（版本号、Git 号）+ `unitlib`（库版本号） | 向目标传编译宏 |
| `find_package(Git)` | 根 `CMakeLists.txt` | 查找 Git 工具以读取提交号 |
| `add_subdirectory` | 根 `CMakeLists.txt` | 引入 `unitlib/` 子目录构建 |
| `install` | 根（可执行文件）+ `unitlib`（库与头文件） | 定义安装规则 |
| `include(GNUInstallDirs)` | 根 `CMakeLists.txt` | 提供标准安装目录变量 |
| `message(STATUS)` | 根 `CMakeLists.txt` | 配置阶段输出工程版本与 Git 提交号 |

另外还用到 `target_compile_features`（PUBLIC `cxx_std_17`）、`target_compile_options`、`execute_process`（执行 `git rev-parse`）与生成器表达式，属于要求之外的补充实践。

## 九、个人理解与收获

**1. CMake 是「描述」而不是「脚本」。** 一开始总想用 `if` 判断平台再手写命令，后来发现更好的方式是把「库需要什么、程序需要什么」描述清楚，让生成器决定命令。`target_link_libraries(unit_converter PRIVATE unitlib)` 一行同时解决链接、头文件路径、C++ 标准三件事。

**2. target 的传播级别很关键。** `PUBLIC` / `PRIVATE` / `INTERFACE` 是在回答「这个设置属于我，还是也属于使用者」。头文件路径写成 `PUBLIC` 后，主程序完全不用再写 include 路径，这正是现代 CMake 的优势。

**3. 库与程序分离不只是目录好看。** 拆出 `unitlib` 后，换算公式有独立编译单元和头文件，校验集中在库里，主程序变短；安装规则也自然分层。

**4. 输入健壮性要专门设计。** 交互程序最容易忽略异常输入和 EOF。把读取封装成 `readInt` / `readDouble` 并返回 `bool`，主循环和子菜单各自决定退出方式，比到处 `goto` 或 `exit()` 清晰。

**5. 构建日志值得认真读。** 配置日志里的 compiler identification、`Build files have been written to` 与产物路径，是排查环境问题最直接的线索，实测正是靠它们确认了生成器类型和产物位置。

## 十、后续可改进方向

- **接入 CTest**：为 6 个转换函数和边界值写单元测试（`enable_testing()` + `add_test()`），并在 README 补 `ctest` 步骤；
- **安装导出**：用 `install(EXPORT ...)` 生成 `unitlibConfig.cmake`，让其他项目能 `find_package(unitlib)`；
- **CMakePresets.json**：把 VS 生成器与 Ninja 两套配置固化成 preset；
- **结合后续任务接入 OpenCV**：用 `find_package(OpenCV REQUIRED)` 链接，为 HelloCV 后续图像任务打基础；
- **测试覆盖更全面**：补充 NaN / inf、极大值等用例，输出格式可做成可配置项。

> 📷 截图占位 7：Visual Studio 2026 中打开的 UnitConverter 项目结构与 `build\bin\Release\unit_converter.exe` 产物（体现实际构建环境）。
