## This file is a quick tutorial on writing CMakeLists for targeting the Vita
cmake_minimum_required(VERSION 3.16)

## This includes the Vita toolchain, must go before project definition
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

## Define project parameters here
project(ZeroTierOneVitaTest)
include("${VITASDK}/share/vita.cmake" REQUIRED)

## Configuration options for this app
set(VITA_APP_NAME "ZeroTierOne Vita Test")
set(VITA_TITLEID  "ZT1VITA01") # Debe ser único y de 9 caracteres
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

include_directories(
    third_party/ZeroTierOne/include
    third_party/samples/common
    src
)
add_subdirectory(third_party)

link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ping.cpp
    src/ime.cpp
    src/zerotierone.cpp
    src/zt_storage.cpp
    src/common/gxm_helper.cpp
    third_party/samples/common/debugScreen.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
)

# Forzar macros de compatibilidad para Vita
add_definitions(-D__UNIX_LIKE__ -D__vita__ -D__PSV__)
target_link_libraries(${PROJECT_NAME}
    SceLibKernel_stub
    SceDisplay_stub
    SceCtrl_stub
    SceNetCtl_stub
    SceCommonDialog_stub
    SceAppUtil_stub
    SceGxm_stub
    SceSysmodule_stub
    ${ZEROTIERONE_CORE_LIB}
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)

