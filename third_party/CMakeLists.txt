cmake_minimum_required(VERSION 3.10)

# Forzar macros de compatibilidad para Vita en third_party
add_definitions(-D__UNIX_LIKE__ -D__vita__ -D__PSV__ -Dstrtok_r=strtok_r_vita -Dextern=)


# Priorizar src para que endian.h personalizado se use en todo el proyecto
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/third_party/ZeroTierOne/include)


# Core de ZeroTierOne
add_subdirectory(ZeroTierOne)
set(ZEROTIERONE_CORE_LIB zerotiercore)
set(ZEROTIERONE_CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ZeroTierOne/include)

# Puedes agregar otros módulos de third_party aquí
# add_subdirectory(mdnsniff)
