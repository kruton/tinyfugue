function(tf_feature_mode variable default description)
    set(${variable} "${default}" CACHE STRING "${description}: AUTO, ON, or OFF")
    set_property(CACHE ${variable} PROPERTY STRINGS AUTO ON OFF)
    string(TOUPPER "${${variable}}" normalized)
    if(NOT normalized MATCHES "^(AUTO|ON|OFF)$")
        message(FATAL_ERROR
            "${variable} must be AUTO, ON, or OFF (got '${${variable}}').")
    endif()
    set(${variable} "${normalized}" CACHE STRING "${description}: AUTO, ON, or OFF" FORCE)
endfunction()

function(tf_require_found mode_variable found_variable dependency)
    if(${mode_variable} STREQUAL "ON" AND NOT ${found_variable})
        message(FATAL_ERROR
            "${dependency} is required because ${mode_variable}=ON.")
    endif()
endfunction()
