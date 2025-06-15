set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "")
cmake_minimum_required(VERSION 4.0.1)  # 最低CMake版本要求
project(YPlayerSDK                      # 项目名称
        VERSION 1.0.0                   # 版本号
        LANGUAGES C CXX)                # 支持的语言
set(CMAKE_BUILD_TYPE Debug)             # 编译类型,指定编译器生成调试信息

# 检测平台
if (WIN32)
    set(WINDOWS TRUE)
    set(PLATFORM "WIN32")
    #set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "")
else()
    #set(CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "")
    message(">>>current platform is not windows,it is ${CMAKE_SYSTEM_NAME}")
endif()
# 检测编译器
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(COMPILER "clang")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(COMPILER "mingw")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    set(COMPILER "intel")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(COMPILER "msvc")
else()
    message(">>>unknown compiler")
endif()
message(">>>current compiler is:${COMPILER}")
# 检测编译器位数
if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(BITS "64")
else()
    set(BITS "32")
endif()
message(">>>current bits is:${BITS}")
# 检测C++版本,仅支持C++11以上版本
include(CheckCXXCompilerFlag) #加载CheckCXXCompilerFlag模块,该模块提供函数check_cxx_compiler_flag
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_STANDARD 14)
    #if(${CMAKE_VERSION_MAJOR} LESS 3.1 AND NOT ${WINDOWS})
        add_compile_options(-std=c++14)
    #endif()
elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    #if(${CMAKE_VERSION_MAJOR} LESS 3.1 AND NOT ${WINDOWS})
        add_compile_options(-std=c++11)
    #endif()
else()
    message(>>>"The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
message(">>>current C++ Standard is:${CMAKE_CXX_STANDARD}")

# 添加宏
add_definitions(-DLINUX)
add_definitions(-DCROSS_LINUX)
add_definitions(-DNO_MULTICAST)


# 引入外部依赖库
set(LIB_PATH ${PROJECT_SOURCE_DIR}/extern/lib)
find_library(AVCODEC avcodec ${LIB_PATH})
find_library(AVFORMAT avformat ${LIB_PATH})
find_library(AVUTIL avutil ${LIB_PATH})
find_library(SWRESAMPLE swresample ${LIB_PATH})
find_library(SWSCALER  swscale ${LIB_PATH})
message(">>>LibPath:${LIB_PATH}")
if(AVCODEC)
    message(">>>find:${AVCODEC}")
endif()

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -Wl,--no-undefined -lpthread")
# 其它设置
set(CMAKE_VERBOSE_MAKEFILE ON) #当设置为 ON 时，构建过程中会打印完整的编译/链接命令

# 打印包含目录
get_property(dir DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(">>>IncludeDir:${dir}")

# 自动收集dir目录下的源文件到变量variable
# aux_source_directory(<dir> <variable>)
# aux_source_directory(src/api SRC_DIR)
# 递归扫描src目录下的所有源文件
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# 创建动态库
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
# 声明依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE  ${AVCODEC} ${AVFORMAT} ${AVUTIL} ${SWRESAMPLE} ${SWSCALER})
# 添加包含目录 target_include_directories可以避免全局路径污染.
# include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(BEFORE SYSTEM ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})  # 强制系统路径优先
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/api)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/common)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/control)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/decoder)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/demuxer)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/utils)
# ffmpeg头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/extern/include)
#target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/extern/include/libavcodec)






#file(GLOB_RECURSE SRC_FILES "src/*.cpp")#  获取所有源文件
#target_include_directories(YPlayerSDK PRIVATE include)# 头文件目录
# --- 可执行文件生成 ---
# add_executable(${PROJECT_NAME} ${SRC_FILES})

# --- 跨平台处理 ---
#if(WIN32)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${FFMPEG_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    )
#endif()

# --- 安装规则 ---
#install(TARGETS ${PROJECT_NAME} DESTINATION bin)