# SONDE-005b — gepinntes FlatBuffers-Werkzeug (Entwurf §53).
#
# Der Entwurf verlangt woertlich: "Eine gepinnte flatc-Version erzeugt C++ und
# Rust; die generierten Dateien werden committed und ein Drift-Test verlangt
# bitgleichen Neugenerierungsdiff."
#
# Dieses Modul loest die erste Haelfte — den PIN. Drei Dinge muessen dafuer aus
# EINER Quelle kommen, sonst ist "Drift ist 0" eine Aussage ueber nichts:
#
#   1. der Compiler `flatc`,
#   2. die C++-Laufzeitheader, gegen die der erzeugte Code uebersetzt,
#   3. die Rust-Crate `flatbuffers`, gegen die der erzeugte Rust-Code uebersetzt.
#
# 1 und 2 kommen hier aus DEMSELBEN Commit — nicht aus einem vorkompilierten
# Binary plus separaten Headern, deren Zusammengehoerigkeit niemand nachprueft.
# 3 steht in broker/Cargo.toml und wird von tools/eq-copilot/pruefe_flatc_drift.py
# gegen dieselbe Steckbriefdatei gehalten.
#
# WARUM EIN COMMIT UND KEIN TAG: gemessen am 21.08.2026 fuehrt der Upstream fuer
# Version 25.12.19 ZWEI Tags (v25.12.19 und v25.12.19-2026-02-06-03fffb2). Eine
# Version wird also nachtraeglich neu geschnitten. Ein GIT_TAG-Pin auf den Namen
# waere ein Pin auf einen beweglichen Ref.

include_guard(GLOBAL)
include(FetchContent)

# ── Der Steckbrief ist die eine Wahrheit. Die Zahl steht nicht zweimal im Baum. ──
function(nakama_flatbuffers_steckbrief steckbrief_datei)
    if(NOT EXISTS "${steckbrief_datei}")
        message(FATAL_ERROR
            "Nakama-FlatBuffers: Werkzeugsteckbrief nicht gefunden.\n"
            "  erwartet: ${steckbrief_datei}\n"
            "Ohne ihn ist die flatc-Version unbestimmt und der Drift-Test wertlos.")
    endif()

    file(READ "${steckbrief_datei}" _js)
    string(JSON _version      GET "${_js}" version)
    string(JSON _repo         GET "${_js}" git_repository)
    string(JSON _commit       GET "${_js}" git_commit)
    string(JSON _major    GET "${_js}" erwartete_headerversion major)
    string(JSON _minor    GET "${_js}" erwartete_headerversion minor)
    string(JSON _revision GET "${_js}" erwartete_headerversion revision)

    set(NAKAMA_FLATC_VERSION  "${_version}"  PARENT_SCOPE)
    set(NAKAMA_FLATC_REPO     "${_repo}"     PARENT_SCOPE)
    set(NAKAMA_FLATC_COMMIT   "${_commit}"   PARENT_SCOPE)
    set(NAKAMA_FLATC_MAJOR    "${_major}"    PARENT_SCOPE)
    set(NAKAMA_FLATC_MINOR    "${_minor}"    PARENT_SCOPE)
    set(NAKAMA_FLATC_REVISION "${_revision}" PARENT_SCOPE)
endfunction()

# Liest die Version aus den GEHOLTEN Headern und vergleicht sie mit dem
# Steckbrief. Das ist der Riegel gegen einen Commit, der etwas anderes ist als
# er zu sein behauptet — ein Bau-ABBRUCH, kein Hinweis.
function(nakama_flatbuffers_version_pruefen quelle major minor revision)
    set(_base "${quelle}/include/flatbuffers/base.h")
    if(NOT EXISTS "${_base}")
        message(FATAL_ERROR
            "Nakama-FlatBuffers: base.h nicht gefunden.\n"
            "  erwartet: ${_base}\n"
            "Der geholte Quellstand ist nicht das, was der Steckbrief beschreibt.")
    endif()

    file(READ "${_base}" _inhalt)
    string(REGEX MATCH "#define FLATBUFFERS_VERSION_MAJOR +([0-9]+)"    _m "${_inhalt}")
    set(_gemessen_major "${CMAKE_MATCH_1}")
    string(REGEX MATCH "#define FLATBUFFERS_VERSION_MINOR +([0-9]+)"    _m "${_inhalt}")
    set(_gemessen_minor "${CMAKE_MATCH_1}")
    string(REGEX MATCH "#define FLATBUFFERS_VERSION_REVISION +([0-9]+)" _m "${_inhalt}")
    set(_gemessen_revision "${CMAKE_MATCH_1}")

    if(NOT _gemessen_major STREQUAL major
       OR NOT _gemessen_minor STREQUAL minor
       OR NOT _gemessen_revision STREQUAL revision)
        message(FATAL_ERROR
            "Nakama-FlatBuffers: der geholte Quellstand traegt eine ANDERE Version.\n"
            "  Steckbrief erwartet: ${major}.${minor}.${revision}\n"
            "  gemessen in base.h:  ${_gemessen_major}.${_gemessen_minor}.${_gemessen_revision}\n"
            "Entweder zeigt der gepinnte Commit woanders hin, oder der Steckbrief\n"
            "wurde geaendert ohne den Pin nachzuziehen. Beides macht 'Codegen-Drift\n"
            "ist 0' zu einer Aussage ueber ein bewegliches Ziel — deshalb Abbruch.")
    endif()

    message(STATUS "Nakama-FlatBuffers: Quellstand ${_gemessen_major}.${_gemessen_minor}.${_gemessen_revision} bestaetigt")
endfunction()

# Misst den tatsaechlich ausgecheckten Git-Commit. `flatc --version` reicht
# dafuer nicht: fuer 25.12.19 existieren mehrere Upstream-Schnitte, die dieselbe
# Versionszeichenkette melden. Der Beleg muss deshalb aus genau dem
# FetchContent-Quellverzeichnis kommen, aus dem auch `flatc` und die Header
# gebaut werden. Ein behaupteter Steckbriefwert waere hier zirkulaer.
function(nakama_flatbuffers_commit_pruefen quelle erwartet ergebnis)
    find_package(Git REQUIRED)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
        WORKING_DIRECTORY "${quelle}"
        RESULT_VARIABLE _git_exit
        OUTPUT_VARIABLE _gemessen
        ERROR_VARIABLE _git_fehler
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(LENGTH "${_gemessen}" _commit_laenge)
    if(NOT _git_exit EQUAL 0
       OR NOT _commit_laenge EQUAL 40
       OR _gemessen MATCHES "[^0-9a-fA-F]")
        message(FATAL_ERROR
            "Nakama-FlatBuffers: Commit des geholten Quellstands nicht belegbar.\n"
            "  Quelle: ${quelle}\n"
            "  git rev-parse HEAD: Exit ${_git_exit}, Ausgabe '${_gemessen}'\n"
            "  Fehler: ${_git_fehler}\n"
            "Ohne gemessenen Commit darf der Drift-Pruefer nie gruen melden.")
    endif()

    string(TOLOWER "${_gemessen}" _gemessen)
    string(TOLOWER "${erwartet}" _erwartet)
    if(NOT _gemessen STREQUAL _erwartet)
        message(FATAL_ERROR
            "Nakama-FlatBuffers: geholter Commit weicht vom Werkzeugsteckbrief ab.\n"
            "  Steckbrief: ${_erwartet}\n"
            "  Checkout:   ${_gemessen}\n"
            "Ein gleichlautendes --version belegt bei mehreren Schnitten nicht den Pin.")
    endif()

    set(${ergebnis} "${_gemessen}" PARENT_SCOPE)
    message(STATUS "Nakama-FlatBuffers: Commit ${_gemessen} bestaetigt")
endfunction()

# Haengt den Binary-Beleg im Verzeichnis des Upstream-Ziels an. CMake erlaubt
# die TARGET-Form von add_custom_command nur dort, wo das Ziel definiert wurde;
# deshalb ruft NakamaFlatBuffersProjektHook.cmake diese Funktion am Ende des
# FlatBuffers-Unterverzeichnisses auf. Entscheidend ist der Lebenszyklus:
# Configure misst den Checkout, aber erst ein tatsaechlicher flatc-Bau schreibt
# Commit UND Hash des soeben erzeugten Executables in denselben Sidecar.
function(nakama_flatbuffers_flatc_beleg_anhaengen)
    if(NOT TARGET flatc)
        message(FATAL_ERROR
            "Nakama-FlatBuffers: das FetchContent-Unterprojekt hat kein flatc-Ziel erzeugt.\n"
            "Ohne Ziel kann der Commit-/Binary-Beleg nicht an den Bau gebunden werden.")
    endif()

    get_target_property(_flatc_quelle flatc SOURCE_DIR)
    nakama_flatbuffers_commit_pruefen(
        "${_flatc_quelle}" "${NAKAMA_FLATC_COMMIT}" _gemessen_commit)

    set(_commit_beleg "${CMAKE_BINARY_DIR}/nakama-flatc-commit-$<CONFIG>.txt")
    add_custom_command(
        TARGET flatc POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            "-DNAKAMA_FLATC_BINARY=$<TARGET_FILE:flatc>"
            "-DNAKAMA_FLATC_COMMIT=${_gemessen_commit}"
            "-DNAKAMA_FLATC_BELEG=${_commit_beleg}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/NakamaFlatcBeleg.cmake"
        BYPRODUCTS "${_commit_beleg}"
        COMMENT "Nakama-FlatBuffers: Commit und SHA-256 an das gebaute flatc binden"
        VERBATIM)

    # Der Aufrufer prueft nach FetchContent_MakeAvailable, dass der Hook
    # tatsaechlich lief. So kann eine Aenderung am Upstream-Projekt den Beleg
    # nicht unbemerkt aus dem Buildgraphen entfernen.
    set_property(GLOBAL PROPERTY
        NAKAMA_FLATBUFFERS_GEMESSENER_COMMIT "${_gemessen_commit}")
endfunction()

# Holt FlatBuffers am gepinnten Commit und baut `flatc` daraus mit.
#
# `flatc` wird hier gebaut und nicht als Release-Binary geladen: die
# vorkompilierten Windows-Binaries haengen an einem RELEASE-Tag, die Header
# kaemen aus dem Quell-Tarball — zwei Downloads, deren Zusammengehoerigkeit
# ausser dem Dateinamen nichts belegt. Ein Commit belegt sie.
function(nakama_flatbuffers_bereitstellen steckbrief_datei)
    nakama_flatbuffers_steckbrief("${steckbrief_datei}")

    # FlatBuffers baut sonst seine eigene Testsuite, flathash und die Shared-Lib
    # mit. Wir brauchen genau zwei Dinge: die Header und den Compiler.
    set(FLATBUFFERS_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
    set(FLATBUFFERS_BUILD_FLATLIB   ON  CACHE BOOL "" FORCE)
    set(FLATBUFFERS_BUILD_FLATC     ON  CACHE BOOL "" FORCE)
    set(FLATBUFFERS_BUILD_FLATHASH  OFF CACHE BOOL "" FORCE)
    set(FLATBUFFERS_BUILD_SHAREDLIB OFF CACHE BOOL "" FORCE)
    set(FLATBUFFERS_INSTALL         OFF CACHE BOOL "" FORCE)
    set(FLATBUFFERS_BUILD_GRPCTEST  OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        flatbuffers
        GIT_REPOSITORY "${NAKAMA_FLATC_REPO}"
        GIT_TAG        "${NAKAMA_FLATC_COMMIT}"
    )

    # `flatc` wird im FlatBuffers-Unterverzeichnis definiert. Der
    # projektspezifische Include plant dort einen Callback fuer das Ende genau
    # dieses Verzeichnisses; nur dort ist ein echtes TARGET/POST_BUILD legal.
    # Einen eventuell vom Aufrufer gesetzten Projekt-Include verkettet der Hook.
    set_property(GLOBAL PROPERTY NAKAMA_FLATBUFFERS_GEMESSENER_COMMIT "")
    set(NAKAMA_FLATBUFFERS_PROJECT_INCLUDE_VORHER
        "${CMAKE_PROJECT_FlatBuffers_INCLUDE}")
    set(CMAKE_PROJECT_FlatBuffers_INCLUDE
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/NakamaFlatBuffersProjektHook.cmake")
    FetchContent_MakeAvailable(flatbuffers)
    set(CMAKE_PROJECT_FlatBuffers_INCLUDE
        "${NAKAMA_FLATBUFFERS_PROJECT_INCLUDE_VORHER}")

    get_property(_gemessen_commit GLOBAL PROPERTY
        NAKAMA_FLATBUFFERS_GEMESSENER_COMMIT)
    if(NOT _gemessen_commit)
        message(FATAL_ERROR
            "Nakama-FlatBuffers: der POST_BUILD-Beleg wurde nicht an flatc angehaengt.\n"
            "Der FlatBuffers-Projekthook lief nicht; ohne ihn darf der Bau nicht fortfahren.")
    endif()

    nakama_flatbuffers_version_pruefen(
        "${flatbuffers_SOURCE_DIR}"
        "${NAKAMA_FLATC_MAJOR}" "${NAKAMA_FLATC_MINOR}" "${NAKAMA_FLATC_REVISION}")

    # Der Drift-Pruefer laeuft ausserhalb von CMake (Python, aus dem Runner).
    # Er darf den Pfad zum gebauten flatc nicht raten: ein geratener Pfad, der
    # ins Leere zeigt, wuerde als "kein flatc" erscheinen und der Test wuerde
    # uebersprungen — genau die Pruefung, die nicht fehlschlagen kann. Deshalb
    # schreibt der Bau ihn hin.
    # Ein Zeiger JE KONFIGURATION: Visual Studio ist ein Multi-Config-Generator,
    # $<TARGET_FILE:flatc> ist unter Debug ein anderer Pfad als unter Release.
    # Eine einzige Datei mit drei Inhalten laesst CMake zu Recht nicht zu.
    set(_zeiger "${CMAKE_BINARY_DIR}/nakama-flatc-pfad-$<CONFIG>.txt")
    file(GENERATE OUTPUT "${_zeiger}" CONTENT "$<TARGET_FILE:flatc>\n")
    message(STATUS "Nakama-FlatBuffers: flatc-Zeiger -> ${CMAKE_BINARY_DIR}/nakama-flatc-pfad-<CONFIG>.txt")
    message(STATUS "Nakama-FlatBuffers: Binary-Beleg (POST_BUILD) -> ${CMAKE_BINARY_DIR}/nakama-flatc-commit-<CONFIG>.txt")
endfunction()
