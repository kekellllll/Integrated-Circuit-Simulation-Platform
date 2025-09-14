# Examples CMakeLists.txt

# Simple circuit example
add_executable(example_rc_circuit rc_circuit_example.cpp)
target_link_libraries(example_rc_circuit ic_sim_core)

# Plugin usage example  
add_executable(example_plugins plugin_example.cpp)
target_link_libraries(example_plugins ic_sim_core)

# CUDA acceleration example
if(CUDA_AVAILABLE)
    add_executable(example_cuda_acceleration cuda_example.cpp)
    target_link_libraries(example_cuda_acceleration ic_sim_core ic_sim_cuda)
endif()

# Install examples
install(TARGETS example_rc_circuit example_plugins DESTINATION share/ic_simulator/examples)
install(FILES rc_filter.json amplifier.json DESTINATION share/ic_simulator/examples)