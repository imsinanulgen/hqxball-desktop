cmake_minimum_required(VERSION 3.19)
project(hqxball_client)

set(CMAKE_CXX_STANDARD 20)

# Point to the downloaded CEF binary directory
set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cef")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

find_package(CEF REQUIRED)

add_subdirectory(${CEF_ROOT}/libcef_dll libcef_dll_wrapper)

add_executable(hqxball_client
    src/main.cpp
)

target_link_libraries(hqxball_client PRIVATE libcef_dll_wrapper ${CEF_STANDARD_LIBS} ${CEF_LIB_RELEASE})
target_compile_definitions(hqxball_client PRIVATE ${CEF_COMPILER_DEFINES})
target_compile_options(hqxball_client PRIVATE ${CEF_COMPILER_FLAGS})
target_include_directories(hqxball_client PRIVATE ${CEF_ROOT})

# Copy CEF files to the executable directory
add_custom_command(
    TARGET hqxball_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_ROOT}/Release/"
        "$<TARGET_FILE_DIR:hqxball_client>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_ROOT}/Resources/"
        "$<TARGET_FILE_DIR:hqxball_client>"
)
