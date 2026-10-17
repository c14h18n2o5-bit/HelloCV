# HelloCV

哈尔滨工业大学（威海）创新平台培训 · **第二次任务（CMake）** 的代码与文档仓库。

## 仓库简介

`HelloCV` 用于存放创新平台培训各次任务的代码与文档。本次（第二次任务）的主题是 **CMake 构建系统**：
在 `UnitConverter/` 中实现可交互的单位转换器，按任务书要求的目录组织源码与构建脚本，把换算逻辑封装成静态库 `unitlib`，并用 CMake 完成配置、构建、安装与版本信息注入；学习过程整理成两篇公开语雀笔记，链接见下文汇总。

仓库包含以下内容：

- `UnitConverter/`：本次 CMake 任务项目（单位转换器主程序 + 转换库 `unitlib`）；
- `README.md`：项目结构、环境配置、构建运行、测试记录与笔记链接；
- `docs/`：两篇语雀笔记的 Markdown 源稿（发布到语雀后以链接形式提交）；
- `docs/语雀笔记链接汇总.md`：所有语雀笔记（含第一次任务 Linux / Git 笔记）的链接汇总与可访问性自检说明；
- `.gitignore`：忽略构建目录、编译产物与 IDE 文件，保证 `build/` 不进入版本库。

## 功能特性

- **三组单位互转**：摄氏度 ↔ 华氏度、米 ↔ 英尺、磅 ↔ 千克，覆盖两个换算方向共 6 个转换函数；
- **交互式菜单**：启动后显示版本信息与主菜单，可选择温度 / 长度 / 质量子菜单，输入 `0` 返回或退出；
- **库与程序分离**：换算公式与参数校验放在静态库 `unitlib` 中，主程序只负责界面交互与错误提示，
  库可被其他项目直接复用；
- **输入健壮**：非法字符、超范围选项、负数、低于绝对零度的温度、管道输入结束（EOF）都有明确提示，
  程序不崩溃、不死循环；
- **CMake 现代写法**：以 target 为中心，用 `target_link_libraries` / `target_include_directories` /
  `target_compile_definitions` / `target_compile_features` 传递使用要求，而不是全局变量；
- **版本信息与安装**：工程版本号由 `project(UnitConverter VERSION 1.0.0)` 定义，CMake 通过 `target_compile_definitions` 以 `APP_VERSION` 宏传给程序，启动横幅显示 `v1.0.0`；支持 `cmake --install`，可执行文件、静态库、头文件按 GNU 目录惯例安装到指定前缀。

## 目录结构

```text
HelloCV/
├── README.md                        # 仓库说明（本文件）
├── .gitignore                       # 忽略 build/、编译产物、IDE 文件
├── docs/
│   ├── 语雀笔记-CMake学习内容.md     # 语雀笔记源稿（学习内容）
│   ├── 语雀笔记-UnitConverter实践.md # 语雀笔记源稿（实践任务）
│   └── 语雀笔记链接汇总.md           # 所有语雀笔记链接汇总（交付要求的汇总文档）
└── UnitConverter/
    ├── CMakeLists.txt               # 根构建脚本：工程声明、全局配置、子目录、可执行程序、安装
    ├── src/
    │   └── main.cpp                 # 主程序：控制台编码、菜单循环、输入校验、异常提示
    ├── unitlib/
    │   ├── CMakeLists.txt           # 库构建脚本：静态库、头文件目录、安装
    │   ├── include/
    │   │   └── unitlib.h            # 库对外接口：转换函数声明、绝对零度常量、版本号
    │   └── src/
    │       └── unitlib.cpp          # 转换函数实现：换算公式与参数合法性检查
    └── build/                       # 本地编译目录（不提交，已被 .gitignore 忽略）
        ├── bin/Release/unit_converter.exe
        └── lib/Release/unitlib.lib
```

> `build/` 由本地 `cmake` 配置时自动生成，不同生成器的产物子目录不同（多配置生成器多一层 `Release/`）。
> 编译产物仅用于本地验证，不进入 Git 仓库；`.gitignore` 同时忽略 `build-*/`、`build_*/` 与 `.vs/` 等目录。

## 项目结构职责说明

| 路径 | 职责 |
| --- | --- |
| `UnitConverter/CMakeLists.txt` | 声明工程与最低 CMake 版本；设置 C++17、默认构建类型、统一输出目录；`add_subdirectory(unitlib)` 引入库；生成 `unit_converter` 可执行文件并链接库、注入版本宏；配置安装规则。 |
| `UnitConverter/unitlib/CMakeLists.txt` | 把 `unitlib.cpp` 编成静态库；对外声明头文件目录（构建时 / 安装后两种路径）与 C++17 要求；安装库文件与头文件。 |
| `UnitConverter/src/main.cpp` | 程序入口与交互逻辑：切换控制台 UTF-8 编码、打印横幅与菜单、读取并校验输入、调用库函数、捕获并展示异常。不含换算公式。 |
| `UnitConverter/unitlib/include/unitlib.h` | 库的公开接口：6 个转换函数、绝对零度常量、`version()`；只声明不实现，调用方无需关心公式。 |
| `UnitConverter/unitlib/src/unitlib.cpp` | 换算公式与参数检查（非有限值、低于绝对零度、负数时抛出 `std::invalid_argument`）。 |
| `.gitignore` | 忽略 `build/`、可执行文件、目标文件、库文件、IDE 工程与系统文件，避免提交编译产物。 |

## 环境要求

| 项目 | 要求 / 本机实测环境 |
| --- | --- |
| 操作系统 | Windows 11 家庭版 中文版（实测）；Linux / macOS 为参考流程，未在本机验证 |
| 编译器 / 构建工具 | MSVC 19.51.36257（Visual Studio 2026 Community，MSBuild 18.10.1，Windows SDK 10.0.26100.0，自带 Ninja）；通用要求 GCC ≥ 7 / MSVC ≥ 2019 / Clang ≥ 7 |
| CMake | 4.3.1-msvc1（随 Visual Studio 安装）；最低要求 3.16 |
| C++ 标准 | C++17（`CMAKE_CXX_STANDARD 17`，`CMAKE_CXX_STANDARD_REQUIRED ON`） |
| 其他 | 无需第三方库；CMake ≥ 3.16 + C++17 编译器即可 |

### 环境准备（Windows，只需做一次）

Visual Studio 自带的 CMake 默认**不在 PATH** 中，直接在 VS Code / PowerShell 里敲 `cmake` 会报
`无法将“cmake”项识别为 cmdlet、函数、脚本文件或可运行程序的名称`。任选一种方式解决：

1. **把 VS 自带 CMake 加入用户 PATH（推荐，一次配置永久生效）**：本机路径为
   `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`
   （随 VS 版本和安装位置不同而变化）。把它加入「系统属性 → 环境变量 → 用户变量 Path」后，
   **完全退出并重新打开 VS Code**（只新开终端不够，VS Code 进程需要重启才能继承新的 PATH），新终端即可识别 `cmake`。
2. **使用 VS 开发者终端**：打开「Developer PowerShell for VS 2026」或「x64 Native Tools Command Prompt for VS 2026」，
   这些终端会自动把 CMake、MSVC、Ninja 加入 PATH，可直接执行构建命令。
3. **只在当前终端临时生效**：

   ```powershell
   $env:PATH = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;" + $env:PATH
   ```

验证：`cmake --version` 能输出 `cmake version 4.3.1-msvc1`（本机实测）即配置成功。

## 构建与运行

### 方式 A：Windows + Visual Studio 生成器（本机实测通过）

在普通终端或 Visual Studio 开发者终端中执行：

```bat
cd UnitConverter
cmake -S . -B build
cmake --build build --config Release
build\bin\Release\unit_converter.exe
```

- 未指定 `-G` 时 CMake 会生成 Visual Studio 工程（多配置），`--config` 决定构建 Debug 还是 Release；
- 产物位于 `build\bin\Release\unit_converter.exe`，库位于 `build\lib\Release\unitlib.lib`；
- 若提示找不到编译器，请改用「x64 Native Tools Command Prompt for VS 2026」或 Developer PowerShell 再试。

### 方式 B：Windows + Ninja（本机实测通过）

Ninja 是单配置生成器，构建类型通过 `CMAKE_BUILD_TYPE` 指定；需要先进入 VS 开发者环境（保证 `cl.exe`、`ninja` 在 PATH 中）：

```bat
cd UnitConverter
cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ninja
build-ninja\bin\unit_converter.exe
```

单配置生成器的产物不带配置子目录，可直接执行 `build-ninja\bin\unit_converter.exe`。

### 方式 C：Linux / macOS / MinGW（参考流程，未在本机验证）

Linux / macOS 的命令与上面一致，使用默认的 Unix Makefiles 生成器即可：

```bash
cd UnitConverter
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/bin/unit_converter
```

Windows 下使用 MinGW-w64（GCC ≥ 7）时：

```bat
cd UnitConverter
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
build\bin\unit_converter.exe
```

> 说明：本机 Dev-C++ 自带的 TDM-GCC 4.9.2 不支持 C++17，因此未在本机完成 MinGW 构建验证；
> Linux / macOS / 新版 MinGW-w64 的步骤为通用标准流程，可参考，命令与其他平台一致。

### 安装（可选）

```bat
cmake --install build --prefix <安装路径> --config Release
```

安装后得到 `<安装路径>/bin/unit_converter.exe`、`<安装路径>/lib/unitlib.lib`、`<安装路径>/include/unitlib.h`（本机实测通过）。

## 运行效果示例

以下为本机实际运行输出的片段（输入通过管道喂入，因此日志中没有键盘回显；中间省略了重复的菜单部分）：

```text
============================================================
              单位转换器 UnitConverter v1.0.0
              转换库版本：1.0.0
============================================================
支持三组单位互转：
  1) 温度：摄氏度 (℃)  <->  华氏度 (℉)
  2) 长度：米 (m)      <->  英尺 (ft)
  3) 质量：磅 (lb)     <->  千克 (kg)
------------------------------------------------------------
...
请选择转换方向 [0-2]: 请输入摄氏度 (℃): 
  结果：100 ℃ = 212 ℉
```

实测的 8 组正常换算结果：

- `100 ℃ = 212 ℉`；`32 ℉ = 0 ℃`；`-273.15 ℃ = -459.67 ℉`；`-459.67 ℉ = -273.15 ℃`；
- `1 m = 3.2808399 ft`；`1 ft = 0.3048 m`；`1 lb = 0.45359237 kg`；`1 kg = 2.2046226 lb`。

## 测试用例

所有用例均在本机实际运行验证，全部通过（程序正常返回码为 0）。

| 编号 | 类别 | 输入 | 预期结果 | 实际结果 |
| --- | --- | --- | --- | --- |
| N-1 | 温度 | 100 ℃ → 华氏度 | `212 ℉` | 通过 |
| N-2 | 温度 | 32 ℉ → 摄氏度 | `0 ℃` | 通过 |
| N-3 | 温度边界 | -273.15 ℃ → 华氏度 | `-459.67 ℉` | 通过 |
| N-4 | 温度边界 | -459.67 ℉ → 摄氏度 | `-273.15 ℃` | 通过 |
| N-5 | 长度 | 1 m → 英尺 | `3.2808399 ft` | 通过 |
| N-6 | 长度 | 1 ft → 米 | `0.3048 m` | 通过 |
| N-7 | 质量 | 1 lb → 千克 | `0.45359237 kg` | 通过 |
| N-8 | 质量 | 1 kg → 磅 | `2.2046226 lb` | 通过 |
| E-1 | 菜单越界 | 输入 `9` | 提示 `选项 9 超出范围，请输入 0 ~ 3 之间的整数。` 后继续等待输入 | 通过 |
| E-2 | 非法字符 | 菜单输入 `abc` | 提示 `输入的不是整数，请重新输入。` 后继续等待输入 | 通过 |
| E-3 | 温度越界 | -300 ℃ | 提示 `摄氏度不能低于绝对零度 -273.15 ℃`，程序继续运行 | 通过 |
| E-4 | 长度负数 | -2 m | 提示 `长度（米）不能为负数`，程序继续运行 | 通过 |
| E-5 | 质量负数 | -1 kg | 提示 `质量（千克）不能为负数`，程序继续运行 | 通过 |
| E-6 | 输入结束 | 管道输入读完（EOF） | 输出 `输入结束，程序退出。`，不崩溃、不死循环，返回码 0 | 通过 |

## 语雀笔记汇总

| 序号 | 笔记主题 | 对应任务 | 语雀链接 |
| --- | --- | --- | --- |
| 1 | Linux 基础学习笔记 | 第一次任务（Linux） | 替换为已发布链接（待填写） |
| 2 | Git 学习笔记 | 第一次任务（Git） | 替换为已发布链接（待填写） |
| 3 | CMake 学习内容 | 第二次任务（CMake） | （待填写：发布语雀后粘贴链接） |
| 4 | UnitConverter 实践任务 | 第二次任务（CMake） | （待填写：发布语雀后粘贴链接） |

**提交前必须把上表的占位内容全部替换为真实、公开可访问的语雀链接；链接需在未登录状态下也能打开。**
同一份链接汇总也见 `docs/语雀笔记链接汇总.md`（含链接公开可访问的自检方法），两处需同步更新。
两篇笔记的 Markdown 源稿位于 `docs/` 目录，发布到语雀后请同步更新本表。

## 常见问题 FAQ

**Q1：执行 `g++ -std=c++17` 报 `unrecognized command line option '-std=c++17'`？**
本机 Dev-C++ 自带的 TDM-GCC 4.9.2 版本过旧，不支持 C++17。本项目改用 Visual Studio 2026 的 MSVC 构建。
通用要求是 GCC ≥ 7、MSVC ≥ 2019、Clang ≥ 7，CMake ≥ 3.16。

**Q2：程序运行后中文提示显示为乱码？**
源码为 UTF-8，而 Windows 控制台默认使用本地代码页。项目做了两件事：CMake 对 MSVC 追加 `/utf-8`
编译选项，保证源码与执行字符集都是 UTF-8；程序启动时调用 `SetConsoleOutputCP(CP_UTF8)` /
`SetConsoleCP(CP_UTF8)` 切换控制台代码页。两者缺一不可。

**Q3：为什么 `main.cpp` 在 `#include <windows.h>` 之前要 `#define NOMINMAX`？**
`<windows.h>` 会定义 `min` / `max` 宏；若不阻止，`std::numeric_limits<T>::max()` 这类写法会先被宏展开，
导致编译错误。因此在 include 之前定义 `NOMINMAX`，从源头阻止这两个宏被引入。

**Q4：VS 生成器和 Ninja 的产物路径为什么不一样？**
Visual Studio 是多配置生成器，CMake 在输出目录后自动追加配置名，因此可执行文件在 `build/bin/Release/`；
Ninja、MinGW Makefiles、Unix Makefiles 是单配置生成器，产物直接在 `build/bin/`。根脚本已用 `CMAKE_RUNTIME_OUTPUT_DIRECTORY` 等变量统一根目录，但配置子目录仍由生成器决定。

**Q5：用管道或重定向输入，或者按 Ctrl+Z 后程序疯狂刷屏？**
输入流读完后 `std::cin >> value` 会失败并置 EOF 位。如果读取失败后不处理 EOF，菜单循环会无限打印。
`readInt` / `readDouble` 检测到 `std::cin.eof()` 时返回 `false`，主循环收到后打印「输入结束，程序退出。」并正常退出。

**Q6：输入一次 `abc` 后，后面所有输入都读不进去了？**
非法输入会让 `std::cin` 置 failbit。必须在提示前 `std::cin.clear()` 清除错误状态，再用
`std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n')` 丢弃整行残留字符，否则后续读取会连续失败。

**Q7：启动横幅里的 `v1.0.0` 是写死的吗？**
不是。它来自 `project(UnitConverter VERSION 1.0.0)` 中的 `VERSION`（即 `${PROJECT_VERSION}`），由
`target_compile_definitions` 以 `APP_VERSION` 宏传给 `main.cpp`。改版本号只需修改 `project()` 的 `VERSION`，
然后重新配置、构建即可。

**Q8：`build/` 目录需要提交吗？Visual Studio 的 `.vs/` 呢？**
都不需要也不能提交。`build/` 是本地生成的编译目录，体积大且与机器环境相关；`.gitignore` 已忽略 `build/`、`build-*/`、`build_*/` 这类构建目录，以及 `.vs/` 缓存目录、`*.vcxproj`、`*.sln` 等 IDE 生成文件。
需要重新构建时，删除 `build/` 或改用新的构建目录（如 `build-ninja/`）重新配置即可。

**Q9：终端提示「无法将“cmake”项识别为 cmdlet、函数、脚本文件或可运行程序的名称」？**
Visual Studio 自带的 CMake 默认没有加入 PATH。按上文「环境准备（Windows）」把
`...\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin` 加入用户 PATH 并重启 VS Code，
或改用 VS 开发者终端；也可在当前终端临时执行
`$env:PATH = "...\CMake\bin;" + $env:PATH` 后再运行 `cmake`。本机已按方式 1 配置并验证通过。

## 提交前检查清单

- [ ] `UnitConverter/build/`、`build-ninja/` 等构建目录与所有编译产物均未提交（已由 `.gitignore` 忽略）；
- [ ] `README.md` 与 `docs/语雀笔记链接汇总.md` 中的四个语雀笔记链接均已替换为真实链接，且在未登录状态下可公开访问；
- [ ] `README.md` 的描述与当前代码、目录结构一致（如命令、路径、版本号）；
- [ ] 两篇语雀笔记已发布，包含代码块、截图（替换掉占位说明）与问题分析；
- [ ] 仓库已设为公开（Public），并在干净环境中重新 clone 后按 README 命令成功配置、构建、运行；
- [ ] 提交前用 `git status` 确认没有误提交的个人路径、密码或无关文件。
