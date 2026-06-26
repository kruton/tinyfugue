if(NOT DEFINED BUILD_DIR)
    message(FATAL_ERROR "BUILD_DIR is required")
endif()
if(NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OUTPUT is required")
endif()

set(dist_dir "${BUILD_DIR}/tinyfugue-wasm")
file(REMOVE_RECURSE "${dist_dir}")
file(MAKE_DIRECTORY "${dist_dir}")

file(COPY
    "${BUILD_DIR}/tf.js"
    "${BUILD_DIR}/tf.wasm"
    "${BUILD_DIR}/tf-lib"
    DESTINATION "${dist_dir}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar cfz "${OUTPUT}" tinyfugue-wasm
    WORKING_DIRECTORY "${BUILD_DIR}"
    RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to create ${OUTPUT}")
endif()
