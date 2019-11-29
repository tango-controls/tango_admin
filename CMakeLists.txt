cmake_minimum_required(VERSION 2.8.12)
project(tango_admin)

include(FindPkgConfig)

pkg_search_module(TANGO_PKG REQUIRED tango)

set(SOURCES
        tango_admin.cpp
        anyoption.cpp)

set(HEADERS
        anyoption.h)

link_directories(${TANGO_PKG_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${TANGO_PKG_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC -std=c++0x ${TANGO_PKG_CFLAGS_OTHER})
target_link_libraries(${PROJECT_NAME} PUBLIC ${TANGO_PKG_LIBRARIES} ${CMAKE_DL_LIBS})

#TODO install
