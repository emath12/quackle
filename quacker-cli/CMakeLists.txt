cmake_minimum_required(VERSION 3.10)

project(QuackleCLI VERSION 1.0.0)

message("-- Configuring QuackleCLI")

include("${CMAKE_CURRENT_SOURCE_DIR}/../Settings.cmake")

# Option for macOS bundling (not needed for CLI)
option(BUNDLE "Build app bundle on macOS; does not populate with resources" OFF)

# Include the necessary libraries (libquackle, quackleio, etc.)
add_subdirectory(.. libquackle)
add_subdirectory(../quackleio quackleio)
add_subdirectory(../data data)

# Define the source files for the CLI
set(CLI_SOURCES
    main.cpp  # Your CLI main file
    # Add any other necessary source files for CLI here
)

# Define the header files for the CLI (if needed)
set(CLI_HEADERS
    # Add header files for CLI
)

# Create the CLI executable (without Qt)
add_executable(QuackleCLI
    ${CLI_SOURCES}  # Include your main file and any other source files
    ${CLI_HEADERS}  # If you have headers, include them as well
    # Remove `quacker.icns` if not needed for the CLI
)

# Link the necessary libraries (no Qt libraries)
target_link_libraries(QuackleCLI quackleio libquackle)

# Set platform-specific configurations if needed
if(APPLE)
    target_link_libraries(QuackleCLI "-framework CoreFoundation")
endif()

# Platform-specific post-build commands (optional)
