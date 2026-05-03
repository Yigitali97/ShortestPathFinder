cmake_minimum_required(VERSION 3.14)
project(ShortestPathFinder CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SFML 3 ────────────────────────────────────────────────────────────────────
#   macOS:  brew install sfml cmake
#   Ubuntu: sudo apt install libsfml-dev cmake
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# ── Sources ───────────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
    main.cpp
    src/GraphCanvas.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME}
    SFML::Graphics
    SFML::Window
    SFML::System
)

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    file(COPY assets DESTINATION "${CMAKE_BINARY_DIR}")
endif()
