cmake_minimum_required(VERSION 2.8.12)

# Scout - move rover around and collect stuff for points!
project(Airplane)

# Specify game extension to use with codetemplate
set(EXTENSION_URL https://github.com/GatorQue/ct-game-ext.git)

# Include codetemplate project (clone if missing)
include(${PROJECT_SOURCE_DIR}/codetemplate.cmake)

# Retrieve SFML third party module
include(ctGetSFML)

# Enable C++11 for this project
ct_enable_cxx11()

# Generate CMakeLists for source files in sources
ct_gen_cmake(${PROJECT_SOURCE_DIR}/sources)
