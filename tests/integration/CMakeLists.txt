# ======================================================================
# Integration Tests - Multi-component behavior tests
# ======================================================================
# These tests should:
#   - Exercise multiple subsystems together
#   - Test realistic end-to-end workflows
#   - May require external resources (GPU, files, etc.)
#   - Run slower than unit tests (up to 3 minutes)
#
# All integration tests are labeled with "integration" and may be
# run on a schedule or behind a manual trigger rather than on every PR.
# ======================================================================

# Integration test: Renderer initialization and basic rendering
# Tests: Renderer creation, device initialization, resource loading
# Requirements: GPU available, DirectX 12 capable hardware
# Testability notes: Requires refactoring to allow headless/mock device
add_project_test(integration_renderer_init
  SOURCES
    test_renderer_init.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS integration renderer
  TIMEOUT ${TEST_TIMEOUT_INTEGRATION}
)

# Integration test: Asset loading pipeline
# Tests: Loading game data, fonts, textures, sounds from gamedata/
# Requirements: gamedata/ directory available at runtime
add_project_test(integration_asset_loading
  SOURCES
    test_asset_loading.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS integration assets
  TIMEOUT ${TEST_TIMEOUT_LONG}
)

# Integration test: Location and world loading
# Tests: Level file parsing, world initialization, entity spawning
# Requirements: Valid level files in gamedata/Levels/
add_project_test(integration_location_loading
  SOURCES
    test_location_loading.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS integration world
  TIMEOUT ${TEST_TIMEOUT_LONG}
)

# Integration test: Network client/server handshake
# Tests: Client connection to server, sync validation, determinism
# Requirements: Networking stack initialized
# Testability notes: May require mocking network layer for CI
add_project_test(integration_network_handshake
  SOURCES
    test_network_handshake.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS integration network
  TIMEOUT ${TEST_TIMEOUT_LONG}
)

# Integration test: Game simulation tick
# Tests: One complete game loop iteration without rendering
# Requirements: Minimal game state initialization
add_project_test(integration_simulation_tick
  SOURCES
    test_simulation_tick.cpp
  LIBRARIES
    InterstellarOutpost::NeuronCore
    InterstellarOutpost::NeuronClient
    InterstellarOutpost::GameLogic
  LABELS integration simulation
  TIMEOUT ${TEST_TIMEOUT_LONG}
)
