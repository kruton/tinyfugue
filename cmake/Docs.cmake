find_program(NROFF_EXECUTABLE NAMES nroff
    DOC "nroff man page converter")

set(TF_DOCS_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")

add_custom_target(tf-man
    mkdir -p ${TF_DOCS_DIR}
    COMMAND env TERM=vt100
        ${NROFF_EXECUTABLE} -man "${CMAKE_CURRENT_SOURCE_DIR}/src/tf.1.nroffman" > ${TF_DOCS_DIR}/tf.1.catman
    COMMENT "Building man page with nroff")

if (MANPAGE)
    add_custom_target(doc ALL DEPENDS tf-man)
endif()
