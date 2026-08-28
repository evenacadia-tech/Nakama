# Wird ueber CMAKE_PROJECT_FlatBuffers_INCLUDE im FetchContent-Unterprojekt
# geladen. Ein bereits gesetzter projektspezifischer Include bleibt erhalten.
if(DEFINED NAKAMA_FLATBUFFERS_PROJECT_INCLUDE_VORHER
   AND NOT NAKAMA_FLATBUFFERS_PROJECT_INCLUDE_VORHER STREQUAL "")
    include("${NAKAMA_FLATBUFFERS_PROJECT_INCLUDE_VORHER}")
endif()

# Der Callback laeuft am Ende dieses FlatBuffers-Verzeichnisses. Dann existiert
# `flatc`, und add_custom_command(TARGET flatc POST_BUILD) ist im selben
# CMake-Verzeichnis zulaessig.
cmake_language(DEFER CALL nakama_flatbuffers_flatc_beleg_anhaengen)
