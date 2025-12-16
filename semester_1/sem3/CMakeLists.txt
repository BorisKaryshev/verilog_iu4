cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 20)

set(project_name verilator)

project(${project_name})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(
    top_module
    main
)
set(source
    ${PROJECT_SOURCE_DIR}/src/main.v
    ${PROJECT_SOURCE_DIR}/src/multiplier.v
    ${PROJECT_SOURCE_DIR}/src/constants.v
    ${PROJECT_SOURCE_DIR}/src/solver.v
    ${PROJECT_SOURCE_DIR}/src/naive_solver.v
)

set(tb_source
    ${PROJECT_SOURCE_DIR}/test_benches/sample.cpp
)


if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(libs
        /usr/share/verilator/include/verilated.cpp
        /usr/share/verilator/include/verilated_vcd_c.cpp
        /usr/share/verilator/include/verilated_threads.cpp
    )
    set(include_locations
        ${PROJECT_BINARY_DIR}/obj_dir
        /usr/share/verilator/include
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(libs
        /usr/local/Cellar/verilator/5.038/share/verilator/include/verilated.cpp
        /usr/local/Cellar/verilator/5.038/share/verilator/include/verilated_vcd_c.cpp
        /usr/local/Cellar/verilator/5.038/share/verilator/include/verilated_threads.cpp
    )
    set(include_locations
        ${PROJECT_BINARY_DIR}/obj_dir
        /usr/local/Cellar/verilator/5.038/share/verilator/include/
    )
    set(tb_source
        ${tb_source}
        ${PROJECT_SOURCE_DIR}/test_benches/sc_time_stamp.cpp
    )
endif()

add_library(${project_name}_lib STATIC ${tb_source} ${libs})
target_include_directories(${project_name}_lib PUBLIC ${include_locations})

add_executable(exe ${PROJECT_SOURCE_DIR}/test_benches/main.cpp)
target_link_libraries(exe ${project_name}_lib)

add_subdirectory(${PROJECT_SOURCE_DIR}/libs/googletest)
target_link_libraries(exe gtest_main)

target_link_libraries(exe "${CMAKE_BINARY_DIR}/from_verilog.a")

target_compile_options(exe PRIVATE -O3)
target_compile_options(${project_name}_lib PRIVATE -O3)
target_compile_options(gtest_main PRIVATE -O3)

find_program(VERILATOR "verilator")
add_custom_target(
    VERILATOR ALL
    COMMAND verilator
    -Wall --cc --trace --build
    --top ${top_module}
    -I${PROJECT_SOURCE_DIR}/src
    ${source} && bash ${PROJECT_SOURCE_DIR}/link_all_in_one.sh ${CMAKE_BINARY_DIR}
)
add_dependencies(${project_name}_lib VERILATOR)
