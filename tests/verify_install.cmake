file(REMOVE_RECURSE "${STAGE_DIR}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env --unset=DESTDIR
        "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix "${STAGE_DIR}"
    RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Staged installation failed with status ${install_result}")
endif()

set(executable "${STAGE_DIR}/${BINDIR}/${EXENAME}${EXE_SUFFIX}")
set(library_dir "${STAGE_DIR}/${DATADIR}/${LIBNAME}")
foreach(required
        "${executable}"
        "${library_dir}/tf-help"
        "${library_dir}/tf-help.idx"
        "${library_dir}/stdlib.tf"
        "${library_dir}/CHANGES"
        "${STAGE_DIR}/${MANDIR}/man1/${EXENAME}.1")
    if(NOT EXISTS "${required}")
        message(FATAL_ERROR "Missing installed file: ${required}")
    endif()
endforeach()

foreach(link
        bind-bash.tf bind-emacs.tf completion.tf factorial.tf file-xfer.tf
        local.tf.sample pref-shell.tf space_page.tf speedwalk.tf
        stack_queue.tf worldqueue.tf)
    if(NOT IS_SYMLINK "${library_dir}/${link}")
        message(FATAL_ERROR "Missing compatibility symlink: ${link}")
    endif()
endforeach()

if(EXPECT_EXECUTABLE_SYMLINK)
    if(NOT IS_SYMLINK "${STAGE_DIR}/${BINDIR}/tf${EXE_SUFFIX}")
        message(FATAL_ERROR "Missing unversioned executable symlink")
    endif()
endif()

set(local_file "${library_dir}/local.tf")
file(WRITE "${local_file}" "site-local configuration\n")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env --unset=DESTDIR
        "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix "${STAGE_DIR}"
    RESULT_VARIABLE reinstall_result
    OUTPUT_QUIET)
if(NOT reinstall_result EQUAL 0)
    message(FATAL_ERROR "Reinstallation failed with status ${reinstall_result}")
endif()
file(READ "${local_file}" local_contents)
if(NOT local_contents STREQUAL "site-local configuration\n")
    message(FATAL_ERROR "Reinstallation overwrote the site-local library file")
endif()

if(NOT SKIP_EXECUTABLE_SMOKE)
    execute_process(
        COMMAND "${executable}" "-?"
        RESULT_VARIABLE smoke_result
        OUTPUT_VARIABLE smoke_output
        ERROR_VARIABLE smoke_error
        TIMEOUT 10)
    if(NOT smoke_result EQUAL 1)
        message(FATAL_ERROR
            "Installed executable usage check returned ${smoke_result}: ${smoke_error}")
    endif()
    string(CONCAT smoke_text "${smoke_output}" "${smoke_error}")
    if(NOT smoke_text MATCHES "Usage:")
        message(FATAL_ERROR
            "Installed executable did not print usage information: ${smoke_text}")
    endif()
endif()

set(destdir_stage "${BUILD_DIR}/test-destdir")
file(REMOVE_RECURSE "${destdir_stage}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env "DESTDIR=${destdir_stage}"
        "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix /usr/local
    RESULT_VARIABLE destdir_result
    OUTPUT_QUIET)
if(NOT destdir_result EQUAL 0)
    message(FATAL_ERROR "DESTDIR installation failed with status ${destdir_result}")
endif()
if(NOT EXISTS
        "${destdir_stage}/usr/local/${BINDIR}/${EXENAME}${EXE_SUFFIX}")
    message(FATAL_ERROR "DESTDIR installation did not contain the executable")
endif()
