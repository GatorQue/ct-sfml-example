###############################################################################

# Include auto target file for this directory
include(${PROJECT_BINARY_DIR}/Autoairplane.cmake)

# Include auto target dependency file for this directory
include(${PROJECT_BINARY_DIR}/AutoairplaneDeps.cmake)

# Add SFML libraries for main
set(main_LIBS airplane ${SFML_LIBRARIES})

# Add library if library sources was defined in Autoairplane.cmake
if(LIB_SOURCES)
  # Add comprehensive library for this folder
  ct_add_lib(airplane
    INSTALL ${airplane_INSTALL}
    TYPE STATIC
    SOURCES ${LIB_SOURCES}
    HEADERS ${LIB_HEADERS})
endif()

# Add applications if app sources was defined in Autoairplane.cmake
if(APP_SOURCES)
  foreach(_app ${APP_SOURCES})
    get_filename_component(_name ${_app} NAME_WE)
    # Add local link dependencies (if lib files exist)
    ct_add_exe(${_name}
      INSTALL ${${_name}_INSTALL}
      SOURCES ${_app}
      DEPENDS ${${_name}_DEPS}
      LIBS ${${_name}_LIBS})
  endforeach()
endif()

# Add test executables if test sources was defined in Autoairplane.cmake
if(TEST_SOURCES)
  foreach(_test ${TEST_SOURCES})
    get_filename_component(_name ${_test} NAME_WE)
    # Add local link dependencies (if lib files exist)
    ct_add_test(${_name}
      INSTALL ${${_name}_INSTALL}
      SOURCES ${_test}
      DEPENDS ${${_name}_DEPS}
      LIBS ${${_name}_LIBS})
  endforeach()
endif()

###############################################################################
# EOF
