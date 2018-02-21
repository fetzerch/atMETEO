# - Utilities
# Utility functions.

# String padding
function(string_padding _STRING _LENGTH _DEST)
    string(LENGTH "${_STRING}" _STRING_LEN)
    math(EXPR _PAD_LEN "${_LENGTH} - ${_STRING_LEN}")
    if(_PAD_LEN GREATER 0)
        string(RANDOM LENGTH ${_PAD_LEN} ALPHABET " " _PAD)
    endif()
    set(${_DEST} "${_STRING}${_PAD}" PARENT_SCOPE)
endfunction()

# Print status message
function(status_message _DESC)
    set(_EXTRA_ARGS ${ARGN})
    list(LENGTH _EXTRA_ARGS _EXTRA_ARGS_LEN)
    if(${_EXTRA_ARGS_LEN} EQUAL 0)
        set(_MESSAGE "   ${_DESC}")
    elseif(${_EXTRA_ARGS_LEN} EQUAL 1)
        list(GET _EXTRA_ARGS 0 _VAR)
        string_padding("${_DESC}" 21 _DESC)
        string_padding("${${_VAR}}" 12 _VALUE)
        set(_MESSAGE "   ${_DESC}: ${_VALUE} (${_VAR})")
    elseif(${_EXTRA_ARGS_LEN} GREATER 1)
        message(FATAL_ERROR "Invalid parameters for status_message call")
    endif()

    message(STATUS ${_MESSAGE})
endfunction()

# Generate Git version header
function(target_add_git_version_header _TARGET)
    find_package(Git REQUIRED)
    if(NOT TARGET git-version)
        file(WRITE ${CMAKE_BINARY_DIR}/git-version.h.in
            "#define GIT_VERSION \"@GIT_VERSION@\"\n"
        )
        file(WRITE ${CMAKE_BINARY_DIR}/git-version.cmake
            "execute_process(
                 COMMAND ${GIT_EXECUTABLE} describe --always --dirty
                 OUTPUT_VARIABLE GIT_VERSION
                 OUTPUT_STRIP_TRAILING_WHITESPACE
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
             )
             message(STATUS \"Git Version: \${GIT_VERSION}\")
             configure_file(
                 ${CMAKE_BINARY_DIR}/git-version.h.in
                 ${CMAKE_BINARY_DIR}/git-version.h @ONLY
             )"
        )
        add_custom_target(
            git-version
            ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/git-version.cmake
            COMMENT "Updating Git version header file (if necessary)"
            BYPRODUCTS ${CMAKE_BINARY_DIR}/git-version.h
        )
    endif()
    target_include_directories(${_TARGET} PRIVATE ${CMAKE_BINARY_DIR})
    target_sources(${_TARGET} PRIVATE ${CMAKE_BINARY_DIR}/git-version.h)
    add_dependencies(${_TARGET} git-version)
endfunction()
