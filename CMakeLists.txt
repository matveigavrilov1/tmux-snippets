cmake_minimum_required(VERSION 3.10.0)
project(tmux-snippets VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(git)
get_git_commit_hash(GIT_COMMIT_HASH)
message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")
add_definitions(-DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

find_package(pugixml REQUIRED)
find_package(ftxui REQUIRED)
find_package(stduuid REQUIRED)

set(UI_EXECUTABLE tmux-snippets-ui)
add_subdirectory(ui)

include(install)
install_project()
