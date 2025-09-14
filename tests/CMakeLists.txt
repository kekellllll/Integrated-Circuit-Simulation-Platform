# Tests CMakeLists.txt
add_subdirectory(unit)
add_subdirectory(integration)

# Unit tests
add_executable(test_circuit unit/test_circuit.cpp)
target_link_libraries(test_circuit ic_sim_core)
add_test(NAME CircuitTests COMMAND test_circuit)

add_executable(test_plugins unit/test_plugins.cpp)
target_link_libraries(test_plugins ic_sim_core)
add_test(NAME PluginTests COMMAND test_plugins)

if(CUDA_AVAILABLE)
    add_executable(test_cuda unit/test_cuda.cpp)
    target_link_libraries(test_cuda ic_sim_core ic_sim_cuda)
    add_test(NAME CudaTests COMMAND test_cuda)
endif()

# Integration tests
add_executable(test_simulation integration/test_simulation.cpp)
target_link_libraries(test_simulation ic_sim_core)
add_test(NAME SimulationIntegration COMMAND test_simulation)

# Set test properties
set_tests_properties(CircuitTests PROPERTIES TIMEOUT 30)
set_tests_properties(PluginTests PROPERTIES TIMEOUT 30)
set_tests_properties(SimulationIntegration PROPERTIES TIMEOUT 60)

if(CUDA_AVAILABLE)
    set_tests_properties(CudaTests PROPERTIES TIMEOUT 60)
endif()