cmake_minimum_required(VERSION 3.10.0)
project(subCascade LANGUAGES C CXX)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(derecho CONFIG REQUIRED)

if (${HAS_FUSE})
    add_executable(fuse_client_hl fuse_client_hl.cpp)
    add_executable(fuse_perftest_put fuse_perftest_put.cpp)
    add_library(link_malloc_obj OBJECT link_malloc.c)
    target_compile_definitions(link_malloc_obj PRIVATE LINKTIME)

    target_include_directories(fuse_client_hl PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    )
    
    target_include_directories(fuse_perftest_put PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    )

    # target_link_libraries(fuse_client_hl $<TARGET_OBJECTS:link_malloc_obj> cascade readline fuse3)
    target_link_libraries(fuse_client_hl cascade readline fuse3)
    target_link_libraries(fuse_perftest_put cascade readline fuse3)

    set_target_properties(fuse_client_hl PROPERTIES OUTPUT_NAME cascade_fuse_client_hl)

    # Link Interposition
    # target_link_options(fuse_client_hl PRIVATE "-Wl,--wrap,malloc")
    # target_link_options(fuse_client_hl PRIVATE "-Wl,--wrap,free")

    # copy testfuse.py and util.py to n4
    add_custom_command(TARGET fuse_client_hl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/fuse_cfg
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/run.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n0/run.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/run.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n1/run.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/run.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n2/run.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/run.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n3/run.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/clear_log.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n0/clear_log.sh
	    COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/clear_log.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n1/clear_log.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/clear_log.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n2/clear_log.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/clear_log.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n3/clear_log.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/clear_log.sh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/clear_log.sh
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/cascade_fuse_client_hl
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/cascade_fuse_client_hl

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_perftest
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/fuse_perftest

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/fuse_put.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/fuse_put.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/test_write.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/test_write.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_perftest.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/fuse_perftest.cpp

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/testfuse_hl.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/testfuse_hl.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/util.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/util.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/util.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n5/util.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/mount.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/mount.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/read.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/read.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/opdir.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/opdir.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/write.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/write.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/snapshot.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n5/snapshot.py

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/fuse_run.sh
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/fuse_run.sh

        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/umount.sh
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/umount.sh

        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fuse_cfg/run.sh.tmp
        COMMENT "prepare fuse node and client configuration"
    )

endif()
