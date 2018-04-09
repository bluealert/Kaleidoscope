cmake_minimum_required(VERSION 3.0.2)
project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(Gflags REQUIRED)
find_package(LLVM 5.0 REQUIRED
        COMPONENTS Core ExecutionEngine Object Support native)

include_directories(${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/include
        ${GFLAGS_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

set(KASO_HEADER_DIRS src/include src/global src/lexer src/parser)
foreach(dir ${KASO_HEADER_DIRS})
    file(GLOB_RECURSE headers ${dir}/*.h)
    set(KASO_HEADERS ${KASO_HEADERS} ${headers})
endforeach()

set(KASO_SOURCE_DIRS src/global src/lexer src/parser)
foreach(dir ${KASO_SOURCE_DIRS})
    file(GLOB_RECURSE headers ${dir}/*.cpp)
    set(KASO_SOURCES ${KASO_SOURCES} ${headers})
endforeach()

add_library(kaso ${KASO_HEADERS} ${KASO_SOURCES})
target_link_libraries(kaso z ncurses ${GFLAGS_LIBRARIES} ${LLVM_LIBRARIES})

file(GLOB_RECURSE SHELL_FILES src/shell/*.h src/shell/*.cpp)
add_executable(kaso-shell ${SHELL_FILES})
target_link_libraries(kaso-shell kaso)

option(BUILD_TESTS "BUILD_TESTS" ON)
if(BUILD_TESTS)
    enable_testing()

    include(ExternalProject)

    set(GTEST_PREFIX gtest1.8.0)
    set(GTEST_URL ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest-1.8.0.tar.gz)

    ExternalProject_Add(${GTEST_PREFIX}
            PREFIX ${GTEST_PREFIX}
            URL ${GTEST_URL}
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND ""
            LOG_DOWNLOAD 1
            LOG_BUILD 1)

    add_library(libgmock IMPORTED STATIC GLOBAL)
    add_dependencies(libgmock ${GTEST_PREFIX})
    add_library(libgmock_main IMPORTED STATIC GLOBAL)
    add_dependencies(libgmock_main ${GTEST_PREFIX})

    ExternalProject_Get_Property(${GTEST_PREFIX} source_dir binary_dir)

    set_target_properties(libgmock PROPERTIES
            "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
            "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
    set_target_properties(libgmock_main PROPERTIES
            "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock_main.a"
            "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")

    include_directories("${source_dir}/googlemock/include")
    include_directories("${source_dir}/googletest/include")

    macro(add_gtest test_name test_source)
        message(STATUS "add_gtest: ${test_name}, ${test_source}")
        add_executable(${test_name} ${test_source} test/TestMain.cpp)
        target_link_libraries(${test_name} kaso libgmock libgmock_main)
        add_test(${test_name} bin/${test_name})
    endmacro(add_gtest)

    file(GLOB test_files test/*Test.cpp)
    foreach(file_path ${test_files})
        get_filename_component(file_name "${file_path}" NAME)
        string(REGEX MATCH ".*/(.*Test).cpp" _ ${file_path})
        add_gtest(${CMAKE_MATCH_1} test/${file_name})
    endforeach()
endif()
