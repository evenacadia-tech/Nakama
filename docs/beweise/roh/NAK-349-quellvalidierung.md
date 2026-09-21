VALIDIERUNG: 3 bestätigt, 3 präzisiert, 0 widerlegt

# NAK-349 — Quellvalidierung: das Kennungskonflikt-Panel ruft den Editor über rohes `this`

Frischer, lesender Validierer (Claude Opus 5), 21.09.2026; Bericht geschrieben ab 20:39:32 (`date`). Auftrag `docs/beweise/roh/NAK-349-quellvalidierung-auftrag.txt`, Gegenstand Register NAK-349 (`docs/offene-punkte.md:352`), Herkunft `docs/beweise/roh/NAK-312-etappe-4-erstpruefung-validierung.md:63` (V-4 Punkt 5). Alle Zeilen gelten am HEAD `9f6c0f07`. `git diff --stat 94f54e4b HEAD -- eq-copilot/plugin/src/PluginEditor.cpp eq-copilot/plugin/src/PluginEditor.h eq-copilot/plugin/tests/ShotTestMain.cpp` ist leer: die Zeilen der Etappe-4-Validierung gelten dort unverändert. JUCE-Quelle `eq-copilot/build/_deps/juce-src`, Tag 8.0.9 (`f72bad64`); unter `modules/` ist allein `juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp` lokal geändert (Brückenpatch, Hunks ab neuer Zeile 70 und ab 2661, keiner in der Editoransicht `:1925-2530`); zitiert wird die gebaute, gepatchte Datei. JUCE-Pfade stehen ab `modules/`. Gelesen und gerechnet, gemessen wurde nichts. Einzige externe Quelle: Microsoft Learn, `GetMessage`, Abschnitt Remarks (V-2).

## V-1 — Weg am Code: BESTÄTIGT

**`zeigeKonflikt()` vollständig** (`eq-copilot/plugin/src/PluginEditor.cpp:535-599`):

```cpp
// M2, Plan §8.4: die sichtbare Entscheidung beim Kennungs-Konflikt. Zwei
// Instanzen melden dieselbe Sensor-ID (typisch: Plugin in FL dupliziert) —
// der User gibt DIESER Instanz eine neue Kennung, die andere behält ihre.
void EqCopilotEditor::zeigeKonflikt()
{
    const float s = ui();

    struct KonfliktPanel : juce::Component
    {
        juce::Label text;
        juce::TextButton neuKnopf;
        std::function<void()> tun;

        KonfliktPanel (const juce::String& t, float skala, std::function<void()> aktion)
            : tun (std::move (aktion))
        {
            text.setText (t, juce::dontSendNotification);
            text.setFont (uiFont (13.0f * skala));
            text.setColour (juce::Label::textColourId, farbe (leitstand::copilot_text_light));
            text.setJustificationType (juce::Justification::topLeft);
            addAndMakeVisible (text);

            neuKnopf.setButtonText (u8 ("Dieser Instanz eine neue Kennung geben"));
            neuKnopf.setColour (juce::TextButton::buttonColourId, farbe (leitstand::copilot_fault));
            neuKnopf.setColour (juce::TextButton::textColourOffId, farbe (leitstand::copilot_text_light));
            neuKnopf.onClick = [this]
            {
                if (tun)
                    tun();
                if (auto* box = findParentComponentOfClass<juce::CallOutBox>())
                    box->dismiss();
            };
            addAndMakeVisible (neuKnopf);
            setSize ((int) (460 * skala), (int) (190 * skala));
        }

        void resized() override
        {
            auto r = getLocalBounds().reduced (8);
            neuKnopf.setBounds (r.removeFromBottom (juce::jmax (28, getHeight() / 6)));
            r.removeFromBottom (8);
            text.setBounds (r);
        }
    };

    auto inhalt = std::make_unique<KonfliktPanel> (
        u8 ("Zwei Messpunkte melden dieselbe Kennung.\n\n"
            "Das passiert, wenn das Plugin in FL dupliziert wurde — beide Kopien "
            "tragen dann dieselbe innere Kennung, und die App kann sie nicht "
            "auseinanderhalten.\n\n"
            "Wenn DIESE Instanz ein eigener Messpunkt sein soll: gib ihr unten "
            "eine neue Kennung. Die andere Instanz behält ihre — dort musst du "
            "nichts tun."),
        s,
        [this]
        {
            statusMeldung = processor.neueSensorId()
                ? u8 ("Neue Kennung vergeben — dieser Messpunkt meldet sich frisch an.")
                : juce::String ("State read-only: " + processor.holeStateGrund() + ". No new identity assigned.");
            statusMeldungBisMs = juce::Time::getMillisecondCounter() + 6000;
        });

    juce::CallOutBox::launchAsynchronously (std::move (inhalt),
                                            konfliktKnopf.getBounds(), this);
}
```

**Entstehung und Besitz.**

- Das Panel ist eine lokale Struktur `KonfliktPanel : juce::Component` mit `juce::Label text`, `juce::TextButton neuKnopf` und `std::function<void()> tun` (`:542-546`), ohne Destruktor (`:542-578`). Es entsteht als `std::make_unique<KonfliktPanel>` (`:580-595`) und geht mit `juce::CallOutBox::launchAsynchronously (std::move (inhalt), konfliktKnopf.getBounds(), this)` weg (`:597-598`); die zurückgegebene Box wird verworfen.
- `launchAsynchronously` legt `new CallOutBoxCallback (std::move (content), area, parent)` an und gibt dessen Mitglied `callout` zurück (`modules/juce_gui_basics/windows/juce_CallOutBox.cpp:89-94`). Mitglieder: `std::unique_ptr<Component> content;` und `CallOutBox callout;` (`:83-84`). Mit Elternteil hängt sich die Box als Kind an den Editor (`parent->addChildComponent (this)`, `:43-47`) und nimmt das Panel als eigenes Kind (`:41`). Der Callback ruft `callout.enterModalState (true, this)` und startet einen 200-ms-Timer (`:71-72`).
- `enterModalState` meldet die Box beim Modal-Manager an und hängt den Callback daran (`modules/juce_gui_basics/components/juce_Component.cpp:1481-1483`); `deleteWhenDismissed` ist per Vorgabe `false` (`modules/juce_gui_basics/components/juce_Component.h:2221-2223`). `startModal` legt ein `ModalItem` an (`modules/juce_gui_basics/components/juce_ModalComponentManager.cpp:112-119`), `attachCallback` gibt ihm den Callback in `OwnedArray<Callback> callbacks` (`:121-139`, `:93`).
- Besitzkette: Modal-Manager → `ModalItem` → `CallOutBoxCallback` → `content` (Panel) und `callout` (Box). Der Editor ist nur Elternkomponente der Box; er besitzt weder Box noch Panel.

**Der Rückruf und was er fängt.** Übergeben wird das Lambda `PluginEditor.cpp:589-595`, im Panel als `tun` gespeichert (`:548-549`). Es fängt allein rohes `this` (`EqCopilotEditor*`): keinen `SafePointer`, keine Referenz auf den Prozessor, keine Kopie von Zustand. Zum Vergleich fangen Hinweisliste und Messpunktpanel einen `SafePointer` (`:505-507`, `:773`).

**Was er anfasst.**

- Am Editor: das Referenzmitglied `processor` (`EqCopilotProcessor& processor;`, `eq-copilot/plugin/src/PluginEditor.h:117`), `statusMeldung` (`juce::String`, `:168`) und `statusMeldungBisMs` (`juce::uint32`, `:169`). Kein `uiDirty`, kein `repaint()`, keine Modelle (`sourcesAnzeige`, `anzeige`, `befunde`).
- Am Prozessor: `neueSensorId()` (`eq-copilot/plugin/src/prozessor/State.cpp:54-67`) — `bindungMutex`, `zustand.nurLesen`, neue `zustand.common.instanceId`, `meldeHostDirty()`, `v3StateRevision.fetch_add (1)`, `pipe.reconnect()`, `controlV3.reconnect()`; im read-only-Fall (`:58-59`) zusätzlich `holeStateGrund()` (`:242-246`: `bindungMutex`, `zustand.grund`). `meldeHostDirty()` ist `updateHostDisplay (…withNonParameterStateChanged (true))` (`eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:160-163`).
- Im Panel selbst: `neuKnopf.onClick` ruft `tun()` und danach `dismiss()` der umgebenden Box (`PluginEditor.cpp:560-566`); dieses `this` ist das Panel.
- Herkunft (`git blame -L 589,598`): Rahmen des Lambdas aus `31c41244` (18.08.2026, Einzug des Workspace), die Statuszeilen `PluginEditor.cpp:591-593` aus `ad162345` (22.08.2026, SONDE-006).

**Aus welchen Ereignissen er läuft.** `onClick` ruft allein `Button::sendClickMessage` (`modules/juce_gui_basics/buttons/juce_Button.cpp:410-434`, Aufruf `:433`), erreicht über `internalClickCallback` (`:364-378`):

- **Mausklick, synchron:** `mouseUp` ruft `internalClickCallback (e.mods)` direkt (`:484-502`, Aufruf `:497`); `mouseDown` klickt nur mit `triggerOnMouseDown` (`:479-480`), das hier nicht gesetzt ist.
- **Enter, gepostet:** `Button::keyPressed` ruft bei `returnKey` `triggerClick()` (`:665-674`); das ist `postCommandMessage (clickMessageId)` (`:359-362`). `Component::postCommandMessage` stellt über `MessageManager::callAsync` ein Lambda mit `WeakReference<Component>` auf den Knopf ein (`juce_Component.cpp:2096-2103`). Zugestellt ruft `Button::handleCommandMessage` bei `isEnabled()` `flashButtonState()` und `internalClickCallback` (`juce_Button.cpp:390-404`). Die Leertaste löst nichts aus (`keyPressed` prüft nur `returnKey`, `:667`).
- **Barrierefreiheit „press", gepostet:** `AccessibilityActionType::press` ruft ebenfalls `button.triggerClick()` (`modules/juce_gui_basics/detail/juce_ButtonAccessibilityHandler.h:108-111`).
- **Tastenkürzel:** klicken synchron nur als registrierte Shortcuts (`juce_Button.cpp:622-630`, `:641-663`); `neuKnopf` hat keine (`PluginEditor.cpp:557-567`).
- **Fokus:** `focusGained` und `focusLost` rufen nur `updateState()` und `repaint()` (`juce_Button.cpp:521-531`); aus Fokus entsteht kein Klick.
- **Wie Enter den Knopf erreicht:** `WM_KEYDOWN` und `WM_CHAR` → `doKeyDown`/`doKeyChar` → `handleKeyPress` (`modules/juce_gui_basics/native/juce_Windowing_windows.cpp:4136-4157`, `:3295-3317`, `:3323-3367`) → `ComponentPeer::handleKeyPress` an die fokussierte Komponente und ihre Eltern (`modules/juce_gui_basics/windows/juce_ComponentPeer.cpp:174-187`, `:196-231`).
- **Der Fokus liegt nach dem Öffnen schon auf `neuKnopf`:** ein `Button` will den Tastaturfokus (`juce_Button.cpp:89`); Komponenten beginnen mit gelöschten Flags (`juce_Component.cpp:261-267`), das `Label` setzt ihn nur über `setEditable` (`modules/juce_gui_basics/widgets/juce_Label.cpp:38-48`, `:109-124`), das im Panel nicht gerufen wird (`PluginEditor.cpp:551-555`), und die Box setzt ihn nie (`juce_CallOutBox.cpp`, kein `setWantsKeyboardFocus`). `enterModalState (true, …)` ruft `grabKeyboardFocus()` (`juce_Component.cpp:1487-1488`); die Box gibt ihn an die erste fokussierbare Kindkomponente weiter (`juce_Component.cpp:2707-2738`; `modules/juce_gui_basics/keyboard/juce_KeyboardFocusTraverser.cpp:41-44`, `:75-82`), also an `neuKnopf`. Er behält ihn, sofern das Fenster den Systemfokus bekommt (`takeKeyboardFocus`, `juce_Component.cpp:2674-2705`, Prüfung `:2684`; Hostverhalten, nicht gemessen). Ein einziges Enter nach dem Öffnen löst damit den geposteten Klick aus.

Bestätigt: der Rückruf fasst den Editor ohne Lebendprüfung an und läuft aus `neuKnopf.onClick`; ein per Enter (ebenso per Barrierefreiheit) ausgelöster Klick läuft über eine gepostete Nachricht, ein Mausklick synchron.

## V-2 — Erreichbarkeit: PRÄZISIERT

**Gelesen: die Warteschlange.** Unter Windows hängt `InternalMessageQueue::postMessage` jede Nachricht an `messageQueue` und postet nur bei leerer Schlange eine Weckung `customMessageID` an das versteckte Fenster (`modules/juce_events/native/juce_Messaging_windows.cpp:91-112`). Dessen Fensterprozedur ruft `dispatchMessages()` (`:160-170`), das die ganze Schlange tauscht und in Einfügereihenfolge zustellt (`:228-247`). `callAsync` (`modules/juce_events/messages/juce_MessageManager.h:115-128`) und `AsyncUpdater::triggerAsyncUpdate` (`modules/juce_events/broadcasters/juce_AsyncUpdater.cpp:74-84`) enden beide in `MessageBase::post()` → `postMessageToSystemQueue` (`modules/juce_events/messages/juce_MessageManager.cpp:81-91`, `juce_Messaging_windows.cpp:279-288`): eine Schlange, erste rein, erste raus.

**Gerechnet: die Reihenfolge.**

1. Enter: der Klick steht in der Schlange (V-1).
2. Der Host baut die Ansicht ab: `removed()` ruft `component->removeFromDesktop()` und `component = nullptr` (`modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:2030-2051`, `:2035`, `:2044`); `~ContentWrapperComponent` meldet `editorBeingDeleted` (`:2322-2329`), mit dem Mitglied `pluginEditor` (`:2493`) stirbt der Editor (`PluginEditor.cpp:233-240`). In `~Component` (`juce_Component.cpp:271-292`) erreicht `componentBeingDeleted` (`:275`) das `ModalItem`, das als `ComponentMovementWatcher` an der Box und an allen ihren Eltern horcht (`modules/juce_gui_basics/layout/juce_ComponentMovementWatcher.cpp:38-46`, `:134-141`). Wegen `comp.isParentOf (component)` setzt es `autoDelete = false` und ruft `cancel()` → `triggerAsyncUpdate()` (`juce_ModalComponentManager.cpp:70-90`): die Aufräumnachricht steht **hinter** dem Klick. Danach nimmt `~Component` die Box aus der Kinderliste (`juce_Component.cpp:277-278`, `child->parentComponent = nullptr` `:1291`); liegt der Fokus in der Box, gibt `giveAwayKeyboardFocusInternal` ihn ab (`:1296-1300`), was am Knopf nur `updateState()` und `repaint()` auslöst. Box und Panel leben.
3. Zustellung, zuerst der Klick: die schwache Referenz auf `neuKnopf` hält (das Panel lebt als `content`, `juce_CallOutBox.cpp:83`), `isEnabled()` ist wahr, weil die Elternkette an der elternlosen Box endet (`juce_Component.cpp:2852-2856`) → `onClick` → `tun()` mit totem `this`. Danach `handleAsyncUpdate` (`juce_ModalComponentManager.cpp:196-215`): das Item wird entfernt und gelöscht (`:204`), `~ModalItem` löscht selbst nichts (`:47-51`), `OwnedArray` löscht den `CallOutBoxCallback`, dessen Mitglieder in umgekehrter Folge sterben — erst `callout`, dann `content` (`juce_CallOutBox.cpp:83-84`).

Die Reihenfolge des Registers hält an der Quelle.

**Präzisierung: wann das Fenster offen ist.**

- Der Editorabbau muss nach dem Einstellen des Klicks und vor seiner Zustellung liegen. Eine „Runde" ist ein Lauf von `dispatchMessages()`. Zwei Formen: (a) Abbau in einem Hostaufruf zwischen der Tastaturnachricht und der Windows-Nachricht, die die Weckung trägt — Klick und Aufräumnachricht liegen in derselben Runde, der Klick vorn; (b) Abbau innerhalb einer früher eingestellten JUCE-Nachricht derselben Runde — der Klick läuft in dieser Runde auf dem toten Editor, die Aufräumnachricht landet in der schon getauschten neuen Schlange (`juce_Messaging_windows.cpp:238`) und kommt eine Runde später.
- Die Weckung ist eine gepostete Windows-Nachricht (`PostMessage`, `juce_Messaging_windows.cpp:111`). Microsoft Learn, `GetMessage`, Remarks (gelesen 21.09.2026, https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage), wörtlich:

  > If no filter is specified, messages are processed in the following order:
  >
  > - Sent messages
  > - Posted messages
  > - Input (hardware) messages and system internal events
  > - Sent messages (again)
  > - WM_PAINT messages
  > - WM_TIMER messages
  >
  > To retrieve input messages before posted messages, use the *wMsgFilterMin* and *wMsgFilterMax* parameters.

  Pumpt FL ungefiltert, holt es nach der Enter-Taste die Weckung vor jeder späteren Eingabe, auch vor einem schon vorausgetippten Klick auf das Schließkreuz; der Klick läuft dann auf dem lebenden Editor. Zwei Nutzergesten „Enter, dann Fenster schließen" öffnen den Weg deshalb nicht.
- Offen ist er nur, wenn FL die Ansicht (1) in seiner eigenen Behandlung derselben Tastennachricht, (2) aus einer gesendeten oder schon vor der Weckung geposteten Nachricht, (3) aus einer gefilterten oder eigenen Pumpe, die Eingaben vor geposteten Nachrichten holt, oder (4) aus einem früheren JUCE-Rückruf derselben Runde abbaut. Alles Hostverhalten, nicht gemessen.
- Randbedingung, die das Fenster weiter verengt: wartet vor dem Klick schon eine Aufräumnachricht des Managers (`triggerAsyncUpdate` postet nur, wenn `shouldDeliver` 0 war, `juce_AsyncUpdater.cpp:80-83`), räumt sie Box und Panel vor dem Klick ab, und die schwache Referenz verwirft ihn (`juce_Component.cpp:2100`). Das setzt voraus, dass kurz davor ein anderes modales Item endete.

**Mausklick: nicht derselbe Weg.** `mouseUp` klickt synchron (`juce_Button.cpp:484-502`). Eine Mausnachricht erreicht `neuKnopf` nur über das Fenster des Editors, weil die Box mit Elternteil kein eigenes Fenster bekommt (`juce_CallOutBox.cpp:43-47`, kein `addToDesktop` in diesem Zweig); nach `removed()` ist dieses Fenster fort (`juce_audio_plugin_client_VST3.cpp:2035` → `juce_Component.cpp:517-538`). Ein Mausklick beginnt also immer bei lebendem Editor. Denkbar bleibt nur ein Abbau **innerhalb** des Rückrufs: `neueSensorId()` → `meldeHostDirty()` → `updateHostDisplay` → `audioProcessorChanged` → `componentRestarter.restart (flags)` (`juce_audio_plugin_client_VST3.cpp:1552-1558`) läuft auf dem Nachrichtenthread synchron (`modules/juce_audio_processors/format_types/juce_VST3Common.h:1639-1650`) in `setDirty (true)` und `restartComponent (flags)` (`juce_audio_plugin_client_VST3.cpp:1613-1622`). Baute FL dort die Ansicht ab, träfe die anschließende Zuweisung an `statusMeldung` einen toten Editor, bei Maus und Enter gleich. Nicht gemessen; die Quelle gibt dafür keinen Anhalt.

**Die Box, wenn Zielbereich oder Elternteil stirbt.**

- Zielbereich: `konfliktKnopf.getBounds()` wird als Wert in `targetArea` kopiert (`juce_CallOutBox.cpp:199-202`); zum Knopf besteht keine Verbindung.
- Elternteil (Editor): wie Schritt 2 — nur `cancel()` und eine gepostete Aufräumnachricht, kein synchrones Löschen (`autoDelete` ist ohnehin `false`; Box und Panel sind Unterobjekte des Callbacks).
- `dismiss()` postet nur (`juce_CallOutBox.cpp:183-186`) und wird gerufen aus `neuKnopf.onClick` (`PluginEditor.cpp:564-565`), über Escape (`juce_CallOutBox.cpp:188-197` → `inputAttemptWhenModal`) und vom 200-ms-Timer, wenn der Prozess nicht im Vordergrund ist (`:77-81`). `inputAttemptWhenModal` (`:140-163`) kommt nur aus einer Eingabe: Escape an der Box oder Maus beziehungsweise Tab an einer blockierten Komponente (`juce_Component.cpp:2232-2235`, `:2813-2816`); ohne Fenster kommt keine. Der Tod des Elternteils ruft keines von beiden.
- Komponentenhierarchie: die Box ist danach elternlos (`juce_Component.cpp:1291`) und lebt mit dem Panel bis `handleAsyncUpdate`.

## V-3 — Folge: PRÄZISIERT

**Berichtigung zum Register:** der Rückruf fasst nicht nur den Editor an. Über den aus dem toten Editor gelesenen `processor`-Verweis erreicht er auch den Prozessor und kann dort eine persistente Änderung mit Host-Dirty auslösen; der Prozessor lebt in diesem Moment nicht sicher.

**Zugriffe am toten Editor, in Ausführungsreihenfolge** (Lambda `PluginEditor.cpp:589-595`):

1. `this->processor` lesen (`PluginEditor.h:117`): der Verweis steht im Editorobjekt, gelesen wird freigegebener Speicher. Der Editor wurde mit `new` angelegt (`eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:182-185`) und vom `std::unique_ptr` des Wrappers freigegeben (`juce_audio_plugin_client_VST3.cpp:2493`).
2. `neueSensorId()` beziehungsweise `holeStateGrund()` auf dem Objekt an der gelesenen Adresse (V-1).
3. `statusMeldung = …`: der Bedingungsausdruck liefert einen `juce::String`-Wert (`u8` gibt `juce::String` zurück, `PluginEditor.cpp:19`), also Verschiebezuweisung `std::swap (text, other.text)` (`modules/juce_core/text/juce_String.cpp:283-287`). Sie liest den alten Pufferzeiger aus dem freigegebenen Speicher und schreibt den neuen hinein. Der Temporärwert gibt am Ende des Ausdrucks den alten Zeiger frei (`:250-253` → `release`, `:172-182`), den `~String` des Mitglieds beim Editorabbau schon freigegeben hat: ein zweites `release` auf einen freigegebenen oder inzwischen wiederverwendeten Puffer (Dekrement in freigegebenem Speicher; ob daraus ein zweites `delete[]` wird, hängt am Inhalt dort). War `statusMeldung` leer, ist der alte Zeiger der statische Leerstring, und `release` tut nichts (`:174`, `:225-228`). Der neue Puffer bleibt im toten Speicher stehen und wird nie freigegeben.
4. `statusMeldungBisMs = …`: vier Byte in freigegebenen Speicher.

Nicht angefasst: `uiDirty`, `repaint()`, Modelle, `konfliktKnopf`. Nach `tun()` postet das Panel nur `dismiss()` an die lebende Box (`PluginEditor.cpp:564-565`), harmlos.

**Urteil zum Speicher:** Zugriff auf freigegebenen Speicher, undefiniertes Verhalten, nicht harmlos. Ob es still durchläuft, den Heap beschädigt oder abstürzt, hängt an der Wiederverwendung des Speichers; nicht vorhersagbar, nicht gemessen. Steht im alten Speicher noch die Adresse eines lebenden Prozessors, läuft `neueSensorId()` wirklich: neue `instanceId`, eine Host-Dirty-Meldung über `updateHostDisplay` bis `setDirty` und `restartComponent` (Kette in V-2), `v3StateRevision` plus 1, Reconnect des v2- und des v3-Clients — Zugriff, Mutation und Dirty-Meldung nach dem Ende des Editors.

**Lebt der Prozessor in diesem Moment?** Nicht sicher.

- Nur das Fenster zu (Ansicht abgebaut, Instanz bleibt): er lebt. Die Komponente hält ihn über `comPluginInstance` (`juce_audio_plugin_client_VST3.cpp:2693`, Mitglied `:4021`), der Controller über `audioProcessor` (`:1599`) bis `terminate()` (`:907-915`, `:912`); `JuceAudioProcessor` besitzt ihn als `std::unique_ptr<AudioProcessor>` (`:774`).
- Instanz entfernt: beendet und gibt der Host Controller und Komponente in derselben Folge wie `removed()` frei, bevor JUCEs Schlange zugestellt wird, ist der Prozessor vor dem Klick tot, und der Rückruf ruft Methoden auf freigegebenem Prozessorspeicher (Mutex, `pipe`, `controlV3`). Der JUCE-Vertrag (Editor stirbt vor seinem Prozessor, `editorBeingDeleted`, `juce_audio_plugin_client_VST3.cpp:2327`; `docs/beweise/NAK-312.md:1477-1478`) ordnet nur die Reihenfolge der beiden Tode; über die Zeit nach dem Editor sagt er nichts.
- Letzte Instanz des Moduls: unter Windows halten Controller, Komponente und Ansicht je einen `ScopedJuceInitialiser_GUI` über `ScopedRunLoop` (`juce_audio_plugin_client_VST3.cpp:827-836`, `:1598`, `:3986`, `:2518`; der der Ansicht fällt in `removed()`, `:2048`). Fällt der letzte, löscht `shutdownJuce_GUI` erst die `DeletedAtShutdown`-Objekte, darunter den Modal-Manager mit Box und Panel (`modules/juce_gui_basics/components/juce_ModalComponentManager.h:51-52`, `juce_ModalComponentManager.cpp:105-109`), dann den MessageManager samt Schlange (`juce_MessageManager.cpp:465-477`, `:47-54`; `juce_Messaging_windows.cpp:303-305`): der Klick wird nie zugestellt. Im typischen Konfliktfall liegt aber eine zweite Instanz desselben Moduls im selben FL-Prozess (Kommentar `PluginEditor.cpp:535-537`: „typisch: Plugin in FL dupliziert") und hält JUCE am Leben; dort bleibt der Fall „Editor und Prozessor tot, Klick wird zugestellt" offen.

## V-4 — Vergleich Messpunktpanel und Hinweisliste: BESTÄTIGT

**Messpunktpanel seit Etappe 4** (`553b70ae`):

- Mitglied `juce::Component::SafePointer<EqCopilotEditor> editor;` (`PluginEditor.cpp:643-647`); derselbe `SafePointer` geht an Panel und Änderungs-Callback (`:773-782`, der Callback prüft ihn, `:777-781`).
- Der Konstruktor greift nur beim Öffnen über `editor->processor` zu (`:658`, Editor lebt).
- `uebernehmen()` prüft zuerst und kehrt ohne Zugriff zurück (`auto* ed = editor.getComponent(); if (ed == nullptr) return;`, `:728-732`); den Prozessor erreicht es nur als `ed->processor` (`:742`). Der Destruktor ruft `uebernehmen()` (`:710`).
- Gezählte Marke nur im Testbau: `testzugang::messpunktMarkeFuerTest()` (`:611-625`), gefragt nach der Lebendprüfung und vor dem Prozessorzugriff (`:738-741`). In B15 der Kanarienvogel `Vogel { lebt, marke }` (`eq-copilot/plugin/tests/ShotTestMain.cpp:340-350`), `vogel->lebt = false` direkt nach `editor.reset()` (`:388-389`), verlangt `m35.marke == 0` (`:427-428`).
- Keine Schleuse: die Besitzschleuse gehört den IPC- und Provider-Rückrufen (Gen `eq-copilot/plugin/src/PluginProcessor.cpp:500`, Sonde `docs/beweise/NAK-312.md:1434-1467`), kein Panel nutzt sie.

**Hinweisliste:** `const auto safe = juce::Component::SafePointer<EqCopilotEditor> (this);` (`PluginEditor.cpp:505-507`); beide Lambdas prüfen `safe.getComponent()` vor jedem Zugriff (`:510-519`). Die Hörknöpfe fangen das `this` der Liste (`:454-459`) und erreichen den Editor nur über diese zwei Lambdas. `BefundListe` hat keinen Destruktor (`:345-493`). Keine Schleuse, keine gezählte Marke, kein Test (`eq-copilot/plugin/tests/`: kein Treffer für `zeigeHinweise`, `BefundListe`, `keine Hinweise`).

**Konfliktpanel:** rohes `this` (`PluginEditor.cpp:589`), kein Destruktor (`PluginEditor.cpp:542-578`), keine Marke, kein Test (`eq-copilot/plugin/tests/`: kein Treffer für `Kennung doppelt`, `konfliktKnopf`, `zeigeKonflikt`, `KonfliktPanel`, `triggerClick`).

**Suche.**

- `PluginEditor.cpp`: `launchAsynchronously` an genau vier Stellen, `:523` und `:531` (Hinweisliste, direkt und im Viewport), `:597` (Konflikt), `:774` (Messpunkt). Kein `enterModalState`, `showAsync`, `AlertWindow`, `MessageManager::callAsync`, `callAfterDelay`, `showMenuAsync`, `launchAsync`, `runModalLoop`, `DialogWindow` oder `NativeMessageBox`. Einziger Timer ist `startTimerHz (30)` (`:230`) mit der Mitgliedsfunktion `timerCallback()` (`:242`), kein Lambda; er endet mit dem Editor.
- Eingebundene Quellen: `PluginEditor.cpp:1-3` (`PluginEditor.h`, `LeitstandTokens.h`, `EqCopilotIds.h`), `PluginEditor.h:20-23` (`PluginProcessor.h`, `Diagnose.h`, `EqCopilotAssetKit.h`). Keine enthält eine Komponente mit Rückruf, einen Timer oder einen modalen Weg (Suche nach `std::function`, `onClick`, Komponentenbasis, `Timer`: nur die eigene Basis `private juce::Timer`, `PluginEditor.h:51`). Die drei Panels sind lokale Strukturen in `PluginEditor.cpp` (`PluginEditor.cpp:345-493`, `:542-578`, `:641-764`). Über ganz `eq-copilot/plugin/src/` trifft die Suche nach den modalen und asynchronen Aufrufen nur `PluginEditor.cpp`.
- Die übrigen `[this]`-Lambdas des Editors (`PluginEditor.cpp:121-197`, `:209-210`) gehören Mitgliedskomponenten. Ihre geposteten Wege tragen eine schwache Referenz auf das Mitglied (`juce_Component.cpp:2096-2103`; `TextEditor` postet Enter und Fokusverlust, `modules/juce_gui_basics/widgets/juce_TextEditor.cpp:622`, `:1978-1991`, Zustellung `:2008-2043`) und sterben mit dem Editor; ebenso der ComboBox-Listener (`PluginEditor.cpp:99`; der eigene `AsyncUpdater` der ComboBox verwirft beim Abbau, `juce_AsyncUpdater.cpp:61-72`). Keiner ist ein CallOutBox- oder Dialogweg, keiner läuft nach dem Ende des Editors.

Bestätigt: die Hinweisliste erreicht den Editor nur über `SafePointer`; das Kennungskonflikt-Panel ist der einzige verbliebene CallOutBox- oder Dialogweg des Editors mit rohem `this`.

## V-5 — Zusagenlage: PRÄZISIERT

**Die Sätze wörtlich.**

- **R-312-2** (`docs/beweise/NAK-312.md:130-133`, wortgleich in §5.6 `:437-441`): „**R-312-2 (T3-04-03).** Ein asynchroner Rückruf des Editors (modales Panel, verzögerter Aufruf) fasst Prozessor und Editor nur nach einer Lebendprüfung an; nach dem Ende seines Eigentümers ist er wirkungslos: kein Zugriff, keine Mutation, keine Dirty-Meldung. Der Handgriff bleibt derselbe." Satzzählung wie in der Etappe-4-Validierung (dort V-3): Satz 1 bis zum Semikolon, Satz 2 bis „keine Dirty-Meldung.", Satz 3 „Der Handgriff bleibt derselbe."
- **Gate S25k** (`docs/plan/plan.json:361`, wortgleich im Kopf `docs/beweise/NAK-312.md:6`), die einschlägigen Stellen: „T3-04-01 und T3-04-03 (Provider nach 2-s-Detach, Messpunktpanel überlebt Prozessor)"; „GRENZE: Audio-Thread ohne Sperren und Allokationen; State bleibt verlustfrei, fremde Bytes bytegleich; Designteil bleibt geparkt (keine neue Oberfläche, nur nötiger Handgriff); keine Legacy-Umbenennung."; „BEWEIS: docs/beweise/NAK-312.md mit Matrix, Rotbeweisen an B2, B6, B7, B14, B15, RT-Wache vor Parameterverarbeitung, vollem Kanon GRÜN und Laufzeit-Arm."
- **M-35** (`docs/beweise/NAK-312.md:818`, Hervorhebungen weggelassen). Befund: „T3-04-03 · R-312-2". Zustand und Vorbedingung: „Gen in Legacy-Rolle, Editor offen, Messpunkt-Popover steht (`eq-copilot/plugin/src/PluginEditor.cpp:734`), im Panel eine echte Änderung gesetzt". Ereignis: „Editor zerstören, danach Prozessor zerstören, DANN der Nachrichtenschleife einen Durchlauf geben". Zusage: „Der Panel-Destruktor fasst weder Prozessor noch Editor an: die Marke zählt 0 Zugriffe nach dem Ende, keine Mutation an der Bindung, 0 Host-Dirty-Meldungen". Reihenfolge und Frist: „Lebendprüfung VOR dem Prozessorzugriff in `uebernehmen()` (`PluginEditor.cpp:700-709`, Zugriff heute bei `:707`)". Test: „B15 NEU `312/M-35 panel_abbau_nach_prozessorende_ist_wirkungslos`, alternativ ein Fall der B14-Familie". Quelle: „R-312-2; Teil B §2".
- **M-36** (`docs/beweise/NAK-312.md:819`). Befund: „T3-04-03 · öffnen↔schließen". Zustand: „wie M-35". Ereignis: „Panel schließen, DANN Editor, DANN Prozessor zerstören". Zusage: „Der normale Handgriff bleibt unverändert: die Bindung wird übernommen, und genau EINE Host-Dirty-Meldung entsteht, weil genau eine persistente Änderung vorliegt". Reihenfolge und Frist: „`uebernehmen()` läuft im Destruktor des Panels, solange der Editor lebt". Quelle: „R-312-2 (letzter Satz)".
- **M-37** (`docs/beweise/NAK-312.md:820`). Befund: „T3-04-03 · Gegenfall ohne Änderung". Zustand: „wie M-36, aber im Panel nichts geändert". Ereignis: „Panel schließen, Editor und Prozessor zerstören". Zusage: „Keine Mutation und keine Dirty-Meldung: der Frühausstieg wegen fehlender Änderung bleibt, wo er ist". Reihenfolge und Frist: „`State.cpp:318-319` liegt INNERHALB der Sperre und bleibt dort". Quelle: „`CLAUDE.md:201-203`; R-312-2".
- **§5.6** (`docs/beweise/NAK-312.md:427-431`): „Keine Zusage gebrochen. Teil B hat Entwurf, Blueprint und `docs/plugin-wissen.md` nach einer Lebensdauerregel für ein ausgelagertes modales Panel durchsucht und keine gefunden; R-D2 regelt ausdrücklich Client-Callbacks. Die Regel fehlt, nicht ihre Erfüllung — deshalb LÜCKE mit technischer Regel."
- **R-312-12** (`docs/beweise/NAK-312.md:2129-2133`): „**R-312-12 (L2).** Ein Rotbeweis läuft nie in undefiniertem Verhalten: der Haken zählt den Zugriff an der Marke, bevor der Rückruf Prozessorzustand berührt, und hält den Rückruf danach dauerhaft an. Das gilt für den Lauf gegen den Basisstand und für jede Mutation von 312/M-30, 312/M-31 und 312/M-39; der Beleg „heute rot" ist die gezählte Marke, nicht ein Absturz."
- **CLAUDE.md**: „Keine toten UI-Elemente (User 24.08.2026): jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand." (`CLAUDE.md:51-52`); „Beziehungen mitprüfen: speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg gehören in denselben Änderungssatz." (`CLAUDE.md:84-86`); Designteil geparkt (`CLAUDE.md:35`).

**Deckung.**

- Allgemein: Satz 1 ist für „modales Panel, verzögerter Aufruf" gefasst. Der Konfliktrückruf ist ein Rückruf des Editors aus einem modalen Panel (die Box geht in `enterModalState`, V-1) und läuft über Enter oder Barrierefreiheit als gepostete, also verzögerte Nachricht. Der Wortlaut deckt ihn.
- Satz 2 deckt die Folge: nach dem Ende des Editors würde der Rückruf zugreifen, die `instanceId` ändern und Dirty melden (V-3) — genau die drei verbotenen Wirkungen, wenn „Eigentümer" der Editor ist. So liest es B15 (`vogel->lebt = false` nach `editor.reset()`, „Ende des Eigentuemers", `ShotTestMain.cpp:388-389`); C++-Eigentümer des Panels ist der `CallOutBoxCallback` (`juce_CallOutBox.cpp:83`). Die Mehrdeutigkeit hat die Etappe-4-Validierung (V-3) festgehalten.
- Satz 3 verlangt, dass der Klick bei lebendem Editor derselbe bleibt (neue Kennung, Status). Eine Lebendprüfung erfüllt das und erzeugt kein totes Element: nach dem Ende des Editors hat das Panel kein Fenster mehr (V-2), also nichts Sichtbares ohne Wirkung. Keine neue Oberfläche nötig (Gate GRENZE).
- Im Besonderen: die Regel trägt den Schlüssel „(T3-04-03)", das Gate nennt nur „Messpunktpanel überlebt Prozessor", M-35 bis M-37 nennen nur das Messpunkt-Popover; §5.6 sucht allgemein nach einer Regel für „ein ausgelagertes modales Panel", zitiert aber nur den Kommentar am Übergabepunkt des Messpunkt-Popovers (`docs/beweise/NAK-312.md:432-436`), und §7.4 nennt nur `MesspunktPanel` (`docs/beweise/NAK-312.md:1469-1478`); die Matrix ordnet öffnen↔schließen nur M-35 bis M-37, M-46, M-59, M-62 zu (`docs/beweise/NAK-312.md:667-671`). Keine Gate-Zeile, keine Matrixzeile und kein Bein führt das Konfliktpanel. Gedeckt im Wortlaut der Regel, ungeregelt auf der Ebene von Gate, Zusage und Messung. Die Registerzeile („R-312-2 Satz 1 ist allgemein für modale Panels gefasst, das Gate von S25k nennt nur das Messpunktpanel") hält.

**Bein.** B15 `EqCopShot --sonde012-suite` (`tools/beweise.ps1:752`), dasselbe Bein und dasselbe Gerüst wie M-35 bis M-37: echter Editor auf einem Heap-Prozessor (`ShotTestMain.cpp:352-357`), Knopf unter den Kindern des Editors gesucht und `onClick` gerufen (`:359-365`), Box und Panel als Kinder gefunden (`:366-369`), die Nachrichtenschleife `pumpe` (`:281-302`), Dirty-Zähler `PanelDirty` (`:304-312`). Für den Konfliktfall:

- Der Konfliktknopf ist über seinen Text „Kennung doppelt!" auffindbar (`PluginEditor.cpp:166`), sein `onClick` auch unsichtbar aufrufbar (unsichtbar gesetzt `:169`, sichtbar nur bei Brokermeldung `:266`).
- `neuKnopf` ist das `juce::TextButton`-Kind des Panels. `Button::triggerClick()` ist öffentlich (`modules/juce_gui_basics/buttons/juce_Button.h:236`) und postet genau die Nachricht, die Enter postet (`juce_Button.cpp:359-362`, `:665-674`).
- Die Folge `triggerClick()` → `editor.reset()` → `vogel->lebt = false` → `pumpe` macht die Reihenfolge ohne Host deterministisch (Schlange wie V-2), in zwei Varianten: Prozessor lebt (Fenster zu) und Prozessor danach zerstört (wie M-35).
- Im Testbau öffnet sich keine Produktions-Pipe: `v2PipeNameDesBaus()` liefert einen Probe-Namen ohne Server (`PluginProcessor.cpp:93-100`), `controlV3` und `telemetryV3` starten nicht (`:464-471`). Eine Regressionswache „Klick bei lebendem Editor vergibt eine neue Kennung mit genau einer Host-Dirty-Meldung" läuft damit ebenfalls in B15.
- B14 (`EqCopSonde012ProjectReloadTest`, `tools/beweise.ps1:751`) baut keinen Editor: `createEditor` kommt in `eq-copilot/plugin/tests/` nur in `ShotTestMain.cpp:94`, `:356`, `:520`, `PaintBenchMain.cpp:55` und `HostProbeTestMain.cpp:661` vor.

**Fehlender Testzugang.** Eine Marke im Rückruf `tun` von `zeigeKonflikt()` unter `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3`, nach dem Muster `testzugang::messpunktMarkeFuerTest()` (`PluginEditor.cpp:611-625`). Heute steht in `:535-599` kein solcher Block.

**Präzisierung (Berichtigung).** R-312-12 setzt die Marke „bevor der Rückruf Prozessorzustand berührt" und nennt nur 312/M-30, 312/M-31 und 312/M-39. Für M-35 genügte das: im Basisstand hielt das Panel den Prozessorverweis selbst (`EqCopilotProcessor& proz;`, `git show a80f0603:eq-copilot/plugin/src/PluginEditor.cpp`, `:627`; Zugriff `:707`), das Lesen vor der Marke traf lebenden Panelspeicher. Im Konfliktrückruf liegt der Prozessorverweis **im toten Editor**: der erste Zugriff auf freigegebenen Speicher ist das Lesen von `this->processor` (`PluginEditor.h:117`), noch vor jedem Prozessorzustand. Eine nach R-312-12 gesetzte Marke „vor dem Prozessorzugriff" ist deshalb nur dann frei von undefiniertem Verhalten, wenn sie vor jeder Dereferenzierung von `this` steht — als erste Anweisung des Lambdas, ohne Editorzustand zu benutzen; nach dem Bau hinter der Lebendprüfung wie `PluginEditor.cpp:730-741`. Sonst liefe schon der Basislauf der neuen Zeile in undefiniertem Verhalten, gegen das Ziel von R-312-12. Der Kanarienvogel aus §6.4 („den Prozessor überlebt und dessen Destruktor es auf false setzt", `docs/beweise/NAK-312.md:789-793`) passt auf das Ende des Editors nur in der Lesart von B15 (`lebt = false` nach `editor.reset()`), wie bei M-35.

## V-6 — Berührung mit Etappe 6: BESTÄTIGT

- Änderungssätze der Etappe 6 (§7.6, `docs/beweise/NAK-312.md:1645-1815`; Pfade §7.0, `:955-971`): Rücknahme vor dem Store in `spiegleKlassifikation` (`State.cpp`), `acquire`-Lesen in `PluginProcessor.cpp`, `wechsleFlaecheWennNoetig` räumt nur Anzeigezustand, die drei Netze in `timerCallback` vor die Abzweigung `:245` oder `else` statt `return` bei `:257`, Klassifikationsabfrage in `schalteMarkierung`, `reset` und `releaseResources` (`PluginProcessor.h`, `Hostbruecke.cpp`, `HoerMarkierung.h`), Entwurfskennung des Labels (`uebernehmeSourcesLabel`, `aktualisiereSourcesSteuerung`), Ersatzziel (`SourcesModel.cpp`), Tick-Testzugang in `PluginEditor.h` (`docs/beweise/NAK-312.md:1706-1712`); Beine A3, B15, B14, B13 (`docs/beweise/NAK-312.md:1805-1806`).
- Die für `PluginEditor.cpp` genannten Funktionen (`docs/beweise/NAK-312.md:959-961`): „`timerCallback`, `schalteMarkierung`, `wechsleFlaecheWennNoetig`, `uebernehmeSourcesLabel`, `aktualisiereSourcesSteuerung`, Kommentare". Nicht darunter: `zeigeKonflikt()` (`PluginEditor.cpp:538-599`), `KonfliktPanel` (`:542-578`), der Konfliktrückruf (`:589-595`).
- Berührungen in derselben Datei ohne Wirkung auf die Lebensdauer des Panels: `timerCallback` schaltet `konfliktKnopf` im Legacy-Zweig sichtbar (`PluginEditor.cpp:266`), direkt zwischen dem `return` (`PluginEditor.cpp:257`) und dem Netzblock (`PluginEditor.cpp:268-278`), den R-312-8 versetzt; `wechsleFlaecheWennNoetig` blendet `konfliktKnopf` in der Main-Fläche aus (`PluginEditor.cpp:880-886`). Beides betrifft nur die Sichtbarkeit des Knopfs, nicht Box, Panel oder Rückruf. E-312-8 („kein Handgriff wirkt, dessen Bedienelement nicht sichtbar ist", `docs/beweise/NAK-312.md:2095-2097`) trifft das offene Panel nicht: der Flächenwechsel blendet nur die Legacy-Liste aus (`PluginEditor.cpp:880-886`), die Box gehört nicht dazu. `statusMeldung` und `statusMeldungBisMs` schreiben auch `schalteMarkierung` und `markierungBeenden` (`PluginEditor.cpp:835-836`, `:847-853`, `:863-864`); geteilt wird nur das Mitglied, kein Änderungssatz.
- Ergebnis: kein Änderungssatz der Etappe 6 berührt `zeigeKonflikt()` oder das Konfliktpanel. Aus der Beziehungsregel (`CLAUDE.md:84-86`) und der Regel zu irreführenden Kommentaren folgt keine Pflicht, die Sicherung in einen dieser Änderungssätze zu legen. Sie teilt mit Etappe 6 die Ticketpfade (`PluginEditor.cpp`, `ShotTestMain.cpp`; §7.0, `docs/beweise/NAK-312.md:959-971`) und das Bein B15 und kann dort als eigener Änderungssatz stehen. Den Tick-Testzugang braucht eine solche Zeile nicht: der Klick wird direkt ausgelöst und die Schleife gepumpt.

## Nicht geprüft

- Nichts gebaut, gefahren oder gemessen; kein Test, kein Compiler, kein FL.
- FLs Verhalten: ob und wann FL die Ansicht abbaut (Fensterschluss, Entfernen der Instanz, Projektende); die Folge von `removed()`, `terminate()` und Freigabe; ob FL gefiltert pumpt oder Enter selbst behandelt; ob das Pluginfenster den Systemtastaturfokus bekommt; ob FL in `setDirty` oder `restartComponent` die Ansicht abbaut. Nicht gemessen; im Repo nicht erneut gesucht (die Etappe-4-Validierung fand zum Abbau nichts, `docs/beweise/roh/NAK-312-etappe-4-erstpruefung-validierung.md:18`).
- Welche Folge der Zugriff auf freigegebenen Speicher im Prozess hat (stiller Durchlauf, Heapschaden, Absturz).
- Ob FL ein dupliziertes Plugin in einem eigenen Bridge-Prozess hostet (betrifft nur den Fall der letzten Instanz, V-3).
- Verschachtelte Nachrichtenschleifen des Hosts in `removed()`; der Editor selbst nutzt kein `runModalLoop` (V-4).
- Der UI-Automation-Weg von Windows jenseits von `ButtonAccessibilityHandler`; die macOS- und Linux-Zweige des Wrappers.
- Abweichung vom Auftrag: ein Lesebefehl enthielt versehentlich `python3 --version` (Ausgabe `Python 3.13.14`). Der Auftrag schließt das Starten eines Python-Interpreters aus; der Aufruf las und schrieb keine Datei und trägt zu keinem Urteil bei.

## HEAD

HEAD zu Beginn: `9f6c0f074a8fa05365ecf6dab91807e366a95d7e` (erste Messung der Session, vor jeder Lektüre). HEAD am Ende: `9f6c0f074a8fa05365ecf6dab91807e366a95d7e` (20:39:32 vor dem Schreiben, 20:44:33 nach dem Schreiben und 20:46:46 nach der letzten Korrektur dieses Berichts, `date`), gleich. Alle mit `git rev-parse HEAD`; `git status --short` zeigte zu Beginn nur `briefing-hub/` und `nimbalyst-local/`, nach dem Schreiben zusätzlich allein diesen Bericht (untracked).

FERTIG Quellvalidierung NAK-349, 3 bestätigt, 3 präzisiert, 0 widerlegt
