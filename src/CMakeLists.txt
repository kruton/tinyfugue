configure_file(tfconfig.h.in tfconfig.h)
configure_file(tfdefs.h.in tfdefs.h)

# Setting the include directory for the application to find tfconfig.h
target_include_directories(tflib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(tf PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(tflib
    PRIVATE
        ${src_dir}/attr.c
        ${src_dir}/command.c
        ${src_dir}/dstring.c
        ${src_dir}/expand.c
        ${src_dir}/expr.c
        ${src_dir}/globals.c
        ${src_dir}/help.c
        ${src_dir}/history.c
        ${src_dir}/keyboard.c
        ${src_dir}/macro.c
        ${src_dir}/malloc.c
        ${src_dir}/output.c
        ${src_dir}/pattern.c
        ${src_dir}/process.c
        ${src_dir}/search.c
        ${src_dir}/signals.c
        ${src_dir}/socket.c
        ${src_dir}/tfio.c
        ${src_dir}/tty.c
        ${src_dir}/util.c
        ${src_dir}/variable.c
        ${src_dir}/world.c
)

target_sources(tf
    PRIVATE
	${src_dir}/main.c
)

# Prepare GTest library for tests
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    tf_test
    attr_test.cc
)
target_include_directories(tf_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
    tf_test
    tflib
    GTest::gtest_main
)
