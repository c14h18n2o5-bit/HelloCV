# ============================================================================
#  unitlib 子目录构建脚本
#  职责：把单位转换函数编译成静态库，并对外暴露头文件目录
#
#  注意：这个文件是被根目录 CMakeLists.txt 里的 add_subdirectory(unitlib)
#        调用的，所以它执行时 PROJECT_NAME 等变量已经由根目录的 project() 定义好，
#        CMAKE_INSTALL_LIBDIR / CMAKE_INSTALL_INCLUDEDIR 也已由根目录的
#        include(GNUInstallDirs) 提供。
# ============================================================================

# 1. 生成静态库（add_library） -----------------------------------------------
#    第二个参数 STATIC 表示静态库（.a / .lib）；改成 SHARED 就是动态库。
#    头文件列进源文件列表不是必须的，但这样 IDE / VS 里能直接看到它。
add_library(unitlib STATIC
    src/unitlib.cpp
    include/unitlib.h)

# 2. 头文件搜索路径（target_include_directories） ----------------------------
#    使用生成器表达式区分“构建时”和“安装后”两种场景：
#      BUILD_INTERFACE  ：在 build 目录里编译时，头文件位于源码目录 include/
#      INSTALL_INTERFACE：执行 install 后，头文件位于 <prefix>/include
#    PUBLIC 表示：unitlib 自己需要这个路径，链接 unitlib 的目标也会自动继承。
target_include_directories(unitlib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# 3. 编译宏（target_compile_definitions） ------------------------------------
#    库版本号只在 unitlib 内部实现里使用（unitlib::version()），
#    所以设为 PRIVATE，不向使用方暴露不必要的宏。
target_compile_definitions(unitlib PRIVATE
    UNITLIB_VERSION_STRING="${PROJECT_VERSION}")

# 4. 对外声明 C++ 标准要求 ----------------------------------------------------
#    PUBLIC + cxx_std_17：凡链接 unitlib 的目标自动获得“至少 C++17”的编译要求。
target_compile_features(unitlib PUBLIC cxx_std_17)

# 5. 编译警告 ----------------------------------------------------------------
#    库代码同样开启警告；MSVC 需要 /utf-8，否则源码中的中文注释可能引发 C4819。
if(MSVC)
    target_compile_options(unitlib PRIVATE /W4 /utf-8)
else()
    target_compile_options(unitlib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 6. 安装规则（install） ------------------------------------------------------
#    静态库：Linux 下是 libunitlib.a（ARCHIVE），Windows+MinGW 下是 libunitlib.a，
#            MSVC 下是 unitlib.lib，也归在 ARCHIVE 里。
#    头文件：安装到 <prefix>/include/unitlib.h，使用方 #include "unitlib.h" 即可。
install(TARGETS unitlib
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES include/unitlib.h
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
