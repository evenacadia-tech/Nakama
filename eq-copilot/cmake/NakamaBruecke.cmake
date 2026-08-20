# SONDE-003 — Quellhash-Gate fuer den Nakama-Bridge-Patch (Entwurf §44.3).
#
# Der Patch veraendert FREMDEN Code (gevendortes JUCE unter _deps). Das ist nur
# vertretbar, wenn drei Dinge maschinell gelten:
#
#   1. IDEMPOTENT  — zweimal konfigurieren aendert nichts. Der gepatchte Hash
#                    ist ein Endzustand, kein Fehler.
#   2. GEGATET     — laeuft der Patch gegen einen ANDEREN Quellstand (JUCE-Update,
#                    fremder Fork, halb angewandt), bricht der Bau AB. Er raet nie.
#   3. SELBSTHEILEND — hat FetchContent die Datei zurueckgesetzt, wird neu gepatcht.
#
# Der Hash wird ueber den ZEILENENDE-NORMALISIERTEN Inhalt gebildet (CRLF -> LF).
# Grund, gemessen: der Windows-Checkout dieser Maschine legt die Datei mit CRLF
# ab (4165/4165 Zeilen). Ein Roh-Byte-Hash haenge damit an der core.autocrlf-
# Einstellung des jeweiligen Rechners — der User arbeitet abwechselnd an Desktop
# und Laptop, das waere ein Fehlalarm mit Bauabbruch statt eines echten Riegels.

# ── Gepinnte Quellstaende. EINZIGE Wahrheit; der C++-Test bekommt sie von hier. ──
set(NAKAMA_JUCE_TAG "8.0.9")
set(NAKAMA_JUCE_WRAPPER_REL "modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp")
set(NAKAMA_JUCE_WRAPPER_SHA_UNBERUEHRT "1374eb400cb6152dc39698dfdd1d7c89c2188d6720edc6bdcc486872f7da72d0")
set(NAKAMA_JUCE_WRAPPER_SHA_GEPATCHT   "6e5d4660d960836a875e4b2207f5bb4372b5266776e00c4bb0fdef1ee87a01bc")

# Berechnet den zeilenende-normalisierten SHA-256 einer Textdatei.
function(nakama_quellhash datei heraus)
    file(READ "${datei}" _inhalt)
    string(REPLACE "\r\n" "\n" _inhalt "${_inhalt}")
    string(SHA256 _hash "${_inhalt}")
    set(${heraus} "${_hash}" PARENT_SCOPE)
endfunction()

# Wendet den Bridge-Patch auf das gevendorte JUCE an — oder bricht ab.
function(nakama_bruecke_anwenden juce_quelle patch_datei)
    set(_ziel "${juce_quelle}/${NAKAMA_JUCE_WRAPPER_REL}")

    if(NOT EXISTS "${_ziel}")
        message(FATAL_ERROR
            "Nakama-Bruecke: JUCE-Wrapper nicht gefunden.\n"
            "  erwartet: ${_ziel}\n"
            "Ohne diese Datei kann der Patch weder angewandt noch geprueft werden.")
    endif()

    if(NOT EXISTS "${patch_datei}")
        message(FATAL_ERROR
            "Nakama-Bruecke: Patchdatei fehlt.\n"
            "  erwartet: ${patch_datei}")
    endif()

    nakama_quellhash("${_ziel}" _ist)

    # (1) Schon gepatcht: nichts tun. Das ist der Normalfall bei jedem
    #     Folge-Configure und der Kern der Idempotenz.
    if(_ist STREQUAL "${NAKAMA_JUCE_WRAPPER_SHA_GEPATCHT}")
        message(STATUS "Nakama-Bruecke: JUCE-Wrapper ist bereits gepatcht (${_ist}).")
        return()
    endif()

    # (2) Fremder Quellstand: ABBRUCH. Nicht raten, nicht erzwingen.
    if(NOT _ist STREQUAL "${NAKAMA_JUCE_WRAPPER_SHA_UNBERUEHRT}")
        message(FATAL_ERROR
            "Nakama-Bruecke: unbekannter JUCE-Quellstand — Bau abgebrochen.\n"
            "  Datei    : ${_ziel}\n"
            "  gemessen : ${_ist}\n"
            "  erwartet : ${NAKAMA_JUCE_WRAPPER_SHA_UNBERUEHRT} (unberuehrt ${NAKAMA_JUCE_TAG})\n"
            "  oder     : ${NAKAMA_JUCE_WRAPPER_SHA_GEPATCHT} (bereits gepatcht)\n"
            "\n"
            "Wahrscheinliche Ursache: JUCE wurde aktualisiert. Dann ist der\n"
            "Bridge-Patch NEU zu beweisen (SONDE-003): Patch gegen den neuen\n"
            "Wrapper pruefen, beide Hashes hier nachziehen, EqCopHostContextTest\n"
            "fahren. Ein blind erzwungener Patch waere genau die stille\n"
            "Fehlanwendung, die dieses Gate verhindert.")
    endif()

    # (3) Unberuehrt: anwenden.
    find_package(Git QUIET)
    if(NOT Git_FOUND)
        message(FATAL_ERROR
            "Nakama-Bruecke: git wird zum Anwenden des Patches gebraucht, ist aber nicht auffindbar.")
    endif()

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply --whitespace=nowarn "${patch_datei}"
        WORKING_DIRECTORY "${juce_quelle}"
        RESULT_VARIABLE _code
        OUTPUT_VARIABLE _aus
        ERROR_VARIABLE  _fehler)

    # Zweiter Versuch mit toleranter Leerraumbehandlung. Grund: JUCE speichert
    # den Wrapper im Objektspeicher mit CRLF, der Patch traegt es deshalb
    # ebenfalls; ein Checkout, der daran etwas dreht (fremdes core.autocrlf,
    # ein anderer Rechner), laesst die Kontextzeilen um genau ein \r
    # auseinanderlaufen. Das ist KEINE Aufweichung des Riegels: das Ergebnis
    # wird unten so oder so nachgemessen, ein falsch angewandter Patch faellt.
    if(NOT _code EQUAL 0)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" apply --whitespace=nowarn --ignore-whitespace "${patch_datei}"
            WORKING_DIRECTORY "${juce_quelle}"
            RESULT_VARIABLE _code2
            OUTPUT_VARIABLE _aus2
            ERROR_VARIABLE  _fehler2)
        if(_code2 EQUAL 0)
            message(STATUS "Nakama-Bruecke: Patch erst im zweiten Anlauf angewandt (--ignore-whitespace); "
                           "vermutlich abweichende Zeilenenden im JUCE-Checkout.")
            set(_code 0)
        else()
            message(FATAL_ERROR
                "Nakama-Bruecke: 'git apply' fehlgeschlagen (Code ${_code}, zweiter Anlauf ${_code2}).\n"
                "  Patch : ${patch_datei}\n"
                "  Baum  : ${juce_quelle}\n"
                "  stderr 1: ${_fehler}${_aus}\n"
                "  stderr 2: ${_fehler2}${_aus2}")
        endif()
    endif()

    # (4) Nachmessen statt glauben: ein 'git apply', das durchlief, ist noch
    #     kein Beweis, dass das Ergebnis der gepinnte Quellstand ist.
    nakama_quellhash("${_ziel}" _danach)
    if(NOT _danach STREQUAL "${NAKAMA_JUCE_WRAPPER_SHA_GEPATCHT}")
        message(FATAL_ERROR
            "Nakama-Bruecke: Patch lief durch, das Ergebnis stimmt aber nicht.\n"
            "  gemessen : ${_danach}\n"
            "  erwartet : ${NAKAMA_JUCE_WRAPPER_SHA_GEPATCHT}\n"
            "Der Wrapper ist jetzt in einem unbekannten Zustand — nicht weiterbauen.")
    endif()

    message(STATUS "Nakama-Bruecke: JUCE-Wrapper gepatcht und nachgemessen (${_danach}).")
endfunction()
