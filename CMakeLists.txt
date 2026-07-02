cmake_minimum_required(VERSION 3.11...3.13)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

include(FetchContent)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    macro(FetchContent_MakeAvailable NAME)
        FetchContent_GetProperties(${NAME})
        if(NOT ${NAME}_POPULATED)
            FetchContent_Populate(${NAME})
            add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})
        endif()
    endmacro()
endif()

FetchContent_Declare(
    concurrentqueue
    GIT_REPOSITORY  https://github.com/cameron314/readerwriterqueue.git
    GIT_TAG         v1.0.6
)
FetchContent_MakeAvailable(concurrentqueue)
include_directories(${concurrentqueue_SOURCE_DIR})

project(sqlite_web_vfs VERSION 1.0
        DESCRIPTION "SQLite3 extension for read-only HTTP(S) database access"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_definitions(-DHTTP_LAZYCURL)
    add_library(web_vfs SHARED src/web_vfs.cc src/web_vfs.h src/SQLiteVFS.h src/ThreadPool.h src/HTTP.h)
    SET_TARGET_PROPERTIES(web_vfs PROPERTIES PREFIX "")
    target_link_libraries(web_vfs PRIVATE pthread dl)  # EXCLUDE curl given HTTP_LAZYCURL -- see src/HTTP.h
    if(APPLE)
        # sqlite3 symbols are provided by the host process loading the extension at runtime;
        # macOS/arm64 requires explicitly allowing undefined symbols.
        target_link_options(web_vfs PRIVATE -undefined dynamic_lookup)
    endif()

    FetchContent_Declare(
        catch
        GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
        GIT_TAG         v2.13.8
    )
    FetchContent_MakeAvailable(catch)
    include_directories(${catch_SOURCE_DIR}/single_include)
    find_path(MICROHTTPD_INCLUDE_DIR microhttpd.h)
    find_library(MICROHTTPD_LIBRARY microhttpd)
    add_executable(local_stress_test test/local_stress_test.cc test/test_httpd.cc test/test_httpd.h)
    if(MICROHTTPD_INCLUDE_DIR)
        target_include_directories(local_stress_test PRIVATE ${MICROHTTPD_INCLUDE_DIR})
    endif()
    target_link_libraries(local_stress_test PRIVATE sqlite3 curl ${MICROHTTPD_LIBRARY} pthread dl)

    include(CTest)
    enable_testing()
    add_test(NAME pytest COMMAND env SQLITE_VFS_LOG=99 python3 -m pytest -sv ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_test(NAME local_stress_test COMMAND local_stress_test)
endif()
