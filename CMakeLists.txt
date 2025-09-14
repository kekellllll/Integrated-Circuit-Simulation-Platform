cmake_minimum_required(VERSION 3.18)
project(IntegratedCircuitSimulation LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to enable CUDA if available
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    set(CUDA_AVAILABLE TRUE)
    message(STATUS "CUDA support enabled")
else()
    set(CUDA_AVAILABLE FALSE)
    message(STATUS "CUDA not available, building CPU-only version")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Check for optional packages
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GTK3 QUIET gtk+-3.0)
endif()

# Find CUDA if available
if(CUDA_AVAILABLE)
    find_package(CUDA REQUIRED)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
if(CUDA_AVAILABLE)
    include_directories(${CUDA_INCLUDE_DIRS})
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
if(CUDA_AVAILABLE)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O3 -use_fast_math")
    # CUDA architecture
    set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75;80")
endif()

# Source files
set(CORE_SOURCES
    src/core/circuit.cpp
    src/core/cuda_engine.cpp
    src/plugins/plugin_system.cpp
)

set(CUDA_SOURCES
    src/cuda/cuda_engine.cu
)

# Create core library
add_library(ic_sim_core STATIC ${CORE_SOURCES})
target_include_directories(ic_sim_core PUBLIC include)
target_link_libraries(ic_sim_core ${CMAKE_DL_LIBS})

# Create CUDA library (conditional)
if(CUDA_AVAILABLE)
    add_library(ic_sim_cuda STATIC ${CUDA_SOURCES})
    target_include_directories(ic_sim_cuda PUBLIC include)
    target_link_libraries(ic_sim_cuda ${CUDA_LIBRARIES} ${CUDA_CUBLAS_LIBRARIES})
    
    # Link CUDA library to core
    target_link_libraries(ic_sim_core ic_sim_cuda)
    target_compile_definitions(ic_sim_core PUBLIC CUDA_AVAILABLE)
endif()

# Main executable
add_executable(ic_simulator src/main.cpp)
target_link_libraries(ic_simulator ic_sim_core Threads::Threads)

# Example plugin
add_library(example_plugin SHARED src/plugins/example_plugin.cpp)
target_include_directories(example_plugin PUBLIC include)
target_link_libraries(example_plugin ic_sim_core)

# Web interface server (optional)
if(GTK3_FOUND)
    add_executable(web_server src/web_server.cpp)
    target_include_directories(web_server PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(web_server ic_sim_core ${GTK3_LIBRARIES} Threads::Threads)
    target_compile_options(web_server PRIVATE ${GTK3_CFLAGS_OTHER})
endif()

# Tests
enable_testing()

add_subdirectory(tests)

# Examples
add_subdirectory(examples)

# Documentation
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
    )
endif()

# Installation
install(TARGETS ic_simulator DESTINATION bin)
install(TARGETS ic_sim_core DESTINATION lib)
if(CUDA_AVAILABLE)
    install(TARGETS ic_sim_cuda DESTINATION lib)
endif()
install(TARGETS example_plugin DESTINATION lib/plugins)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY web/ DESTINATION share/ic_simulator/web)

# CPack configuration
set(CPACK_PACKAGE_NAME "IntegratedCircuitSimulator")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "Advanced Integrated Circuit Simulation Platform")
set(CPACK_PACKAGE_CONTACT "developer@example.com")

include(CPack)