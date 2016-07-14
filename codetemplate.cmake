###############################################################################
# NOTE: Bump the version below if this file changes
###############################################################################
set(_ct_cmake_version 1.2.0)
if(CODE_TEMPLATE_FOUND)
  return()
endif()
set(CODE_TEMPLATE_FOUND TRUE)
set(_current_ct_cmake_version ${_ct_cmake_version})

# Specify codetemplate repository URL if not provided
if(NOT CODE_TEMPLATE_URL)
  set(CODE_TEMPLATE_URL "https://github.com/GatorQue/codetemplate.git")
endif()

# Specify codetemplate branch to use if not provided
if(NOT CODE_TEMPLATE_BRANCH)
  set(CODE_TEMPLATE_BRANCH "master")
endif()

# Specify codetemplate repository path if not provided as sibling to
# CMAKE_SOURCE_DIR
if(NOT CODE_TEMPLATE_DIR)
  set(CODE_TEMPLATE_DIR ${CMAKE_SOURCE_DIR}/../codetemplate)
endif()
get_filename_component(CODE_TEMPLATE_DIR ${CODE_TEMPLATE_DIR} ABSOLUTE)
message(STATUS "CODE_TEMPLATE_DIR=${CODE_TEMPLATE_DIR}")

# Obtain parent of codetemplate repository path
set(CODE_TEMPLATE_PARENT_DIR ${CODE_TEMPLATE_DIR}/..)
get_filename_component(CODE_TEMPLATE_PARENT_DIR ${CODE_TEMPLATE_PARENT_DIR} ABSOLUTE)
message(STATUS "CODE_TEMPLATE_PARENT_DIR=${CODE_TEMPLATE_PARENT_DIR}")

# Use codetemplate extension URL if provided
if(EXTENSION_URL)
  # Specify codetemplate extension branch to use if not provided
  if(NOT EXTENSION_BRANCH)
    set(EXTENSION_BRANCH "master")
  endif()
  # Specify codetemplate extension repository path if not provided as sibling
  # to CMAKE_SOURCE_DIR
  if(NOT EXTENSION_DIR)
    set(EXTENSION_DIR ${CMAKE_SOURCE_DIR}/../extension)
  endif()
  get_filename_component(EXTENSION_DIR ${EXTENSION_DIR} ABSOLUTE)
  message(STATUS "EXTENSION_DIR=${EXTENSION_DIR}")

  # Obtain parent of codetemplate extension repository path
  set(EXTENSION_PARENT_DIR ${EXTENSION_DIR}/..)
  get_filename_component(EXTENSION_PARENT_DIR ${EXTENSION_PARENT_DIR} ABSOLUTE)
  message(STATUS "EXTENSION_PARENT_DIR=${EXTENSION_PARENT_DIR}")
endif()

# Specify download cache path if not provided as sibling to CMAKE_SOURCE_DIR
if(NOT DOWNLOAD_CACHE_DIR)
  set(DOWNLOAD_CACHE_DIR ${CMAKE_SOURCE_DIR}/../dl)
endif()
get_filename_component(DOWNLOAD_CACHE_DIR ${DOWNLOAD_CACHE_DIR} ABSOLUTE)
message(STATUS "DOWNLOAD_CACHE_DIR=${DOWNLOAD_CACHE_DIR}")

# Create download cache directory if it doesn't exist yet
if(NOT EXISTS ${DOWNLOAD_CACHE_DIR})
  file(MAKE_DIRECTORY ${DOWNLOAD_CACHE_DIR})
endif()

# Specify third party path if not provided as sibling to CMAKE_SOURCE_DIR
if(NOT THIRD_PARTY_DIR)
  set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/../third_party)
endif()
get_filename_component(THIRD_PARTY_DIR ${THIRD_PARTY_DIR} ABSOLUTE)
message(STATUS "THIRD_PARTY_DIR=${THIRD_PARTY_DIR}")

# Create download cache directory if it doesn't exist yet
if(NOT EXISTS ${THIRD_PARTY_DIR})
  file(MAKE_DIRECTORY ${THIRD_PARTY_DIR})
endif()

# Find git executable (exit with error if missing)
find_program(GIT_PATH git)

if(NOT EXISTS ${GIT_PATH})
  message(FATAL_ERROR "CodeTemplate requires git executable")
endif()

macro(_get_version _dir _var)
  # Tell the user which codetemplate version is being used
  execute_process(
    COMMAND ${GIT_PATH} describe --tags
    WORKING_DIRECTORY ${_dir}
    OUTPUT_VARIABLE ${_var}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    set(${_var} "unknown")
  endif()
endmacro()

# This macro will show and store the codetemplate version in use
macro(ct_show_version)
  _get_version(${CODE_TEMPLATE_DIR} CODE_TEMPLATE_VERSION)
  message(STATUS "Using CodeTemplate Version (${CODE_TEMPLATE_VERSION})")
endmacro()

# This macro will show and store the codetemplate extension version in use
macro(ext_show_version)
  _get_version(${EXTENSION_DIR} EXTENSION_VERSION)
  message(STATUS "Using Extension Version (${EXTENSION_VERSION})")
endmacro()

# This macro creates a cache of the directory specified to the filename
# provided.
macro(_create_cache _parent _dir _filename)
  file(RELATIVE_PATH _ct_relative ${_parent} ${_dir})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cfz ${DOWNLOAD_CACHE_DIR}/${_filename} ${_ct_relative}
    WORKING_DIRECTORY ${_parent}
    RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Unable to create ${_filename} archive in ${DOWNLOAD_CACHE_DIR}")
  endif()
endmacro()

# This macro extracts filename provided to the directory specified
macro(_extract_cache _parent _dir _filename)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xfz ${DOWNLOAD_CACHE_DIR}/${_filename} ${_dir}
    WORKING_DIRECTORY ${_parent}
    RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    file(REMOVE ${DOWNLOAD_CACHE_DIR}/${_filename})
    message(FATAL_ERROR "Unable to extract ${_filename} to ${_dir}")
  endif()
endmacro()

# This macro attempts to use git to clone the URL and branch specified to the
# directory provided.
macro(_get_from_git _parent _dir _archive _url _branch)
  # Use Git to download codetemplate repository
  file(REMOVE_RECURSE ${_dir})
  execute_process(
    COMMAND ${GIT_PATH} clone -b ${_branch} -- ${_url} ${_dir}
    WORKING_DIRECTORY ${_parent}
    RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Unable to clone repository to ${_dir}")
  else()
    _create_cache(${_parent} ${_dir} ${_archive})
  endif()
endmacro()

# Check to see if codetemplate repository is already available
if(NOT EXISTS ${CODE_TEMPLATE_DIR})
  set(CODE_TEMPLATE_ARCHIVE codetemplate-${CODE_TEMPLATE_BRANCH}.tgz)
  # Download cache of codetemplate doesn't exist? clone it now
  if(NOT EXISTS ${DOWNLOAD_CACHE_DIR}/${CODE_TEMPLATE_ARCHIVE})
    _get_from_git(${CODE_TEMPLATE_PARENT_DIR} ${CODE_TEMPLATE_DIR}
        ${CODE_TEMPLATE_ARCHIVE} ${CODE_TEMPLATE_URL} ${CODE_TEMPLATE_BRANCH})
  else()
    # Use download cache of codetemplate instead
    _extract_cache(${CODE_TEMPLATE_PARENT_DIR} ${CODE_TEMPLATE_DIR}
        ${CODE_TEMPLATE_ARCHIVE})
  endif()
endif()

ct_show_version()

# Include codetemplate.cmake file in codetemplate and verify versions
include(${CODE_TEMPLATE_DIR}/codetemplate.cmake)
if(_ct_cmake_version VERSION_GREATER _current_ct_cmake_version)
  message(WARNING "Your codetemplate.cmake file is old (${_current_ct_cmake_version}), please this update project to use newer codetemplate.cmake file (${_ct_cmake_version}).")
endif()

# Add codetemplate cmake module path as first path
set(CMAKE_MODULE_PATH ${CODE_TEMPLATE_DIR}/Modules ${CMAKE_MODULE_PATH})

# Retrieve codetemplate extension repository/archive if specified
if(EXTENSION_URL)
  # Does extension directory not yet exist? create it now
  if(NOT EXISTS ${EXTENSION_DIR})
    set(EXTENSION_ARCHIVE extension-${EXTENSION_BRANCH}.tgz)
    # Download cache of codetemplate extension doesn't exist? clone it now
    if(NOT EXISTS ${DOWNLOAD_CACHE_DIR}/${EXTENSION_ARCHIVE})
      _get_from_git(${EXTENSION_PARENT_DIR} ${EXTENSION_DIR}
          ${EXTENSION_ARCHIVE} ${EXTENSION_URL} ${EXTENSION_BRANCH})
    else()
      # Use download cache of codetemplate extension instead
      _extract_cache(${EXTENSION_PARENT_DIR} ${EXTENSION_DIR}
          ${EXTENSION_ARCHIVE})
    endif()
  endif()

  ext_show_version()

  # Add codetemplate extension cmake module path as first path
  set(CMAKE_MODULE_PATH ${EXTENSION_DIR}/Modules ${CMAKE_MODULE_PATH})
endif()

# Include ctIncludes as last step
include(ctIncludes)
