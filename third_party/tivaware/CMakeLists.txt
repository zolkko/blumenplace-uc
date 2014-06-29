cmake_minimum_required(VERSION 2.8)
project(driverlib C)

set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${COMMON_FLAGS} -Os")
set(CMAKE_C_FLAGS_DEBUG "${ARCH_FLAGS} ${COMMON_FLAGS} -g -gstrict-dwarf -Og")
set(CMAKE_STATIC_LINKER_FLAGS "")
set(CMAKE_CXX_FLAGS "")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/driverlib")

aux_source_directory(driverlib DRIVERLIB_SRC)

add_library(${PROJECT_NAME} STATIC ${DRIVERLIB_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME})

