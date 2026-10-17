# CMake 学习内容笔记

> 培训项目：哈工大（威海）创新平台培训 · 第二次任务（CMake）
> 对应实践仓库：HelloCV 中的 UnitConverter 项目（仓库链接：待填写）
> 说明：本文结合 CMake 官方文档、公开教程与本次动手实践整理，代码示例均来自本仓库真实文件。

## 一、CMake 到底解决什么问题

在 CMake 之前，多文件 C++ 项目通常要手写编译命令或 Makefile：源文件一多，编译顺序、库依赖、增量编译都得自己维护；换到 Windows 又要重写一套，Visual Studio 的 `.sln` / `.vcxproj` 与 Linux 的 Makefile 无法互换。

CMake 的定位是**构建系统生成器**：只写一份与平台无关的 `CMakeLists.txt`，CMake 根据目标平台的生成器生成对应构建文件（VS 工程、Ninja 文件、Makefile），再由底层构建工具完成编译链接。

个人理解：CMake 不是编译器也不是包管理器，它管的是「怎么把项目拼起来」——哪些源文件编成库、哪些编成程序、谁链接谁、头文件在哪、安装到哪。项目描述写对，换机器、换编译器、换 IDE 都只需重新生成。

## 二、生成器与构建工具

| 生成器 | 底层构建工具 | 配置特征 | 典型平台 |
| --- | --- | --- | --- |
| Visual Studio 18 2026 | MSBuild | 多配置，产物带 `Release/` 子目录 | Windows |
| Ninja | ninja | 单配置，需指定 `CMAKE_BUILD_TYPE` | 全平台 |
| MinGW Makefiles | mingw32-make | 单配置 | Windows |
| Unix Makefiles | make | 单配置 | Linux / macOS |

多配置生成器配置一次即可切换 Debug / Release，构建时用 `--config` 指定；单配置生成器在配置时就通过 `-DCMAKE_BUILD_TYPE=Release` 定死，构建命令不需要 `--config`。本项目根脚本对未指定构建类型的单配置生成器默认设为 Release，避免「无优化」构建。

> 📷 截图占位 1：在 VS 开发者终端执行 `cmake --version`（显示 4.3.1-msvc1）与 `cmake --help` 末尾的 Generators 列表。

## 三、配置阶段与构建阶段

CMake 的流程分两步：

- **配置阶段（configure）**：读取 `CMakeLists.txt`，检测编译器、C++ 标准支持、查找依赖，生成构建文件与 `CMakeCache.txt`；
- **构建阶段（build）**：调用底层构建工具编译链接，只有改动过的文件会重新编译。

```bash
cmake -S . -B build   # 配置：-S 源目录，-B 构建目录
cmake --build build   # 构建
```

源码外构建最经典的写法是「先进入构建目录，再执行 cmake ..」，即以 Unix Makefiles 为代表的单配置流程（Linux / macOS 常用）：

```bash
cd UnitConverter
mkdir build && cd build
cmake ..     # 生成 Makefile
make         # 调用 make 编译链接，也可用 cmake --build .
```

Windows 下不指定 `-G` 时 CMake 默认使用 Visual Studio 生成器，生成的是 `.sln` / `.vcxproj` 而不是 Makefile，因此没有 `make` 这一步；等价写法是：

```bat
cmake -S . -B build
cmake --build build --config Release
```

两种写法本质相同：都在源码目录之外生成构建文件并完成构建，区别只在生成器与底层构建工具。`cmake --build` 是跨平台统一入口，对 Makefile、Ninja、MSBuild 都适用，所以本项目统一使用 `-S/-B` + `cmake --build` 的写法。`CMakeCache.txt` 会缓存编译器路径与选项；修改 `CMakeLists.txt` 后再次构建时会自动重新配置，不需要手动删缓存。

## 四、源码外构建的意义

源码外构建（out-of-source build）指构建目录与源码目录分离。本项目所有产物都落在 `UnitConverter/build/` 中，源码目录只保留源文件与 `CMakeLists.txt`。好处有三点：

1. 源码目录干净，不会混入 `.obj`、`.exe`、`.sln`，提交仓库时不容易误提交；
2. 可以并存多套构建：`build/` 用 VS 生成器、`build-ninja/` 用 Ninja，互不干扰；
3. 清理简单，删除构建目录即可；也方便对比同一份代码在不同生成器下的产物路径差异。

个人理解：构建目录本质是「可丢弃的生成物」，所以 `.gitignore` 里要忽略 `build/`、`build-*/` 等目录。

> 📷 截图占位 2：源码外构建后的目录结构（`UnitConverter/` 源码目录只有 `src/`、`unitlib/`、`CMakeLists.txt`，而 `build/` 下有 `CMakeCache.txt`、`CMakeFiles/`、`bin/`、`lib/`）。

## 五、常用命令逐个理解

### 1. cmake_minimum_required

```cmake
cmake_minimum_required(VERSION 3.16)
```

**作用**：声明工程所需的最低 CMake 版本，同时启用对应版本的策略（policy）。本项目把它放在第一行（注释除外），低于 3.16 直接报错，而不是产生难懂的兼容问题。

### 2. project

```cmake
project(UnitConverter
    VERSION 1.0.0
    DESCRIPTION "单位转换器：摄氏度/华氏度、米/英尺、磅/千克"
    LANGUAGES CXX)
```

**作用**：声明工程名、版本、描述与语言，自动产生 `PROJECT_NAME`、`PROJECT_VERSION` 等变量。本项目用它生成版本号并传给编译宏；`LANGUAGES CXX` 表示只启用 C++，不必检测 C 编译器。**理解**：`project()` 是「工程身份」的声明，后面的 install 和子目录都能直接使用这些变量。

### 3. set

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

**作用**：定义或修改变量。本项目用 `set` 设置 C++17、要求编译器必须支持（否则直接报错而非悄悄降级）、关闭 GNU 扩展（`CMAKE_CXX_EXTENSIONS OFF`），并为单配置生成器设置默认 `CMAKE_BUILD_TYPE=Release`、统一产物输出目录。**理解**：给 `CMAKE_` 开头的内置变量赋值，本质就是「修改 CMake 的配置项」。

### 4. include(GNUInstallDirs)

```cmake
include(GNUInstallDirs)
```

**作用**：引入 CMake 自带模块，提供 `CMAKE_INSTALL_BINDIR`、`CMAKE_INSTALL_LIBDIR`、`CMAKE_INSTALL_INCLUDEDIR` 等标准安装目录变量。本项目 `install()` 用这些变量代替写死的 `bin/`、`lib/`，不同平台下更符合目录惯例。

### 5. add_subdirectory

```cmake
add_subdirectory(unitlib)
```

**作用**：把子目录加入构建，执行该目录的 `CMakeLists.txt`。本项目把库的定义拆到 `unitlib/` 子目录，根脚本只负责调度，职责更清晰。**理解**：这是「一个仓库多个模块」的标准做法，子目录可以独立描述自己的产物。

### 6. add_library

```cmake
add_library(unitlib STATIC src/unitlib.cpp include/unitlib.h)
```

**作用**：生成库目标；`STATIC` 表示静态库（Linux 下 `.a`，MSVC 下 `.lib`），改成 `SHARED` 就是动态库。本项目把换算逻辑编成静态库，主程序链接后复用；静态库不需要运行时附带 DLL，部署简单。

### 7. add_executable

```cmake
add_executable(unit_converter src/main.cpp)
```

**作用**：生成可执行文件目标。本项目的 `main.cpp` 是唯一入口，只负责交互界面，不包含换算公式。

### 8. target_link_libraries

```cmake
target_link_libraries(unit_converter PRIVATE unitlib)
```

**作用**：给目标链接库。写目标名 `unitlib` 后，CMake 会自动带上它的头文件路径与编译要求（这正是 target-based 的好处）。`PRIVATE` 表示只有 `unit_converter` 自己消费 `unitlib`，不向其他目标传播。

### 9. target_include_directories

```cmake
target_include_directories(unitlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
```

**作用**：声明头文件搜索路径。`PUBLIC` 让链接 `unitlib` 的目标自动继承。生成器表达式区分「构建时」（源码 `include/`）与「安装后」（`<prefix>/include`）两种路径，是库能被安装后复用的关键。**理解**：路径不能写死，因为同一份 `CMakeLists.txt` 要服务两种场景。

### 10. target_compile_definitions

```cmake
target_compile_definitions(unit_converter PRIVATE APP_VERSION="${PROJECT_VERSION}")
```

**作用**：给目标传编译宏。本项目把工程版本号、Git 提交号传给主程序，把库版本号传给 `unitlib`。宏的值必须是字符串字面量，所以写成 `APP_VERSION="1.0.0"`。**理解**：编译期注入版本信息，比运行时读配置文件更简单，也不会增加部署负担。

### 11. find_package

```cmake
find_package(Git QUIET)
```

**作用**：查找依赖包或系统工具，找到后提供 `Git_FOUND`、`GIT_EXECUTABLE` 等变量；`QUIET` 表示找不到时不打印警告。本项目用它找到 Git 后配合 `execute_process` 读取提交号，找不到时用 `unknown` 兜底，不阻断构建。

### 12. install

```cmake
install(TARGETS unit_converter RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
```

**作用**：定义安装规则，配合 `cmake --install` 把产物复制到指定前缀。本项目安装可执行文件、静态库和头文件，并实际验证了安装结果。

### 13. message(STATUS)

```cmake
message(STATUS "UnitConverter ${PROJECT_VERSION}（git: ${UNITCONVERTER_GIT_HASH}）")
```

**作用**：在配置阶段打印信息，`STATUS` 级别的输出带 `-- ` 前缀。本项目用它确认版本与 Git 提交号；没有 Git 提交时显示 `UnitConverter 1.0.0（git: unknown）`，有提交后显示实际短提交号。

> 📷 截图占位 3：配置日志中包含 `UnitConverter 1.0.0（git: ...）` 与 `Build files have been written to` 的片段（Git 号位置显示 `unknown` 或实际短哈希都可以）。

## 六、现代 CMake：target-based 与 PUBLIC / PRIVATE / INTERFACE

现代 CMake 提倡「以 target 为中心」：把库或程序看成 target，把编译选项、头文件路径、宏定义挂在 target 上，再通过传播规则影响使用者，而不是全局 `add_definitions` / `include_directories`。

| 关键字 | 对当前 target | 对链接它的 target |
| --- | --- | --- |
| `PRIVATE` | 需要 | 不传播 |
| `INTERFACE` | 不需要 | 传播 |
| `PUBLIC` | 需要 | 传播 |

本项目：`unitlib` 的头文件路径、`cxx_std_17` 用 `PUBLIC`（使用者同样需要）；库版本宏用 `PRIVATE`（只在库内部实现里用）；主程序链接 `unitlib` 用 `PRIVATE`（没有其他目标再链接主程序）。个人理解：选传播级别就是在回答「这个设置属于我，还是也属于我的使用者」。用对之后，别人链接你的库不需要再手动补一堆 include 路径和宏，比全局变量可维护得多。

## 七、生成器表达式

生成器表达式 `$<...>` 是「配置或构建时才会求值」的写法。最典型的用途是区分构建接口与安装接口：源码树里头文件在 `unitlib/include/`，安装后却在 `<prefix>/include/`，一份 `CMakeLists.txt` 要同时支持两种场景，就必须写 `$<BUILD_INTERFACE:...>` 和 `$<INSTALL_INTERFACE:...>`。其他常见形式还有 `$<CONFIG:Release>`（按配置区分）、`$<CXX_COMPILER_ID:MSVC>`（按编译器区分）。

个人理解：它把「同一段配置在不同场景取不同值」变成声明式表达，避免写 `if()` 分支，也避免为安装场景复制一份构建脚本。

## 八、一次完整构建流程（本项目）

```bash
cmake -S . -B build                                        # 配置，生成构建文件
cmake --build build --config Release                       # 构建（VS 多配置）
cmake --install build --prefix <prefix> --config Release   # 安装
```

换生成器只需换 `-G` 和构建类型参数：`cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release`。产物路径随生成器变化：VS 在 `build/bin/Release/`，Ninja 在 `build/bin/`，这是理解「生成器差异」最直观的例子。

> 📷 截图占位 4：在 `UnitConverter` 目录执行首次 `cmake -S . -B build` 时终端输出的前几行和最后一行。

## 九、常见错误排查

- **找不到编译器**（`could not find CMAKE_CXX_COMPILER`）：未进入 VS 开发者环境，或生成器与工具链不匹配。
- **同一构建目录混用生成器**：`build/` 已用 VS 生成器配置，再加 `-G Ninja` 会报生成器不一致，应另建目录。
- **`unrecognized command line option '-std=c++17'`**：GCC 版本过低（本机 Dev-C++ 自带 TDM-GCC 4.9.2 即如此），需 GCC ≥ 7 或改用 MSVC。
- **中文乱码 / C4819 警告**：MSVC 需 `/utf-8`，运行时还需把控制台代码页切到 UTF-8。
- **找不到头文件**：`target_include_directories` 缺失，或错误地写成 `PRIVATE` 导致使用者拿不到路径。
- **链接错误 undefined reference**：忘记 `target_link_libraries`（本项目链接静态库必须显式声明）。
- **构建成功但找不到产物**：去对应生成器输出目录找，多配置生成器记得 `Release/` 子目录。
- **配置缓存异常**：删除构建目录重新配置，比猜缓存内容更快。

## 十、参考资料

- CMake 官方文档：[cmake.org/cmake/help/latest](https://cmake.org/cmake/help/latest/)（命令与变量定义的权威来源，本文命令解释以此为准）
- CMake 官方教程：[CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)（循序渐进的入门教程）
- 《CMake 入门实战》：[hahack.com/codes/cmake](https://www.hahack.com/codes/cmake/)（中文入门教程，用于理解生成器与构建流程）
- 个人实践：本仓库 UnitConverter 项目的构建、运行、安装记录，文中数值与日志均来自实际动手验证。

## 十一、小结

CMake 的学习重点不在背命令，而在建立三组关系：源码与构建目录的关系（源码外构建）、生成器与构建工具的关系（谁生成谁执行）、target 与使用者的关系（`PRIVATE` / `PUBLIC` / `INTERFACE` 的传播）。把这三组关系想清楚，再看具体命令就只是语法问题。下一步准备在 UnitConverter 上接入 CTest 补单元测试，并尝试 `find_package(OpenCV)`，为后续图像处理任务打基础。
