cmake_minimum_required(VERSION 3.21)
project(EpidemicSFML CXX)

# language/flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
  add_definitions(-DNOMINMAX)
endif()

# your sources (fixed names to match your tree)
set(SRCS
        main_sfml.cpp
        Node.cpp
        AdjacencyGrid.cpp
        QuadTree.cpp
)

add_executable(epidemic ${SRCS})
add_custom_command(TARGET epidemic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/arial.ttf"
        "$<TARGET_FILE_DIR:epidemic>/arial.ttf"
)
target_include_directories(epidemic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# try system SFML first
set(SFML_MIN_VERSION 2.6)
find_package(SFML ${SFML_MIN_VERSION} COMPONENTS graphics window system QUIET)

# if not found, fetch & build SFML automatically
if (NOT SFML_FOUND)
  message(STATUS "SFML not found on system; fetching SFML via FetchContent...")

  include(FetchContent)

  # only build what we need
  set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(SFML_USE_STATIC_STD_LIBS OFF CACHE BOOL "" FORCE)

  # fetch SFML
  FetchContent_Declare(
          sfml
          GIT_REPOSITORY https://github.com/SFML/SFML.git
          GIT_TAG 2.6.1
  )
  FetchContent_MakeAvailable(sfml)

  # SFML targets
  target_link_libraries(epidemic PRIVATE sfml-graphics sfml-window sfml-system)
else()
  message(STATUS "Using system SFML: ${SFML_VERSION}")
  target_link_libraries(epidemic PRIVATE sfml-graphics sfml-window sfml-system)
endif()

# copy required DLLs next to the exe on Windows
if (WIN32)
  add_custom_command(TARGET epidemic POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_RUNTIME_DLLS:epidemic>
          $<TARGET_FILE_DIR:epidemic>
          COMMAND_EXPAND_LISTS
  )
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Binary will be at: ${CMAKE_BINARY_DIR}/epidemic[.exe]")