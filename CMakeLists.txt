cmake_minimum_required(VERSION 3.11)
project(p4io_mdxfdrv VERSION 0.1 LANGUAGES C)

if ( TARGET_WIN )
    set(CMAKE_SYSTEM_NAME Windows)

    set(toolchain i686-w64-mingw32)
    set(CMAKE_SYSROOT /usr/${toolchain})
    set(CMAKE_C_COMPILER ${toolchain}-gcc)

    add_compile_definitions(WIN32_LEAN_AND_MEAN
                            WINVER=0x0601
                            _WIN32_WINNT=0x0601
                            COBJMACROS
                          # AC_IO_MSG_LOG
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(LIBEVDEV REQUIRED libevdev)

    add_library(windows_stub STATIC src/drv/stub/windows.c)
    link_libraries(windows_stub)
    include_directories(src/drv/stub)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

if ( DEBUG_BUILD )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    add_compile_definitions(
		# AC_IO_MSG_LOG
		P4IO_DEBUG_BUILD
	)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

add_subdirectory(extern/libusb-cmake)

include_directories(bemanitools/src/main)

if ( NOT TARGET_WIN )
    add_library(aciodrv_port STATIC src/drv/port_linux.c src/drv/termios_util.c)
    add_library(util STATIC src/util/log_linux.c
                            src/util/util.c
							bemanitools/src/main/util/hex.c
    )
    add_library(p4io_thread STATIC src/thread/thread_linux.c)
else()
    add_library(aciodrv_port STATIC bemanitools/src/main/aciodrv/port.c)
    add_library(util STATIC bemanitools/src/main/util/log.c
                            bemanitools/src/main/util/mem.c
                            bemanitools/src/main/util/str.c
							bemanitools/src/main/util/hex.c
    )
    add_library(p4io_thread STATIC src/thread/thread_win.c)
endif()

add_library(aciodrv STATIC bemanitools/src/main/aciodrv/device.c)
target_link_libraries(aciodrv aciodrv_port util)

add_executable(mdxftest src/main.c src/mdxf.c)
target_link_libraries(mdxftest aciodrv p4io_thread)

add_library(p4io STATIC src/p4io.c)
add_executable(p4iotest src/p4iotest.c)
target_link_libraries(p4io usb-1.0)
target_link_libraries(p4iotest p4io)

if ( NOT TARGET_WIN )
    add_executable(mdxfuinput src/uinput.c src/mdxf.c)
    target_link_libraries(mdxfuinput aciodrv p4io evdev ${LIBEVDEV_LIBRARIES})
    target_include_directories(mdxfuinput PRIVATE ${LIBEVDEV_INCLUDE_DIRS})
endif()

