configure_file(tfconfig.h.in tfconfig.h)
configure_file(tfdefs.h.in tfdefs.h)
# Setting the include directory for the application to find tfconfig.h
target_include_directories(tf PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_sources(tf
    PRIVATE
        attr.c
        command.c
        dstring.c
        expand.c
        expr.c
        help.c
        history.c
        keyboard.c
        macro.c
        main.c
        malloc.c
        output.c
        pattern.c
        process.c
        search.c
        signals.c
        socket.c
        tfio.c
        tty.c
        util.c
        variable.c
        world.c
)
