# S9 / SONDE-007b — die Identitaet kommt aus dem Manifest, nicht aus dem Bauskript.
#
# ── Was hier geloest wird (NAK-52) ──────────────────────────────────────────
#
# Entwurf §53.4 sagt woertlich: "Identitaet kommt ausschliesslich aus
# plugin-identities-v1.json ueber die duennen Target-Schichten." S8 hat die
# eine Haelfte dieses Satzes gebaut und bewiesen - der gemeinsame Kern sieht
# keine Identitaet. Die andere Haelfte stand aus: `juce_add_plugin(EqCopilot …)`
# trug COMPANY_NAME, PLUGIN_CODE, PRODUCT_NAME als Literale im Bauskript.
#
# Mit EINEM Ziel war das Zeremonie ohne Nutzen (NAK-52 sagt das selbst). Mit
# DREI Zielen ist es das Gegenteil: drei Bauskriptbloecke mit je vier
# Identitaetszeilen sind vier Stellen, an denen zwei Wahrheiten auseinander-
# laufen koennen - und Identitaet ist hier ein DATEIFORMAT. Aendert sich eine
# dieser Zeilen, verlieren bestehende FL-Projekte ihr Plugin.
#
# ── Was dieser Leser NICHT tut ──────────────────────────────────────────────
#
# Er erfindet nichts. Steht ein Feld im Manifest auf `null`, bricht der Bau ab,
# statt einen Vorgabewert einzusetzen: ein Ziel, dessen Identitaet noch nicht
# vergeben ist, darf nicht gebaut werden. Die Class-IDs liest er GAR NICHT -
# die leitet JUCE aus Hersteller- und Plugin-Code ab, und genau diese Ableitung
# misst EqCopIdentityTest am gebauten `moduleinfo.json` gegen das Manifest
# nach. Zwei Wege zur selben Zahl, sonst waere der Test eine Tautologie.
#
# ── Nacharbeit 23.08.2026 (T2-Befund T2-2, Manifest SONDE-007b §6.1) ────────
#
# Der Satz oben stand hier schon, war aber nur zu drei Fuenfteln wahr: die
# Null-Schleife deckte `produktname`, `bundle` und `plugin_code` - NICHT
# `hersteller.name` und `hersteller.code`. Der Herstellercode ist kein Beiwerk
# zum Viercode, er ist der ZWEITE SUMMAND derselben Class-ID (jucePluginId) und
# steht einmal fuer alle drei Ziele. Gemessen: mit `hersteller.code: null` lief
# der Configure gruen durch, und JUCE setzte still `Manu`
# (JucePlugin_ManufacturerCode=0x4d616e75) bzw. `yourcompany` ein - genau der
# Schaden, den der Absatz darueber ausschliesst.
#
# Beim Schliessen fiel eine zweite Luecke auf, gemessen statt vermutet
# (`cmake -P` auf einem Probe-JSON):
#
#   string(JSON … GET) liefert bei JSON-`null`  einen LEEREN String
#   und bei FEHLENDEM Schluessel (mit ERROR_VARIABLE)  `<membername>-NOTFOUND`
#   - also den Namen des gesuchten Feldes, nicht den der Ausgabevariablen.
#
# Die alten Vergleiche gegen die Literale "null" und "NOTFOUND" konnten damit
# NIE zutreffen. Der leere String fing `null` mit ab; ein GELOESCHTER Schluessel
# aber rutschte durch. Gemessen am Leser von HEAD: `produktname` entfernt ⇒
# Configure Exitcode 0 und `JucePlugin_Name="produktname-NOTFOUND"` im
# erzeugten Projekt. Die Schleife prueft jetzt auf das, was CMake wirklich
# zurueckgibt.

include_guard(GLOBAL)

set(NAKAMA_IDENTITAET_DATEI "${CMAKE_CURRENT_LIST_DIR}/../identity/plugin-identities-v1.json"
    CACHE INTERNAL "Eingefrorene Pluginidentitaeten (SONDE-001)")

# Liest den Eintrag `ziel_id` und legt die Werte unter <praefix>_* im
# aufrufenden Bereich ab:
#
#   <praefix>_HERSTELLER       Firmenname   (COMPANY_NAME)
#   <praefix>_HERSTELLERCODE   Viercode     (PLUGIN_MANUFACTURER_CODE)
#   <praefix>_PLUGINCODE       Viercode     (PLUGIN_CODE)
#   <praefix>_PRODUKTNAME      Produktname  (PRODUCT_NAME)
#   <praefix>_BUNDLE           Bundlename inkl. .vst3 (nur zum Nachmessen)
#   <praefix>_KATEGORIEN       CMake-Liste  (VST3_CATEGORIES)
#
# Der Bau haengt an der Datei: aendert sie sich, konfiguriert CMake neu.
function(nakama_identitaet_lesen ziel_id praefix)
    if(NOT EXISTS "${NAKAMA_IDENTITAET_DATEI}")
        message(FATAL_ERROR
            "S9/SONDE-007b: Identitaetsmanifest nicht gefunden:\n"
            "  ${NAKAMA_IDENTITAET_DATEI}\n"
            "Ohne das Manifest gibt es keine Identitaet - und geraten wird sie nicht.")
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${NAKAMA_IDENTITAET_DATEI}")

    file(READ "${NAKAMA_IDENTITAET_DATEI}" _js)

    # ERROR_VARIABLE, damit ein FEHLENDER Schluessel nicht mit CMakes eigener
    # Meldung abbricht, sondern unten durch dieselbe Schleife laeuft wie
    # `null` - eine Erklaerung, nicht zwei.
    string(JSON _hersteller     ERROR_VARIABLE _e4 GET "${_js}" hersteller name)
    string(JSON _herstellercode ERROR_VARIABLE _e5 GET "${_js}" hersteller code)

    # Das Ziel wird GESUCHT, nicht ueber einen Index gegriffen: die Reihenfolge
    # im Manifest ist keine Zusage, und ein stiller Griff daneben waere die
    # schlimmste Art von Fehler, die diese Datei haben kann.
    string(JSON _anzahl LENGTH "${_js}" ziele)
    # T1-Selbstaudit 23.08. (numerische Raender): `foreach(x RANGE -1)` laeuft
    # NICHT null Mal, sondern ueber '0;-1' - gemessen mit `cmake -P`. Bei
    # leerem `ziele` griffe die Schleife also auf Index 0 zu und braeche mit
    # CMakes eigener Meldung ab statt mit dieser hier. Der Abbruch waere
    # richtig, die Erklaerung waere es nicht.
    if(_anzahl EQUAL 0)
        message(FATAL_ERROR
            "S9/SONDE-007b: Das Identitaetsmanifest enthaelt kein einziges Ziel\n"
            "(${NAKAMA_IDENTITAET_DATEI}).\n"
            "Ohne eingefrorene Ziele gibt es keine Identitaet - und geraten wird sie nicht.")
    endif()
    math(EXPR _letzter "${_anzahl} - 1")
    set(_eintrag "")
    set(_bekannte "")
    foreach(_i RANGE ${_letzter})
        string(JSON _z GET "${_js}" ziele ${_i})
        string(JSON _id GET "${_z}" id)
        list(APPEND _bekannte "${_id}")
        if(_id STREQUAL "${ziel_id}")
            set(_eintrag "${_z}")
        endif()
    endforeach()

    if(_eintrag STREQUAL "")
        string(REPLACE ";" ", " _liste "${_bekannte}")
        message(FATAL_ERROR
            "S9/SONDE-007b: Kein Ziel '${ziel_id}' in ${NAKAMA_IDENTITAET_DATEI}.\n"
            "Bekannt sind: ${_liste}")
    endif()

    string(JSON _plugincode GET "${_eintrag}" plugin_code)
    string(JSON _produktname ERROR_VARIABLE _e1 GET "${_eintrag}" produktname)
    string(JSON _bundle      ERROR_VARIABLE _e2 GET "${_eintrag}" bundle)

    # `null` heisst "noch nicht vergeben" - und damit "nicht baubar". Ein
    # Vorgabewert waere eine erfundene Identitaet, und die ueberlebt jeden
    # spaeteren Riegel, weil sie dann schon im ausgelieferten Bundle steht.
    #
    # Die BEIDEN Herstellerfelder stehen seit 23.08. mit in der Schleife (T2-2):
    # sie gelten fuer alle drei Ziele gleichzeitig, und der Herstellercode ist
    # die eine Haelfte jeder Class-ID.
    foreach(_paar "hersteller.name:${_hersteller}"
                  "hersteller.code:${_herstellercode}"
                  "produktname:${_produktname}"
                  "bundle:${_bundle}"
                  "plugin_code:${_plugincode}")
        string(REGEX REPLACE "^([^:]+):(.*)$" "\\1" _feld "${_paar}")
        string(REGEX REPLACE "^([^:]+):(.*)$" "\\2" _wert "${_paar}")
        # Gemessen, nicht geraten (siehe Kopf): JSON-`null` kommt als LEERER
        # String zurueck, ein fehlender Schluessel als `<varname>-NOTFOUND`.
        if(_wert STREQUAL "" OR _wert MATCHES "-NOTFOUND$")
            message(FATAL_ERROR
                "S9/SONDE-007b: Ziel '${ziel_id}' hat kein '${_feld}' im Identitaetsmanifest\n"
                "(${NAKAMA_IDENTITAET_DATEI}).\n"
                "Ein reserviertes Ziel wird NICHT mit Vorgabewerten gebaut - trag die Zeile\n"
                "erst im Manifest ein, dann baue. Sonst traegt das ausgelieferte Bundle eine\n"
                "Identitaet, die nirgends eingefroren ist.")
        endif()
    endforeach()

    # Beide Viercodes, nicht nur einer: JUCE bildet die Class-ID aus
    # jucePluginId(herstellercode, plugincode, typ). Ein dreistelliger
    # Herstellercode ergaebe dieselbe stille Verschiebung wie ein
    # dreistelliger Plugincode - JUCE fuellt selbst auf, das Manifest nicht.
    foreach(_codepaar "plugin_code:${_plugincode}" "hersteller.code:${_herstellercode}")
        string(REGEX REPLACE "^([^:]+):(.*)$" "\\1" _codefeld "${_codepaar}")
        string(REGEX REPLACE "^([^:]+):(.*)$" "\\2" _codewert "${_codepaar}")
        string(LENGTH "${_codewert}" _len)
        if(NOT _len EQUAL 4)
            message(FATAL_ERROR
                "S9/SONDE-007b: ${_codefeld} von '${ziel_id}' ist '${_codewert}' (${_len} Zeichen).\n"
                "VST3-Viercodes sind genau vier Zeichen; JUCE baut daraus die Class-ID.")
        endif()
    endforeach()

    string(JSON _katanzahl ERROR_VARIABLE _e3 LENGTH "${_eintrag}" kategorien)
    set(_kategorien "")
    if(_katanzahl AND NOT _katanzahl STREQUAL "NOTFOUND")
        math(EXPR _katletzter "${_katanzahl} - 1")
        foreach(_k RANGE ${_katletzter})
            string(JSON _kat GET "${_eintrag}" kategorien ${_k})
            list(APPEND _kategorien "${_kat}")
        endforeach()
    endif()
    if(NOT _kategorien)
        message(FATAL_ERROR
            "S9/SONDE-007b: Ziel '${ziel_id}' hat keine 'kategorien' im Identitaetsmanifest.\n"
            "Die VST3-Kategorie steht im gebauten moduleinfo.json - sie ist Teil dessen,\n"
            "was der Host sieht, und wird nicht geraten.")
    endif()

    set(${praefix}_HERSTELLER     "${_hersteller}"     PARENT_SCOPE)
    set(${praefix}_HERSTELLERCODE "${_herstellercode}" PARENT_SCOPE)
    set(${praefix}_PLUGINCODE     "${_plugincode}"     PARENT_SCOPE)
    set(${praefix}_PRODUKTNAME    "${_produktname}"    PARENT_SCOPE)
    set(${praefix}_BUNDLE         "${_bundle}"         PARENT_SCOPE)
    set(${praefix}_KATEGORIEN     "${_kategorien}"     PARENT_SCOPE)

    string(REPLACE ";" " " _kattext "${_kategorien}")
    message(STATUS
        "Nakama-Identitaet: '${ziel_id}' aus dem Manifest — ${_produktname} "
        "(${_herstellercode}/${_plugincode}), Bundle ${_bundle}, Kategorien ${_kattext}.")
endfunction()
