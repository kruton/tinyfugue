find_program(NROFF_EXECUTABLE NAMES nroff
    DOC "nroff man page converter")

set(TF_DOCS_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")

add_custom_target(tf-man
    mkdir -p ${TF_DOCS_DIR}/man/man1
    COMMAND env TERM=vt100
        ${NROFF_EXECUTABLE} -man "${CMAKE_CURRENT_SOURCE_DIR}/src/tf.1.nroffman" > ${TF_DOCS_DIR}/man/man1/tf.1
    COMMENT "Building man page with nroff")

if (MANPAGE)
    add_custom_target(doc ALL DEPENDS tf-man)
endif()
