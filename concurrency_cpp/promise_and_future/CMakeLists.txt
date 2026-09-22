# CMakeList.txt: promise_and_future 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()




project ("promise_and_future")



# 设置编译选项
if (MSVC)
# /W4 = 警告等级 4（MSVC 最严格的常规警告级别）
# /utf-8 → 强制使用 UTF-8 编码
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 将源代码添加到此项目的可执行文件。
add_executable (promise_and_future "promise_and_future.cpp" "promise_and_future.h"
 "async_and_packaged_task.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET promise_and_future PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
