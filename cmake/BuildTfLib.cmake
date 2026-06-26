file(MAKE_DIRECTORY "${OUTPUT_DIR}")

file(GLOB source_files LIST_DIRECTORIES false "${SOURCE_DIR}/*")
foreach(source_file IN LISTS source_files)
    get_filename_component(file_name "${source_file}" NAME)
    configure_file("${source_file}" "${OUTPUT_DIR}/${file_name}" COPYONLY)
endforeach()

if(DEFINED CHANGES_FILE)
    configure_file("${CHANGES_FILE}" "${OUTPUT_DIR}/CHANGES" COPYONLY)
endif()

file(GLOB command_files LIST_DIRECTORIES false
    "${HELP_SOURCE}/commands/*.html")
file(GLOB topic_files LIST_DIRECTORIES false
    "${HELP_SOURCE}/topics/*.html")
list(SORT command_files)
list(SORT topic_files)

set(command_help "${OUTPUT_DIR}/tf-help.commands")
set(topic_help "${OUTPUT_DIR}/tf-help.topics")
execute_process(
    COMMAND ${HTML2TF} ${command_files}
    OUTPUT_FILE "${command_help}"
    RESULT_VARIABLE html2tf_commands_result)
if(NOT html2tf_commands_result EQUAL 0)
    message(FATAL_ERROR
        "html2tf failed for command help with status ${html2tf_commands_result}")
endif()

execute_process(
    COMMAND ${HTML2TF} ${topic_files}
    OUTPUT_FILE "${topic_help}"
    RESULT_VARIABLE html2tf_topics_result)
if(NOT html2tf_topics_result EQUAL 0)
    message(FATAL_ERROR
        "html2tf failed for topic help with status ${html2tf_topics_result}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E cat "${command_help}" "${topic_help}"
    OUTPUT_FILE "${OUTPUT_DIR}/tf-help"
    RESULT_VARIABLE concatenate_result)
file(REMOVE "${command_help}" "${topic_help}")
if(NOT concatenate_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to concatenate tf-help with status ${concatenate_result}")
endif()

execute_process(
    COMMAND ${MAKEHELP}
    INPUT_FILE "${OUTPUT_DIR}/tf-help"
    OUTPUT_FILE "${OUTPUT_DIR}/tf-help.idx"
    RESULT_VARIABLE makehelp_result)
if(NOT makehelp_result EQUAL 0)
    message(FATAL_ERROR "makehelp failed with status ${makehelp_result}")
endif()

set(compat_links
    "bind-bash.tf|kb-bash.tf"
    "bind-emacs.tf|kb-emacs.tf"
    "completion.tf|complete.tf"
    "factorial.tf|factoral.tf"
    "file-xfer.tf|filexfer.tf"
    "local.tf.sample|local-eg.tf"
    "pref-shell.tf|psh.tf"
    "space_page.tf|spc-page.tf"
    "speedwalk.tf|spedwalk.tf"
    "stack_queue.tf|stack-q.tf"
    "worldqueue.tf|world-q.tf")

foreach(link_spec IN LISTS compat_links)
    string(REPLACE "|" ";" link_parts "${link_spec}")
    list(GET link_parts 0 link_name)
    list(GET link_parts 1 link_target)
    file(REMOVE "${OUTPUT_DIR}/${link_name}")
    file(CREATE_LINK "${link_target}" "${OUTPUT_DIR}/${link_name}" SYMBOLIC)
endforeach()
