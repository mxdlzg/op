# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.0)

# 编译选项
option(build_swig_py "build swig py,requrie python" OFF)
option(build_qttool "buid qt tool,requrie qt lib" OFF)
option(OP_BUILD_DLL "Dynamic dll or static" OFF)

# 检查头文件是否设置正确
if(NOT $ENV{BLACKBONE_ROOT} STREQUAL "")
    message(STATUS "BlackBone lib set" )
    set(blackbone_include_path "$ENV{BLACKBONE_ROOT}/src")
endif()
if(EXISTS ${blackbone_include_path}/BlackBone/Process/Process.h)
    message(STATUS "${blackbone_include_path}/BlackBone/Process/Process.h finded" )
else()
    message(FATAL_ERROR "${blackbone_include_path}/BlackBone/Process/Process.h not finded" )
endif()



project ("op")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    MESSAGE(STATUS "----------------Now is MSVC EHA----------------")
    if(OP_BUILD_DLL)
        message(STATUS "Dynamic DLL")
        set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /EHa")
        set(CMAKE_C_FLAGS_RELEASE "/MD  /O2 /EHa")
        set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /EHa")
        set(CMAKE_C_FLAGS_DEBUG "/MDd /Zi /EHa")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /O2  /EHa")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MD /O2 /EHa")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /EHa")
        set(CMAKE_C_FLAGS_RELEASE "/MT  /O2 /EHa")
        set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /EHa")
        set(CMAKE_C_FLAGS_DEBUG "/MTd /Zi /EHa")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /O2  /EHa")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MT /O2 /EHa")
    endif()
endif()

message(STATUS ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS ${CMAKE_CXX_FLAGS_DEBUG})

# 设置 VCPKG_TARGET_TRIPLET_NAME
if(OP_BUILD_DLL)
    set(VCPKG_TARGET_TRIPLET_NAME "")
    set(BACKBONE_DIR_NAME "DLL")
else()
    set(VCPKG_TARGET_TRIPLET_NAME "-static")
    set(BACKBONE_DIR_NAME "")
endif()

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(
        ${CMAKE_SOURCE_DIR}/3rd_party/lib/x64
    )
    set(op_com op_x64)
    set(VCPKG_TARGET_TRIPLET "x64-windows${VCPKG_TARGET_TRIPLET_NAME}")
	MESSAGE(STATUS "----------------Platform ${VCPKG_TARGET_TRIPLET}----------------")
ELSE()
    link_directories(
        ${CMAKE_SOURCE_DIR}/3rd_party/lib/x86
        )
    set(op_com op_x86)
    set(VCPKG_TARGET_TRIPLET "x86-windows${VCPKG_TARGET_TRIPLET_NAME}")
	MESSAGE(STATUS "----------------Platform ${VCPKG_TARGET_TRIPLET}----------------")
ENDIF()

include(E://ExtraProgram//vcpkg//scripts//buildsystems//vcpkg.cmake)

# 找到并添加Blackbone库文件
find_library(BLACKBONE_LIB
    NAMES BlackBone
    PATHS $ENV{BLACKBONE_ROOT}/build/X64/Release${BACKBONE_DIR_NAME}
)
find_library(BLACKBONE_LIB_D
    NAMES BlackBone
    PATHS $ENV{BLACKBONE_ROOT}/build/X64/Debug${BACKBONE_DIR_NAME}
)
MESSAGE(STATUS BLACKBONE_LIB选项：${BLACKBONE_LIB})
MESSAGE(STATUS BLACKBONE_LIB_D选项：${BLACKBONE_LIB_D})

include_directories( 
  ./
  ${CMAKE_SOURCE_DIR}/3rd_party/include/
  ${CMAKE_SOURCE_DIR}/3rd_party/include/3rd_party
  ${blackbone_include_path}
  ${blackbone_include_path}/3rd_party
) 

# 包含子项目。
## libop主项目
add_subdirectory ("libop")
## tools项目
##add_subdirectory("tools")
## 测试项目
##add_subdirectory("tests")
## swig 项目
if(build_swig_py)
    add_subdirectory("swig")
endif()


