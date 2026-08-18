# EQ-Copilot — Bauwurzel (Plan §10). JUCE 8, Version gepinnt; Zielformat VST3.
cmake_minimum_required(VERSION 3.22)
project(EqCopilotSuite VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
# Gepinnt auf den neuesten 8er-Tag (git ls-remote, 2026-08-13). Upgrade nur bewusst.
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        8.0.9
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

add_subdirectory(plugin)
