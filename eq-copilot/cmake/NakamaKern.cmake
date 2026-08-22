# S8 / SONDE-007a — der gemeinsame Kern der drei Nakama-Apps.
#
# ── Was hier geloest wird ───────────────────────────────────────────────────
#
# Bis heute stand im Kopf von plugin/CMakeLists.txt: "Probe und Tests
# kompilieren die geteilten Quellen erneut, statt eine static-lib mit fremden
# JUCE-Moduldefinitionen zu teilen — bewusst simpel." Das war richtig, solange
# es EIN Bundle gab. Mit Gen, Probeeq und Suna wird es falsch, denn dann
# vervielfacht sich nicht nur die Bauzeit, sondern auch die Zahl der Stellen,
# an denen der Kern eine fremde Identitaet einatmen kann.
#
# Entwurf §53.4 (Static-Lib-Randbedingung) erlaubt den Wechsel unter EINER
# harten Regel: "Der gemeinsame Kern sieht KEINE JucePlugin_*-Konstanten;
# Identitaet kommt ausschliesslich aus plugin-identities-v1.json ueber die
# duennen Target-Schichten. Andernfalls erhalten zwei der drei Bundles die
# Identitaetskonstanten des dritten."
#
# ── Warum eine Kopf-Fassade und nicht `target_link_libraries(... juce::…)` ──
#
# JUCE-Module sind INTERFACE-Bibliotheken, deren .cpp als INTERFACE_SOURCES
# haengen (_deps/juce-src/extras/Build/CMake/JUCEModuleSupport.cmake:505). Wer
# sie linkt, KOMPILIERT sie mit hinein. Eine Static-Lib, die juce::juce_core
# linkt, traegt also ihre eigene Kopie von juce_core.obj — und jedes Ziel, das
# beides linkt, haette zwei. Drei Messungen am 22.08.2026 (Wegwerfprojekt,
# VS 17 2022 / CMake 3.31.6):
#
#   1. `$<COMPILE_ONLY:iface>` streift INTERFACE_SOURCES NICHT ab — die
#      Modulquelle wurde trotzdem in die Lib kompiliert. Weg tot.
#   2. Beide Kopien nebeneinander LINKEN sauber (Exit 0), weil MSVC ein
#      .lib-Mitglied nur zieht, wenn sonst ein Symbol offen bliebe. "Geht
#      heute" — aber es haelt nur, solange beide Kopien deckungsgleich
#      uebersetzt werden. Fuer einen Kern, der drei Bundles traegt, zu wenig.
#   3. Eine Fassade, die Includes und Defines aus dem Modul-Target ABLEITET,
#      seine Quellen aber nicht erbt: Kern sieht die Defines (ein `#error`-
#      Gegentest schwieg), die Modulquelle wurde GENAU EINMAL uebersetzt,
#      Link sauber. Das ist der Weg.
#
# Der Kern uebersetzt damit gegen JUCE-KOEPFE; die Implementierung kommt aus
# der einen Kopie, die das verbrauchende Ziel ohnehin schon baut.
#
# ── Riegel ─────────────────────────────────────────────────────────────────
#
#   K1  plugin/state/NakamaKernRiegel.h — namentlich, im Uebersetzer.
#   K2  nakama_kern_riegel_pruefen() hier — Regex ueber die ganze Linkhuelle;
#       schliesst die Praefix-Luecke, die der Praeprozessor offenlaesst.
#   K3  tools/eq-copilot/pruefe_kern_identitaetsfrei.py — misst das GEBAUTE
#       NakamaKern.lib gegen eq-copilot/identity/plugin-identities-v1.json.
#
# K1 und K2 reden ueber die Baubeschreibung, K3 ueber das Artefakt. Erst alle
# drei zusammen sind eine Aussage.

include_guard(GLOBAL)

# ── Linkhuellen-Laeufer: einmal geschrieben, zweimal benutzt ────────────────
# Sammelt rekursiv alle erreichbaren Ziele ab `start`. Nicht-Ziele (System-
# bibliotheken wie ws2_32, nackte Generatorausdruecke) werden uebersprungen —
# sie tragen weder Includes noch Defines, die uns interessieren.
function(_nakama_kern_huelle start ausgabe)
    set(_gesehen "")
    set(_offen "${start}")

    while(_offen)
        list(POP_FRONT _offen _ziel)

        # Alias aufloesen: juce::juce_core -> juce_core
        if(TARGET "${_ziel}")
            get_target_property(_alias "${_ziel}" ALIASED_TARGET)
            if(_alias)
                set(_ziel "${_alias}")
            endif()
        endif()

        if(NOT TARGET "${_ziel}")
            continue()
        endif()
        if("${_ziel}" IN_LIST _gesehen)
            continue()
        endif()
        list(APPEND _gesehen "${_ziel}")

        foreach(_eigenschaft INTERFACE_LINK_LIBRARIES LINK_LIBRARIES)
            get_target_property(_kanten "${_ziel}" ${_eigenschaft})
            if(_kanten)
                foreach(_kante IN LISTS _kanten)
                    # Generatorausdruecke koennen einen Zielnamen umhuellen
                    # ($<LINK_ONLY:x>, $<COMPILE_ONLY:x>). Den Namen freilegen,
                    # sonst laeuft die Huelle an einer echten Kante vorbei.
                    string(REGEX REPLACE "^\\$<[A-Z_]+:(.+)>$" "\\1" _kante "${_kante}")
                    list(APPEND _offen "${_kante}")
                endforeach()
            endif()
        endforeach()
    endwhile()

    set(${ausgabe} "${_gesehen}" PARENT_SCOPE)
endfunction()

# ── K2: der Konfigurier-Riegel ──────────────────────────────────────────────
# Faellt, BEVOR eine einzige Uebersetzungseinheit laeuft, sobald irgendwo in
# der Linkhuelle des Kerns ein `JucePlugin_` steht. Der haeufigste Weg dorthin
# ist eine Linkkante zu einem juce_add_plugin-Ziel — dessen Identitaetsdefines
# sind PUBLIC (JUCEUtils.cmake:1543), also erbt sie jeder Linker.
#
# GRENZE, die hier ausgesprochen gehoert: der Riegel liest Eigenschaften zur
# Konfigurierzeit. Ein Define, das erst ueber einen Generatorausdruck
# ($<TARGET_PROPERTY:…>) entsteht, sieht er als unausgewerteten Text. Genau
# diese Luecke misst K3 am gebauten Artefakt.
function(nakama_kern_riegel_pruefen ziel)
    _nakama_kern_huelle("${ziel}" _huelle)

    # Die Fassade leitet per Generatorausdruck aus Modulzielen ab, statt sie zu
    # linken. Diese Herkunft steht als Eigenschaft am Ziel, damit der Riegel
    # nicht an ihr vorbeilaeuft.
    foreach(_h IN LISTS _huelle)
        get_target_property(_herkunft "${_h}" NAKAMA_KERN_ABGELEITET_VON)
        if(_herkunft)
            foreach(_m IN LISTS _herkunft)
                if(TARGET "${_m}" AND NOT "${_m}" IN_LIST _huelle)
                    list(APPEND _huelle "${_m}")
                endif()
            endforeach()
        endif()
    endforeach()

    set(_funde "")
    foreach(_h IN LISTS _huelle)
        foreach(_eigenschaft COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)
            get_target_property(_defs "${_h}" ${_eigenschaft})
            if(_defs)
                foreach(_d IN LISTS _defs)
                    if("${_d}" MATCHES "JucePlugin_")
                        list(APPEND _funde "${_h} [${_eigenschaft}] ${_d}")
                    endif()
                endforeach()
            endif()
        endforeach()
    endforeach()

    list(LENGTH _huelle _anzahl)
    if(_funde)
        string(REPLACE ";" "\n    " _liste "${_funde}")
        message(FATAL_ERROR
            "S8/SONDE-007a K2: Der gemeinsame Kern '${ziel}' sieht JucePlugin_-Konstanten.\n"
            "  ${_liste}\n"
            "Damit traegt sein Objektcode die Identitaet EINES Bundles, und Gen, Probeeq und\n"
            "Suna erben sie gemeinsam — der Identitaets-Golden aus S2 faellt (Entwurf §53.4).\n"
            "Ursache ist fast immer eine Linkkante zu einem juce_add_plugin-Ziel: dessen\n"
            "JucePlugin_*-Defines sind PUBLIC (JUCEUtils.cmake:1543). Identitaet gehoert in\n"
            "die duenne Target-Schicht, nicht in den Kern.")
    endif()

    message(STATUS
        "Nakama-Kern: K2 gruen — ${_anzahl} Ziele in der Linkhuelle von '${ziel}', "
        "keine JucePlugin_-Konstante.")
endfunction()

# ── Die Kopf-Fassade ───────────────────────────────────────────────────────
# Leitet Includes und Defines aus den Modulzielen ab (inklusive deren eigener
# Linkhuelle, damit z. B. juce_atomic_wrapper nicht durchfaellt) und laesst die
# Modulquellen liegen. Nichts hier ist abgeschrieben: aendert JUCE seine
# Moduldefines, wandert die Aenderung von selbst mit.
function(nakama_kern_juce_fassade name)
    add_library(${name} INTERFACE)

    set(_module ${ARGN})
    set(_alle "")
    foreach(_m IN LISTS _module)
        _nakama_kern_huelle("${_m}" _teil)
        list(APPEND _alle ${_teil})
    endforeach()
    list(REMOVE_DUPLICATES _alle)

    foreach(_m IN LISTS _alle)
        target_include_directories(${name} INTERFACE
            $<TARGET_PROPERTY:${_m},INTERFACE_INCLUDE_DIRECTORIES>)
        target_compile_definitions(${name} INTERFACE
            $<TARGET_PROPERTY:${_m},INTERFACE_COMPILE_DEFINITIONS>)
        target_compile_options(${name} INTERFACE
            $<TARGET_PROPERTY:${_m},INTERFACE_COMPILE_OPTIONS>)
    endforeach()

    # Herkunft festhalten — K2 laeuft sie ab, weil Generatorausdruecke zur
    # Konfigurierzeit keine Kante sind.
    set_target_properties(${name} PROPERTIES NAKAMA_KERN_ABGELEITET_VON "${_alle}")

    list(LENGTH _alle _anzahl)
    message(STATUS "Nakama-Kern: JUCE-Kopffassade '${name}' aus ${_anzahl} Modulzielen abgeleitet.")
endfunction()

# ── K2b: die JUCE-Projektkonfiguration darf nicht auseinanderlaufen ─────────
# Der Kern uebersetzt DIESELBEN JUCE-Kopfdateien wie seine Verbraucher, aber
# mit seiner eigenen Definemenge. Solange eine JUCE_*-Konfigurationsschraube
# hier anders steht als dort, koennte derselbe Header zwei verschiedene Dinge
# bedeuten - genau die Voraussetzung einer ODR-Verletzung.
#
# Gemessen am 22.08.2026: heute weicht genau eine ab (JUCE_USE_CURL; der Kern
# saehe den Vorgabewert 1 aus juce_core.h:152 statt der 0 des Projekts), und
# sie ist folgenlos - eine Grep ueber alle Kopfdateien von juce_core,
# juce_events, juce_data_structures und juce_cryptography findet den Namen NUR
# im Konfigblock von juce_core.h selbst, kein Header verzweigt darauf, und der
# Kern uebersetzt ohnehin keine juce_core-Quelle. Folgenlos heute heisst nicht
# folgenlos morgen: dieser Riegel haelt die beiden Mengen zusammen.
#
# Nicht verglichen wird, was legitim verschieden ist:
#   JUCE_MODULE_AVAILABLE_*  - der Kern nutzt weniger Module, das ist sein Sinn
#   JUCE_SHARED_CODE / JUCE_STANDALONE_APPLICATION / JUCE_VST3_CAN_REPLACE_VST2
#                            - Sache der Plugin-Huelle; K1 verbietet die
#                              ersten beiden im Kern ausdruecklich
function(nakama_kern_konfig_pruefen kern referenz)
    get_target_property(_ref "${referenz}" COMPILE_DEFINITIONS)
    get_target_property(_kern_defs "${kern}" COMPILE_DEFINITIONS)
    get_target_property(_fassade "${kern}" LINK_LIBRARIES)

    set(_haben "${_kern_defs}")
    foreach(_f IN LISTS _fassade)
        if(TARGET "${_f}")
            get_target_property(_fd "${_f}" INTERFACE_COMPILE_DEFINITIONS)
            if(_fd)
                list(APPEND _haben ${_fd})
            endif()
        endif()
    endforeach()

    set(_fehlt "")
    foreach(_d IN LISTS _ref)
        if(NOT "${_d}" MATCHES "^JUCE_")
            continue()
        endif()
        if("${_d}" MATCHES "^JUCE_MODULE_AVAILABLE_"
           OR "${_d}" MATCHES "^JUCE_SHARED_CODE"
           OR "${_d}" MATCHES "^JUCE_STANDALONE_APPLICATION"
           OR "${_d}" MATCHES "^JUCE_VST3_CAN_REPLACE_VST2")
            continue()
        endif()
        if(NOT "${_d}" IN_LIST _haben)
            list(APPEND _fehlt "${_d}")
        endif()
    endforeach()

    if(_fehlt)
        string(REPLACE ";" "\n    " _liste "${_fehlt}")
        message(FATAL_ERROR
            "S8/SONDE-007a K2b: Der Kern '${kern}' uebersetzt die JUCE-Kopfdateien mit einer\n"
            "anderen Konfiguration als '${referenz}'. Fehlend am Kern:\n"
            "    ${_liste}\n"
            "Derselbe Header kann dort dann etwas anderes bedeuten als hier. Trag die\n"
            "Schraube an der Kopf-Fassade nach (plugin/CMakeLists.txt, NakamaKernJuce).")
    endif()

    message(STATUS "Nakama-Kern: K2b gruen — JUCE-Konfiguration von '${kern}' deckt '${referenz}'.")
endfunction()
