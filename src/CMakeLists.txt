configure_file(tfconfig.h.in tfconfig.h)
configure_file(tfdefs.h.in tfdefs.h)
# Setting the include directory for the application to find tfconfig.h
target_include_directories(tf PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(tf
    PRIVATE
        ${src_dir}/attr.c
        ${src_dir}/command.c
        ${src_dir}/dstring.c
        ${src_dir}/expand.c
        ${src_dir}/expr.c
        ${src_dir}/help.c
        ${src_dir}/history.c
        ${src_dir}/keyboard.c
        ${src_dir}/macro.c
        ${src_dir}/main.c
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
