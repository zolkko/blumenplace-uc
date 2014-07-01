cmake_minimum_required(VERSION 2.8)
project(blumenplace C CXX)


set(THIRD_PARTY "${PROJECT_SOURCE_DIR}/third_party")
set(FREERTOS_PATH "${THIRD_PARTY}/freertos")
set(TIVAWARE_PATH "${THIRD_PARTY}/tivaware")

add_subdirectory("${TIVAWARE_PATH}")

set(CPP_FLAGS "-std=gnu++11 -fno-rtti -fno-exceptions")
set(DEBUG_FLAGS "-g -gstrict-dwarf -Og")

set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${COMMON_FLAGS} -Os")
set(CMAKE_C_FLAGS_DEBUG "${ARCH_FLAGS} ${COMMON_FLAGS} ${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${COMMON_FLAGS} ${CPP_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${ARCH_FLAGS} ${COMMON_FLAGS} ${CPP_FLAGS} ${DEBUG_FLAGS}")
set(CMAKE_C_LINK_FLAGS "-specs=\"nano.specs\" -Wl,-Map,\"${PROJECT_NAME}.map\" -Wl,-T\"${PROJECT_SOURCE_DIR}/tm4c123ge6pm.lds\"")

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${FREERTOS_PATH}/include")
include_directories("${FREERTOS_PATH}/portable/GCC/ARM_CM4F")
include_directories("${TIVAWARE_PATH}")

aux_source_directory("src" APP_SRC)
list(REMOVE_ITEM APP_SRC "src/tm4c123ge6pm_startup_ccs.c")
aux_source_directory("${FREERTOS_PATH}" FREERTOS_SRC)
aux_source_directory("${FREERTOS_PATH}/portable/GCC/ARM_CM4F" FREERTOS_PORT_SRC)
set(FREERTOS_MEM_SRC "${FREERTOS_PATH}/portable/MemMang/heap_2.c")

add_executable(${PROJECT_NAME}.elf ${APP_SRC} ${FREERTOS_SRC} ${FREERTOS_PORT_SRC} ${FREERTOS_MEM_SRC})
target_link_libraries(${PROJECT_NAME}.elf -lc)
target_link_libraries(${PROJECT_NAME}.elf -lm)
target_link_libraries(${PROJECT_NAME}.elf -lstdc++)
target_link_libraries(${PROJECT_NAME}.elf -lgcc)
target_link_libraries(${PROJECT_NAME}.elf -lnosys)
target_link_libraries(${PROJECT_NAME}.elf driverlib)

add_custom_target(${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf"
    COMMENT "Generate hex file")

add_custom_target(${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf"
    COMMENT "Generate binary file")

