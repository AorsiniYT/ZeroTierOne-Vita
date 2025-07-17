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

link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)


# Buscar automáticamente todos los archivos .c y .cpp en src/


# Incluir librerías de terceros
add_subdirectory(third_party)


# Buscar todos los archivos .c y .cpp en src/, pero excluir debugScreenFont.c
file(GLOB_RECURSE SRC_FILES
    src/*.c
    src/*.cpp
)
list(FILTER SRC_FILES EXCLUDE REGEX ".*debugScreenFont\\.c$")

# Añadir explícitamente ripemd160.c si no es capturado por GLOB
list(APPEND SRC_FILES src/zerotierlite/ripemd160.c)

add_executable(${PROJECT_NAME} ${SRC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
)

# Forzar macros de compatibilidad para Vita
add_definitions(-D__vita__ -D__PSV__)

# Enlazar librerías de Vita y de terceros
target_link_libraries(${PROJECT_NAME}
    SceLibKernel_stub
    SceDisplay_stub
    SceCtrl_stub
    SceNetCtl_stub
    SceCommonDialog_stub
    SceAppUtil_stub
    SceGxm_stub
    SceSysmodule_stub
    ed25519-donna
    tweetnacl
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)

