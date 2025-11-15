# ======================================================================
# Unit Tests - Fast, isolated tests of individual components
# ======================================================================
# These tests should:
#   - Execute quickly (< 30 seconds)
#   - Not require external resources (GPU, network, file I/O if possible)
#   - Test single classes/functions in isolation
#   - Use mocks/stubs for dependencies when necessary
#
# All unit tests are labeled with "unit" and run on every PR/push.
# ======================================================================

# NeuronCore library smoke tests
# Tests: Basic functionality of core engine utilities
add_project_test(unit_neuron_core_smoke
  SOURCES
    neuron_core/test_neuron_core.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
  LABELS unit neuroncore
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

# NeuronClient DirectX math wrapper tests
# Tests: OpenGL/DirectX math conversion utilities
add_project_test(unit_neuron_client_dxmath
  SOURCES
    neuron_client/test_opengl_directx_math.cpp
  LIBRARIES
    InterstellarOutpost::NeuronClient
  LABELS unit neuronclient
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

# NeuronClient matrix stack tests
# Tests: Matrix stack operations for rendering transforms
add_project_test(unit_neuron_client_matrix_stack
  SOURCES
    neuron_client/test_opengl_directx_matrix_stack.cpp
  LIBRARIES
    InterstellarOutpost::NeuronClient
  LABELS unit neuronclient
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

# NeuronClient display list / command buffer tests
# Tests: Deferred rendering command recording
add_project_test(unit_neuron_client_display_list
  SOURCES
    neuron_client/test_opengl_directx_commands.cpp
  LIBRARIES
    InterstellarOutpost::NeuronClient
  LABELS unit neuronclient
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

# GameLogic library smoke tests
# Tests: Core gameplay systems and entity management
add_project_test(unit_gamelogic_smoke
  SOURCES
    gamelogic/test_gamelogic.cpp
  LIBRARIES
    InterstellarOutpost::GameLogic
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
  LABELS unit gamelogic
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

# Camera system unit tests
# Tests: Camera modes, positioning, view frustum calculations
add_project_test(unit_camera_basics
  SOURCES
    src/test_camera.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS unit camera
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)

# Routing system unit tests
# Tests: Pathfinding waypoint and route management
add_project_test(unit_routing_system
  SOURCES
    src/test_routing_system.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS unit routing
  TIMEOUT ${TEST_TIMEOUT_SHORT}
)
