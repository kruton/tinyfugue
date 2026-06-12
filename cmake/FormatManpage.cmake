execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env TERM=vt100 "${NROFF}" -man "${INPUT}"
    OUTPUT_FILE "${OUTPUT}"
    RESULT_VARIABLE nroff_result)
if(NOT nroff_result EQUAL 0)
    message(FATAL_ERROR "nroff failed with status ${nroff_result}")
endif()
