# ============================================================================
#  UnitConverter 根目录构建脚本
#  职责：工程声明、全局配置、添加子目录、构建可执行程序、安装规则
#
#  标准用法（源码外构建，所有编译产物都在 build/ 里，不污染源码目录）：
#      cd UnitConverter
#      mkdir build && cd build
#      cmake ..            # Windows 默认使用 Visual Studio 生成器；
#                          # 如用 MinGW，则用：cmake -G "MinGW Makefiles" ..
#      cmake --build .     # 等价于 make / mingw32-make；VS 再多配置生成器需加 --config Release
#
#  产物位置：
#      Visual Studio 等多配置生成器：build/bin/Release/unit_converter.exe
#      Ninja / MinGW / Linux 等单配置生成器：build/bin/unit_converter.exe
# ============================================================================

# 1. 最低版本与工程声明 -------------------------------------------------------
#    cmake_minimum_required 必须放在第一行（注释除外），它决定后面命令的可用性。
cmake_minimum_required(VERSION 3.16)

#    project() 会同时定义 PROJECT_NAME / PROJECT_VERSION 等变量，
#    后面的 install() 和子目录都能直接使用。
project(UnitConverter
    VERSION 1.0.0
    DESCRIPTION "单位转换器：摄氏度/华氏度、米/英尺、磅/千克"
    LANGUAGES CXX)

#    配置阶段打印工程信息，确认当前构建的是哪个版本（message(STATUS) 会带 "-- " 前缀）。
message(STATUS "UnitConverter v${PROJECT_VERSION}")

# 2. 全局编译配置（set） ------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 编译器不支持 C++17 就直接报错，而不是悄悄降级
set(CMAKE_CXX_EXTENSIONS OFF)         # 关闭 gnu++17 扩展，保证代码在各平台都是标准 C++

# 未指定 CMAKE_BUILD_TYPE 时默认使用 Release：
# 单配置生成器（Ninja / MinGW Makefiles / Unix Makefiles）没有默认构建类型，
# 如果不设置，编译器不会开任何优化。
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "构建类型（Debug / Release）" FORCE)
endif()

# 统一产物输出目录：可执行文件 -> build/bin，库文件 -> build/lib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# GNUInstallDirs 提供 CMAKE_INSTALL_BINDIR / LIBDIR / INCLUDEDIR 等标准目录变量，
# 让 install() 的路径符合平台惯例，而不是写死 bin/ lib/。
include(GNUInstallDirs)

# 3. 添加转换库子目录（add_subdirectory） -------------------------------------
#    unitlib/CMakeLists.txt 负责把转换函数编译成静态库 unitlib，
#    并对外声明头文件目录（PUBLIC），主程序链接后即可使用。
add_subdirectory(unitlib)

# 4. 构建可执行程序 -----------------------------------------------------------
add_executable(unit_converter src/main.cpp)

# 链接：写目标名 unitlib 即可，CMake 会自动带上它的头文件路径和编译要求。
# PRIVATE 表示“只有 unit_converter 自己需要 unitlib，不需要传播给别的目标”。
target_link_libraries(unit_converter PRIVATE unitlib)

# 编译宏：把工程版本号传给 main.cpp，用于启动时显示版本信息。
# 宏的值必须是字符串字面量，所以这里写成 APP_VERSION="1.0.0"。
target_compile_definitions(unit_converter PRIVATE
    APP_VERSION="${PROJECT_VERSION}")

# 编译警告与编码设置：
#   MSVC：/W4 开启较高警告等级；/utf-8 让源码按 UTF-8 解析、执行字符集也是 UTF-8，
#         否则源码里的中文提示在 Windows 上会乱码。
#   GCC/Clang：-Wall -Wextra -Wpedantic 打开常规警告。
if(MSVC)
    target_compile_options(unit_converter PRIVATE /W4 /utf-8)
else()
    target_compile_options(unit_converter PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 5. 安装规则（install） ------------------------------------------------------
#    使用：cmake --install build --prefix <安装路径>
#    会把可执行文件装到 <prefix>/bin 下。
install(TARGETS unit_converter
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
