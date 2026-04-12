cmake_minimum_required(VERSION 3.16)

# Set the project name
project(pico-sst39sf0x0-programmer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

# get Git HASH
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(GIT_HASH="${GIT_HASH}")

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Widgets SerialPort Network Test)

add_library(pico_flasher_core STATIC
    src/cartridgereadthread.cpp
    src/flashthread.cpp
    src/ioworker.cpp
    src/readthread.cpp
    src/serial_interface.cpp
    src/serial_transport_qt.cpp
)

target_include_directories(pico_flasher_core PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(pico_flasher_core PUBLIC Qt5::Core Qt5::SerialPort)

add_library(pico_flasher_ui STATIC
    src/bankviewer.cpp
    src/dialogslotselection.cpp
    src/filedownloader.cpp
    src/hexviewwidget.cpp
    src/logwindow.cpp
    src/mainwindow.cpp
    src/picoflasherapplication.cpp
    src/settingswidget.cpp
    resources.qrc
)

target_include_directories(pico_flasher_ui PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(pico_flasher_ui PUBLIC pico_flasher_core Qt5::Widgets Qt5::Network)

add_executable(pico-sst39sf0x0-programmer
    src/main.cpp
)

if(WIN32)
    target_sources(pico-sst39sf0x0-programmer PRIVATE
        appicon.rc
    )
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    set_target_properties(pico-sst39sf0x0-programmer PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

target_link_libraries(pico-sst39sf0x0-programmer PRIVATE pico_flasher_ui)

set_target_properties(pico-sst39sf0x0-programmer PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if(BUILD_TESTING)
    add_executable(tst_serial_interface
        tests/tst_serial_interface.cpp
        tests/support/emulated_serial_transport.cpp
        tests/support/fault_injecting_transport.cpp
    )
    target_include_directories(tst_serial_interface PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/tests
    )
    target_link_libraries(tst_serial_interface PRIVATE pico_flasher_core Qt5::Test)
    add_test(NAME tst_serial_interface COMMAND tst_serial_interface)

    add_executable(tst_worker_threads
        tests/tst_worker_threads.cpp
        tests/support/emulated_serial_transport.cpp
    )
    target_include_directories(tst_worker_threads PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/tests
    )
    target_link_libraries(tst_worker_threads PRIVATE pico_flasher_core Qt5::Test)
    add_test(NAME tst_worker_threads COMMAND tst_worker_threads)

    add_executable(tst_mainwindow
        tests/tst_mainwindow.cpp
        tests/support/emulated_serial_transport.cpp
        tests/support/fault_injecting_transport.cpp
    )
    target_include_directories(tst_mainwindow PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/tests
    )
    target_link_libraries(tst_mainwindow PRIVATE pico_flasher_ui Qt5::Test)
    add_test(NAME tst_mainwindow COMMAND tst_mainwindow)
endif()
