cmake_minimum_required(VERSION 3.12)

project("wireless_box_receive")

set(SOURCE_ROOT ${CMAKE_SOURCE_DIR})

set(CHIP 650V200)

set(CMAKE_C_COMPILER   arm-xm-linux-gcc)
set(CMAKE_CXX_COMPILER arm-xm-linux-g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DARCHBOARD -DBOARD650V200")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 \
-mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=neon -mno-unaligned-access \
-fno-aggressive-loop-optimizations -flax-vector-conversions -fsigned-char \
-fopenmp -g -lstdc++ -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu99")

set(CMAKE_C_FLAGS_RELEASE   "-g -funwind-tables -rdynamic -Os -Wall -s -w")
set(CMAKE_CXX_FLAGS_RELEASE "-g -funwind-tables -rdynamic -Os -Wall -s -w")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -g -fno-omit-frame-pointer -fstack-protector-strong -funwind-tables -rdynamic -Wall")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -fno-omit-frame-pointer -fstack-protector-strong -funwind-tables -rdynamic -Wall")

if(${_WIFI_TYPE} EQUAL 2)
    add_definitions(-DWIFI_TYPE_6162=1)
elseif(${_WIFI_TYPE} EQUAL 1)
    add_definitions(-DWIFI_TYPE_6062C=1)
elseif(${_WIFI_TYPE} EQUAL 0)
    add_definitions(-DWIFI_TYPE_6062S=1)
endif()

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT COMMIT_HASH)
    set(COMMIT_HASH "unknown")
endif()
add_definitions(-DCOMMIT_HASH="${COMMIT_HASH}")

include_directories(${SOURCE_ROOT}/include)
include_directories(${SOURCE_ROOT}/src)
include_directories(${SOURCE_ROOT}/sdk/include)
include_directories(${SOURCE_ROOT}/sdk/include/${CHIP}/mpp)
include_directories(${SOURCE_ROOT}/sdk/include/${CHIP}/smart)
include_directories(${SOURCE_ROOT}/sdk/include/${CHIP}/fvideo)
include_directories(${SOURCE_ROOT}/sdk/include/${CHIP}/comm)
include_directories(${SOURCE_ROOT}/sdk/include/${CHIP}/isp)

file(GLOB_RECURSE RECV_SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
message(STATUS "==== 收集到的源文件 ====")
foreach(f ${RECV_SOURCE_FILES})
    message(STATUS "  ${f}")
endforeach()

link_directories(${SOURCE_ROOT}/sdk/lib/${CHIP})
link_directories(${SOURCE_ROOT}/sdk/lib_extra)

add_executable(wireless_box_receive ${RECV_SOURCE_FILES})

target_link_libraries(wireless_box_receive
-Wl,--start-group
xmipprotocol
XM_MiddleWare_api
h265d
h265e
ive
mpi_all
rtc
x264_weak
venc
vdec
xmaudio3a
zzip
xmsdk
avformat
avcodec
swscale
swresample
avutil
lvgl
z
pthread
m
dl
jpegdec
jpegenc
crypt
optcrypt
qr
mbedcrypto
433
frehop
sda
cr
xmia_dec
unwind
unwind-arm
-Wl,--end-group
)