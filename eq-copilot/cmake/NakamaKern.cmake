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
# ⚠️ BERICHTIGT 24.08.2026 (G1-Nacharbeit). Hier stand seit dem 22.08.:
# "heute weicht genau eine ab (JUCE_USE_CURL; der Kern saehe den Vorgabewert 1
# aus juce_core.h:152 statt der 0 des Projekts), und sie ist folgenlos."
#
# Das ist GEMESSEN FALSCH, und der Fehler ist lehrreich: die Abweichung war nie
# eine Eigenschaft des Baus, sondern eine Eigenschaft des BLICKS. Die
# Kopf-Fassade traegt ihre Defines als Generatorausdruck; zur Konfigurierzeit
# las der alte Riegel davon nur unaufgeloesten Text und sah deshalb KEIN
# JUCE_USE_CURL am Kern. Beim Uebersetzen expandiert derselbe Ausdruck sehr
# wohl - der Kern hatte die 0 also immer. Aus einer Messluecke wurde eine
# Aussage ueber das Produkt, und die stand hier zwei Tage als Befund.
# Seit die Fassade aufgeloest wird, meldet der Riegel beide Seiten literal:
# JUCE_DISPLAY_SPLASH_SCREEN=0, JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1,
# JUCE_USE_CURL=0, JUCE_WEB_BROWSER=0 - vier Defines, beide Seiten gleich.
#
# ⚠️ Und der zweite Teil desselben Befunds (G1 §4.4): der Satz "haelt die
# beiden Mengen zusammen" stand hier, seit es den Riegel gibt — er war eine
# Zusage auf MENGENGLEICHHEIT, und der Code darunter leistete sie nicht. Die
# tragende Schleife lief NUR ueber die Referenz und fragte nur "fehlt es am
# Kern?".
# Damit war unsichtbar:
#
#   * ein Define, das es NUR am Kern gibt        -> derselbe Header, andere
#                                                   Bedeutung, kein Alarm
#   * ein Name mit ZWEI Werten am Kern           -> der erwartete Wert stand ja
#                                                   drin, nur nicht allein
#
# Das ist genau die Befundklasse, fuer die S8 gebaut wurde: ein Riegel, der
# weniger kann, als er behauptet, und dessen Schweigen deshalb nichts beweist.
# Er misst jetzt in BEIDE Richtungen und zusaetzlich auf Wertwiderspruch; die
# Zusage oben stimmt seitdem.
#
# K2b vergleicht DEFINES. Uebersetzungsschalter liegen ausserhalb seiner
# Reichweite - dafuer steht K2c darunter (T2-Befund 23.08.).
#
# Nicht verglichen wird, was legitim verschieden ist:
#   JUCE_MODULE_AVAILABLE_*  - der Kern nutzt weniger Module, das ist sein Sinn
#   JUCE_SHARED_CODE / JUCE_STANDALONE_APPLICATION / JUCE_VST3_CAN_REPLACE_VST2
#                            - Sache der Plugin-Huelle. K1 nennt davon nur
#                              JUCE_SHARED_CODE namentlich; die anderen beiden
#                              haelt K2 auf, denn ihre WERTE tragen den Praefix
#                              (JUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone,
#                              nachzulesen in der Rohausgabe B3 des Manifests).
#                              Bis 23.08. stand hier "K1 verbietet die ersten
#                              beiden" - das war falsch; der Ausschluss haelt,
#                              aber ein anderer Riegel traegt ihn.
# Sammelt die WIRKSAMEN JUCE-Defines eines Ziels: seine eigenen plus die
# INTERFACE-Defines aller Ziele, gegen die es linkt. Beide Seiten des Vergleichs
# werden damit GLEICH gerechnet — das ist die Voraussetzung dafuer, dass ein
# Mengenvergleich in beide Richtungen ueberhaupt aussagekraeftig ist.
#
# ⚠️ Vor dem 24.08.2026 war genau das nicht so: die Referenzseite wurde nur aus
# COMPILE_DEFINITIONS gelesen, die Kernseite zusaetzlich aus der Fassade. Zwei
# verschieden gerechnete Mengen kann man nur in EINE Richtung vergleichen, ohne
# Fehlalarme zu ernten — und genau in eine Richtung wurde verglichen.
function(_nakama_kern_wirksame_defines ziel aus)
    get_target_property(_eigene "${ziel}" COMPILE_DEFINITIONS)
    get_target_property(_links "${ziel}" LINK_LIBRARIES)

    set(_alle "")
    if(_eigene)
        list(APPEND _alle ${_eigene})
    endif()
    foreach(_l IN LISTS _links)
        if(NOT TARGET "${_l}")
            continue()
        endif()

        # 🚨 Die Kopf-Fassade traegt ihre Defines als GENERATORAUSDRUCK
        # ($<TARGET_PROPERTY:...>, siehe nakama_kern_juce_fassade). Zur
        # Konfigurierzeit liefert get_target_property davon den unaufgeloesten
        # Text — kein einziges JUCE_-Literal. Wer die beiden Seiten naiv
        # vergleicht, haelt eine literale Menge gegen eine unaufgeloeste und
        # bekommt Fehlalarme fuer JEDES Define, das nur ueber die Fassade
        # kommt (gemessen 24.08.: JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1).
        #
        # Deshalb wird die Fassade ueber dieselbe Herkunftsspur aufgeloest,
        # die K2 schon benutzt, und zwar aus demselben Grund, den der Kommentar
        # dort nennt: "Generatorausdruecke sind zur Konfigurierzeit keine
        # Kante." Danach sind BEIDE Seiten literal — und erst dann ist ein
        # Mengenvergleich in beide Richtungen ueberhaupt eine Aussage.
        set(_quellen "${_l}")
        get_target_property(_abgeleitet "${_l}" NAKAMA_KERN_ABGELEITET_VON)
        if(_abgeleitet)
            list(APPEND _quellen ${_abgeleitet})
        endif()

        foreach(_q IN LISTS _quellen)
            if(TARGET "${_q}")
                get_target_property(_ld "${_q}" INTERFACE_COMPILE_DEFINITIONS)
                if(_ld)
                    list(APPEND _alle ${_ld})
                endif()
            endif()
        endforeach()
    endforeach()

    # Nur JUCE-Konfiguration, und nur das, was legitim verschieden sein DARF,
    # faellt raus (Begruendung im Kopf dieses Abschnitts).
    set(_gefiltert "")
    foreach(_d IN LISTS _alle)
        if(NOT "${_d}" MATCHES "^JUCE_")
            continue()
        endif()
        if("${_d}" MATCHES "^JUCE_MODULE_AVAILABLE_"
           OR "${_d}" MATCHES "^JUCE_SHARED_CODE"
           OR "${_d}" MATCHES "^JUCE_STANDALONE_APPLICATION"
           OR "${_d}" MATCHES "^JUCE_VST3_CAN_REPLACE_VST2")
            continue()
        endif()
        list(APPEND _gefiltert "${_d}")
    endforeach()
    list(REMOVE_DUPLICATES _gefiltert)
    list(SORT _gefiltert)
    set(${aus} "${_gefiltert}" PARENT_SCOPE)
endfunction()

# Findet Namen, die in EINER Menge mit ZWEI verschiedenen Werten stehen.
#
# ⚠️ Diese Klasse war vor dem 24.08. voellig unsichtbar, und zwar aus einem
# subtilen Grund: der einseitige Test fragte "steht der erwartete Wert in der
# Kernmenge?". Bei `JUCE_USE_CURL=0` UND `JUCE_USE_CURL=1` am Kern lautet die
# Antwort JA — der erwartete Wert steht ja da, nur eben nicht allein. Der
# Uebersetzer sieht dann zwei -D fuer denselben Namen; welcher gewinnt, haengt
# an der Reihenfolge.
function(_nakama_kern_widersprueche menge aus)
    set(_namen "")
    set(_treffer "")
    foreach(_d IN LISTS menge)
        string(REGEX REPLACE "=.*$" "" _n "${_d}")
        if("${_n}" IN_LIST _namen)
            list(APPEND _treffer "${_n}")
        else()
            list(APPEND _namen "${_n}")
        endif()
    endforeach()
    if(_treffer)
        list(REMOVE_DUPLICATES _treffer)
    endif()
    set(${aus} "${_treffer}" PARENT_SCOPE)
endfunction()

function(nakama_kern_konfig_pruefen kern referenz)
    _nakama_kern_wirksame_defines("${referenz}" _ref)
    _nakama_kern_wirksame_defines("${kern}" _haben)

    # Richtung 1 (gab es schon): hat der Kern alles, was der Verbraucher hat?
    set(_fehlt "")
    foreach(_d IN LISTS _ref)
        if(NOT "${_d}" IN_LIST _haben)
            list(APPEND _fehlt "${_d}")
        endif()
    endforeach()

    # Richtung 2 (NEU, G1-Befund §4.4): hat der Kern etwas, was der Verbraucher
    # NICHT hat? Der Kopf dieses Riegels sagt seit jeher, er halte "die beiden
    # Mengen zusammen" — das ist eine Zusage auf MENGENGLEICHHEIT, und
    # einseitige Enthaltung (_ref ⊆ _haben) leistet sie nicht. Ein Define, das
    # NUR am Kern haengt, laesst denselben Header dort etwas anderes bedeuten
    # als beim Verbraucher; die Richtung des Unterschieds ist dafuer egal.
    set(_zuviel "")
    foreach(_d IN LISTS _haben)
        if(NOT "${_d}" IN_LIST _ref)
            list(APPEND _zuviel "${_d}")
        endif()
    endforeach()

    # Richtung 3 (NEU): derselbe Name zweimal mit verschiedenem Wert.
    _nakama_kern_widersprueche("${_haben}" _wider_kern)
    _nakama_kern_widersprueche("${_ref}" _wider_ref)

    set(_klagen "")
    if(_fehlt)
        string(REPLACE ";" "\n        " _l "${_fehlt}")
        list(APPEND _klagen "  fehlt am Kern (hat der Verbraucher, der Kern nicht):\n        ${_l}")
    endif()
    if(_zuviel)
        string(REPLACE ";" "\n        " _l "${_zuviel}")
        list(APPEND _klagen "  nur am Kern (hat der Kern, der Verbraucher nicht):\n        ${_l}")
    endif()
    if(_wider_kern)
        string(REPLACE ";" "\n        " _l "${_wider_kern}")
        list(APPEND _klagen "  widerspruechlich AM KERN (ein Name, zwei Werte):\n        ${_l}")
    endif()
    if(_wider_ref)
        string(REPLACE ";" "\n        " _l "${_wider_ref}")
        list(APPEND _klagen "  widerspruechlich AM VERBRAUCHER (ein Name, zwei Werte):\n        ${_l}")
    endif()

    if(_klagen)
        string(REPLACE ";" "\n" _text "${_klagen}")
        message(FATAL_ERROR
            "S8/SONDE-007a K2b: Der Kern '${kern}' uebersetzt die JUCE-Kopfdateien mit einer\n"
            "anderen Konfiguration als '${referenz}':\n"
            "${_text}\n"
            "Derselbe Header kann dort dann etwas anderes bedeuten als hier. Trag die\n"
            "Schraube an der Kopf-Fassade nach (plugin/CMakeLists.txt, NakamaKernJuce).")
    endif()

    # Der Riegel nennt, WAS er verglichen hat. Ein Riegel, der nur "gruen"
    # sagt, macht seinen eigenen Umfang unsichtbar - und ein Riegel, der nichts
    # findet, sagt nichts, solange nicht gezeigt ist, dass er ueberhaupt etwas
    # finden koennte (Lehre A14/SONDE-007a). Schrumpft die Menge hier still auf
    # null, faellt es in der Konfigurierausgabe auf.
    list(LENGTH _ref _n)
    string(REPLACE ";" ", " _gemessen "${_ref}")
    message(STATUS
        "Nakama-Kern: K2b gruen — Mengen von '${kern}' und '${referenz}' GLEICH "
        "(beide Richtungen, keine Wertwidersprueche); ${_n} verglichen: ${_gemessen}")
endfunction()

# ── K2c: dieselben JUCE-Empfehlungsschalter wie der Verbraucher ─────────────
# T2-Befund 23.08. K2b vergleicht COMPILE_DEFINITIONS und ist damit blind fuer
# Uebersetzungsschalter. Genau dort riss der Umbau ein Loch: die Helferziele
# juce_recommended_*_flags haengen PUBLIC an den Verbraucherzielen, aber die
# Kopf-Fassade leitet nur aus MODUL-Zielen ab. Bis 23.08. uebersetzte der Kern
# deshalb als einziger Code im Baum unter /W1 statt /W4 - gemessen an den
# erzeugten .vcxproj: 0 Zeilen <WarningLevel> gegen 4x <WarningLevel>Level4 bei
# jedem Verbraucher.
#
# Der Riegel fragt nicht nach einzelnen Schaltern (die sind versionsabhaengig
# und stecken in Generatorausdruecken), sondern nach ihrer QUELLE: welches
# juce_recommended_*-Ziel die Referenz traegt, muss auch der Kern tragen.
#
# EINE Ausnahme, und sie ist begruendet, nicht bequem:
#   juce_recommended_lto_flags - setzt /GL und verlangt -LTCG am Endlink. Nur
#     EqCopilot linkt es; die Konsolenziele (NullTest, SchemaTest, ...) linken
#     den Kern OHNE -LTCG. /GL-Objekte im Kern wuerden dort auf einen Linker
#     ohne LTCG treffen. Der Kern muss dieses Ziel also NICHT tragen.
function(nakama_kern_schalter_pruefen kern referenz)
    _nakama_kern_huelle("${kern}" _kern_huelle)
    _nakama_kern_huelle("${referenz}" _ref_huelle)

    set(_ausgenommen juce_recommended_lto_flags)

    set(_fehlt "")
    set(_gedeckt "")
    foreach(_t IN LISTS _ref_huelle)
        if(NOT "${_t}" MATCHES "^juce_recommended_")
            continue()
        endif()
        if("${_t}" IN_LIST _ausgenommen)
            continue()
        endif()
        if("${_t}" IN_LIST _kern_huelle)
            list(APPEND _gedeckt "${_t}")
        else()
            list(APPEND _fehlt "${_t}")
        endif()
    endforeach()

    if(_fehlt)
        string(REPLACE ";" "\n    " _liste "${_fehlt}")
        message(FATAL_ERROR
            "S8/SONDE-007a K2c: Der Kern '${kern}' uebersetzt ohne Empfehlungsschalter, die\n"
            "'${referenz}' traegt. Fehlend am Kern:\n"
            "    ${_liste}\n"
            "Vor S8 lagen die Kernquellen IN den Verbrauchern und erbten diese Schalter (PUBLIC).\n"
            "Als eigene Lib erbt der Kern sie nicht mehr: die Kopf-Fassade leitet nur aus\n"
            "MODUL-Zielen ab, und die Empfehlungsziele sind keine Module. Trag sie am Kern nach\n"
            "(plugin/CMakeLists.txt, target_link_libraries(NakamaKern PRIVATE …)).")
    endif()

    list(LENGTH _gedeckt _anzahl)
    string(REPLACE ";" ", " _namen "${_gedeckt}")
    message(STATUS
        "Nakama-Kern: K2c gruen — ${_anzahl} Empfehlungsschalter von '${referenz}' auch am Kern "
        "(${_namen}); ausgenommen: juce_recommended_lto_flags (/GL ohne -LTCG im Verbraucher).")
endfunction()
