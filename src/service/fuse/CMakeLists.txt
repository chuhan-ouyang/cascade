cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

find_package(derecho CONFIG REQUIRED)

if (${HAS_FUSE})
    add_library(fuse_client_signals OBJECT fuse_client_signals.cpp)
    target_include_directories(fuse_client_signals PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    add_executable(fuse_client_hl fuse_client_hl.cpp)
    target_include_directories(fuse_client_hl PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    )
    target_link_libraries(fuse_client_hl cascade readline fuse3 fuse_client_signals)
    set_target_properties(fuse_client_hl PROPERTIES OUTPUT_NAME cascade_fuse_client_hl)

    # add_executable(fuse_client fuse_client.cpp)
    # target_include_directories(fuse_client PRIVATE
    #     $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    #     $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    #     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    #     $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    # )
    # target_link_libraries(fuse_client cascade readline fuse3 fuse_client_signals)
    # set_target_properties(fuse_client PROPERTIES OUTPUT_NAME cascade_fuse_client)

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
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/fuse_put.py
        ${CMAKE_CURRENT_BINARY_DIR}/fuse_cfg/n4/fuse_put.py
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fuse_cfg/run.sh.tmp
        COMMENT "prepare fuse node and client configuration"
    )
endif()
