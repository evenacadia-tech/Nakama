export type FriendlyOption = {
  value: string;
  label: string;
  text: string;
};

export type FriendlyQuestion = {
  title: string;
  prompt: string;
  options: FriendlyOption[];
};

export type DecisionPackage = {
  id: string;
  title: string;
  summary: string;
  timing: string;
  image?: string;
  imageAlt?: string;
  questionIds: string[];
};

const option = (value: string, label: string, text: string): FriendlyOption => ({
  value,
  label,
  text,
});

export const decisionPackages: DecisionPackage[] = [
  {
    id: 'U2',
    title: 'Gen als Gesamtbild prüfen',
    summary: 'Erst die Richtung bestätigen, danach die sichtbaren Details einzeln klären.',
    timing: 'Jetzt',
    image: '/images/nakama-gen.png',
    imageAlt: 'Aktueller dunkler Entwurf von Nakama Gen',
    questionIds: [
      'U2.0', 'U2.1', 'U2.2', 'U2.3', 'U2.4', 'U2.5', 'U2.6', 'U2.7',
      'U2.8', 'U2.9', 'U2.10', 'U2.11', 'U2.12', 'U2.13', 'U2.14', 'U2.15',
      'U2.17',
    ],
  },
  {
    id: 'U5',
    title: 'Sidechain-Auswahl in Probeeq',
    summary: 'Festlegen, ob und wo die zusätzliche Quelle pro Band sichtbar wird.',
    timing: 'Wenn du Zeit hast',
    image: '/images/u5-probeeq-bandrow.png',
    imageAlt: 'Bandzeile von Nakama Probeeq mit zwölf sichtbaren Werten',
    questionIds: ['U5'],
  },
  {
    id: 'U6',
    title: 'Suna im Detail bestätigen',
    summary: 'Zehn kleine Fragen zur bereits festgelegten Suna-Kachel.',
    timing: 'Wenn du Zeit hast',
    image: '/images/u6-suna-tile.png',
    imageAlt: 'Aktueller dunkler Entwurf der Nakama Suna Kachel',
    questionIds: [
      'U6.1', 'U6.2', 'U6.3', 'U6.4', 'U6.5',
      'U6.6', 'U6.7', 'U6.8', 'U6.9', 'U6.10',
    ],
  },
  {
    id: 'U9',
    title: 'Figma und Technik angleichen',
    summary: 'Fünf Unterschiede klären, bevor die Quellenübersicht gebaut wird.',
    timing: 'Später',
    image: '/images/u9-gen-grid.png',
    imageAlt: 'Quellen und Frequenzbereiche im aktuellen Gen Entwurf',
    questionIds: ['U9.1', 'U9.2', 'U9.3', 'U9.4', 'U9.5'],
  },
  {
    id: 'U10',
    title: 'Hör-Markierung ohne Wiedergabe-Signal',
    summary: 'Entscheiden, wie Gen reagiert, wenn FL Studio kein eindeutiges Wiedergabe-Signal liefert.',
    timing: 'Später',
    questionIds: ['U10'],
  },
];

export const friendlyQuestions: Record<string, FriendlyQuestion> = {
  'U2.0': {
    title: 'Passt Gen als Ganzes?',
    prompt: 'Beurteile zuerst nur Material, Anordnung, Dichte und Farbwelt. Kleine Details kommen danach.',
    options: [
      option('Vorschlag übernehmen', 'Ja, diese Richtung stimmt', 'Die Übersetzung des Entwurfs kann beginnen.'),
      option('Alternative wählen', 'Grundsätzlich anders', 'Die Richtung wird vor der Übersetzung noch einmal geändert.'),
    ],
  },
  'U2.1': {
    title: 'Sollen leere Formen überall dasselbe bedeuten?',
    prompt: 'In Gen bedeutet eine leere Zelle heute „Messung alt“. In Probeeq und Suna bedeutet ein leerer Punkt „nicht verbunden“.',
    options: [
      option('Vorschlag übernehmen', 'Beide Bedeutungen bleiben', 'Die Form wird je Oberfläche aus ihrem Zusammenhang verstanden.'),
      option('Alternative wählen', 'Eine Bedeutung für alle Apps', 'Eine der beiden Anzeigen bekommt ein neues Zeichen.'),
    ],
  },
  'U2.2': {
    title: 'Wo sollen die vier Arbeitsbereiche stehen?',
    prompt: 'Es geht um Observe, Advise, Audition und Apply.',
    options: [
      option('Vorschlag übernehmen', 'Oben links im Hauptfenster', 'Die vier Bereiche bleiben dort sofort sichtbar.'),
      option('Alternative wählen', 'Unten bei Sources und Undo', 'Oben entsteht Platz für eine eigene Kopfzeile.'),
    ],
  },
  'U2.3': {
    title: 'Wie zeigt Gen die betroffene Stelle?',
    prompt: 'Der Fokus soll klar sein, ohne die Quellenliste dauerhaft laut zu machen.',
    options: [
      option('Vorschlag übernehmen', 'Ring im Raster', 'Nur beim Vorhören wird zusätzlich die ganze Zeile markiert.'),
      option('Alternative wählen', 'Zeile dauerhaft markieren', 'Das Vorhören braucht dann ein anderes sichtbares Zeichen.'),
    ],
  },
  'U2.4': {
    title: 'Soll ein veralteter Quellenname oben wiederholt werden?',
    prompt: 'Der Name steht bereits in seiner Zeile. Im Kartenkopf könnte er zusätzlich erscheinen.',
    options: [
      option('Vorschlag übernehmen', 'Zahl und Name wiederholen', 'Oben steht zum Beispiel „1 stale, Synth Pad“.'),
      option('Alternative wählen', 'Nur die Zahl zeigen', 'Die Quellenzeile reicht für den Namen.'),
    ],
  },
  'U2.5': {
    title: 'Welche Wörter sollen die drei Aktionen tragen?',
    prompt: 'Die Familie soll überall denselben Wortschatz verwenden.',
    options: [
      option('Vorschlag übernehmen', 'Hold to audition, Apply, Reject', 'Gen übernimmt die Wörter aus Probeeq.'),
      option('Alternative wählen', 'Audition, Apply, Discard', 'Gen behält die ältere Formulierung.'),
    ],
  },
  'U2.6': {
    title: 'Wo stehen Sicherheit und mögliche andere Ursache?',
    prompt: 'Beide Angaben helfen beim Einordnen eines Befunds, sollen aber nicht vom Wesentlichen ablenken.',
    options: [
      option('Vorschlag übernehmen', 'Direkt im Befund', 'Sicherheit steht oben, die andere Ursache am Ende der Erklärung.'),
      option('Alternative wählen', 'Einen Schritt tiefer', 'Mindestens eine Angabe erscheint erst beim Öffnen weiterer Details.'),
    ],
  },
  'U2.7': {
    title: 'Soll Sources die gemeinsame Tür zu allen Quellen sein?',
    prompt: 'Dahinter liegen Quellenliste, Rollen, Suche und Gruppen.',
    options: [
      option('Vorschlag übernehmen', 'Eine Taste namens Sources', 'Alle Einstellungen bleiben an einem klaren Ort.'),
      option('Alternative wählen', 'Anders benennen oder aufteilen', 'Einzelne Quellenfunktionen kommen direkt auf die Hauptfläche.'),
    ],
  },
  'U2.8': {
    title: 'Wie viel Rückweg soll Undo zeigen?',
    prompt: 'Der aktuelle Entwurf zeigt nur die Anzahl bereits angewandter Änderungen.',
    options: [
      option('Vorschlag übernehmen', 'Undo mit Anzahl', 'Ein Klick nimmt die letzte Änderung zurück. Es gibt keine eigene Verlaufsliste.'),
      option('Alternative wählen', 'Vollständigen Verlauf zeigen', 'Angewandte Änderungen lassen sich einzeln ansehen und zurücknehmen.'),
    ],
  },
  'U2.9': {
    title: 'Sollen Rollen direkt neben dem Quellennamen stehen?',
    prompt: 'Beispiele sind Leads, Carries, Supports, Protected und Merged.',
    options: [
      option('Vorschlag übernehmen', 'Rolle direkt in jeder Zeile', 'Frische Quellen brauchen kein zusätzliches „Live“.'),
      option('Alternative wählen', 'Rollen nur unter Sources', 'Die Übersicht bleibt ruhiger.'),
    ],
  },
  'U2.10': {
    title: 'Passt die weinrote Helligkeit der Zellen?',
    prompt: 'Dunklere und hellere Flächen zeigen die Energie in einem Frequenzbereich.',
    options: [
      option('Vorschlag übernehmen', 'So lassen', 'Die aktuelle Abstufung wird übernommen.'),
      option('Alternative wählen', 'Stärker oder zurückhaltender', 'Farbe oder Helligkeitsstufen werden angepasst.'),
    ],
  },
  'U2.11': {
    title: 'Sollen verschiedene Zustände an verschiedenen Orten stehen?',
    prompt: 'Gen zeigt allgemeine Warnungen oben, Messfortschritt unten und Probleme einzelner Quellen direkt an der Quelle.',
    options: [
      option('Vorschlag übernehmen', 'Je Zustand der passende Ort', 'Die Hauptfläche bleibt nach Bedeutung geordnet.'),
      option('Alternative wählen', 'Alles in einer Kopfzeile', 'Alle Statusangaben stehen zusammen.'),
    ],
  },
  'U2.12': {
    title: 'Braucht der Befund eine zusätzliche Fokuszeile?',
    prompt: 'Der betroffene Bereich ist bereits im Raster markiert und im Text benannt.',
    options: [
      option('Vorschlag übernehmen', 'Keine zusätzliche Fokuszeile', 'Die vorhandenen Hinweise reichen.'),
      option('Alternative wählen', 'Fokuszeile behalten', 'Quelle und Frequenzbereich werden oben noch einmal genannt.'),
    ],
  },
  'U2.13': {
    title: 'Wie wechselt man zwischen Befunden und Details?',
    prompt: 'Es geht um mehrere Befunde und um die genauere Ansicht einer einzelnen Quelle.',
    options: [
      option('Vorschlag übernehmen', 'Befunde über Observe, Details beim Urteilen', 'Jeder Bereich hat eine klare Aufgabe.'),
      option('Alternative wählen', 'Pfeile und eigene Detailansicht', 'Befunde lassen sich direkt im Advisor durchblättern.'),
    ],
  },
  'U2.14': {
    title: 'Sollen bei 16 Quellen noch alle Namen sichtbar sein?',
    prompt: 'Mit 16 Quellen wird die Schrift klein. Im üblichen Projekt mit wenigen Quellen ist das kein Problem.',
    options: [
      option('Vorschlag übernehmen', 'Namen auch bei 16 Quellen zeigen', 'Die Übersicht bleibt vollständig, aber sehr dicht.'),
      option('Alternative wählen', 'Bei 16 Quellen Namen ausblenden', 'Die Namen erscheinen erst beim Auswählen.'),
    ],
  },
  'U2.15': {
    title: 'Soll die Gehäuseschale einen Lichtstreifen bekommen?',
    prompt: 'Ein älterer Probeeq-Stand hatte diesen Effekt. Der aktuelle Gen-Entwurf nicht.',
    options: [
      option('Vorschlag übernehmen', 'Ohne Lichtstreifen', 'Die Schale bleibt ruhig und dunkel.'),
      option('Alternative wählen', 'Lichtstreifen auf beiden Apps', 'Gen und Probeeq erhalten denselben sichtbaren Effekt.'),
    ],
  },
  'U2.17': {
    title: 'Welche Größen sollen vorbereitet werden?',
    prompt: 'Claude holt die fertigen Teile direkt aus Figma. Du musst nichts exportieren.',
    options: [
      option('100, 125, 150 und 200 Prozent', 'Vier feste Größen', 'Diese Abstufungen decken kleine bis große Anzeigen ab.'),
      option('Andere Größen', 'Andere Abstufungen', 'Schreibe die gewünschten Werte in das Textfeld.'),
    ],
  },
  U5: {
    title: 'Wo soll die Sidechain-Quelle pro Band erscheinen?',
    prompt: 'Technisch ist sie bereits gespeichert. Offen ist nur, wie viel davon auf der Oberfläche sichtbar wird.',
    options: [
      option('Sichtbar in der Bandzeile', 'Direkt in der Bandzeile', 'Alle 13 Werte sind auf einen Blick sichtbar.'),
      option('Untermenü unter DYN', 'Unter DYN aufklappen', 'Die normale Zeile bleibt bei zwölf Werten.'),
      option('Im ersten Release unsichtbar', 'Zunächst nicht zeigen', 'Der Wert ist vorbereitet, aber noch nicht bedienbar.'),
    ],
  },
  'U6.1': {
    title: 'Sollen die Verbindungspunkte wie bei Probeeq funktionieren?',
    prompt: 'Gefüllt mit Saum bedeutet verbunden und aktiv. Hohl bedeutet kein Kontakt.',
    options: [
      option('Vorschlag übernehmen', 'Ja, dieselbe Logik', 'Suna und Probeeq sprechen dieselbe sichtbare Sprache.'),
      option('Alternative wählen', 'Ein anderes System', 'Die Verbindungspunkte werden neu festgelegt.'),
    ],
  },
  'U6.2': {
    title: 'Passen die drei Positionen Pre, Insert und Post?',
    prompt: 'Damit zeigt Suna, wo sie in der Signalkette misst.',
    options: [
      option('Vorschlag übernehmen', 'Pre, Insert und Post', 'Die Kachel bleibt kurz und eindeutig.'),
      option('Alternative wählen', 'Mixerspur zusätzlich nennen', 'Die Kachel zeigt auch den Namen oder die Nummer der FL-Spur.'),
    ],
  },
  'U6.3': {
    title: 'Passen die fünf Rollenwörter?',
    prompt: 'Leads, Carries, Supports, Protected und Merged beschreiben die Aufgabe einer Quelle im Mix.',
    options: [
      option('Vorschlag übernehmen', 'Diese fünf Rollen verwenden', 'Die Wörter gelten für die ganze Familie.'),
      option('Alternative wählen', 'Andere Wörter', 'Schreibe deine gewünschten Rollen in das Textfeld.'),
    ],
  },
  'U6.4': {
    title: 'Wie soll eine veraltete Messung beschrieben werden?',
    prompt: 'Der Entwurf zeigt „Stale“ und direkt dahinter das Alter in Sekunden.',
    options: [
      option('Vorschlag übernehmen', 'Wort plus genaue Zeit', 'Zum Beispiel „Stale, 40 s“.'),
      option('Alternative wählen', 'Nur grob oder ohne Zeit', 'Zum Beispiel „seit einigen Minuten“ oder nur „Stale“.'),
    ],
  },
  'U6.5': {
    title: 'Passen die beiden Warntexte?',
    prompt: 'Es geht um fehlenden Kontakt zu Gen und um eine doppelte Instanz.',
    options: [
      option('Vorschlag übernehmen', 'Not delivering to Gen, Duplicate instance', 'Die Texte bleiben so kurz wie möglich.'),
      option('Alternative wählen', 'Andere Formulierung', 'Schreibe den gewünschten englischen Wortlaut in das Textfeld.'),
    ],
  },
  'U6.6': {
    title: 'Soll „Passive Probe“ auf jeder Kachel stehen?',
    prompt: 'Bei vielen Suna-Kacheln wiederholt sich diese Kopfzeile oft.',
    options: [
      option('Vorschlag übernehmen', 'Kopfzeile behalten', 'Suna bleibt klar als passive Sonde erkennbar.'),
      option('Alternative wählen', 'Kopfzeile entfernen', 'Der gewonnene Platz wird für Status und Warnungen genutzt.'),
    ],
  },
  'U6.7': {
    title: 'Sollen Bus, Rolle und Position feste Spalten haben?',
    prompt: 'Feste Spalten sorgen dafür, dass viele Kacheln sauber untereinander stehen.',
    options: [
      option('Vorschlag übernehmen', 'Feste Spalten', 'Kurze Namen lassen etwas Luft, alle Werte bleiben ausgerichtet.'),
      option('Alternative wählen', 'Breite nach Inhalt', 'Jede Kachel nutzt den Platz anders.'),
    ],
  },
  'U6.8': {
    title: 'Passt der kleinere Schatten von Suna?',
    prompt: 'Der große Probeeq-Schatten wäre fast so hoch wie die gesamte Suna-Kachel.',
    options: [
      option('Vorschlag übernehmen', 'Kleineren Schatten verwenden', 'Suna wirkt verwandt, aber nicht überladen.'),
      option('Alternative wählen', 'Probeeq-Schatten übernehmen', 'Alle Gehäuse bekommen exakt denselben Schatten.'),
    ],
  },
  'U6.9': {
    title: 'Soll die Nakama-Wortmarke den leichten Glanz behalten?',
    prompt: 'Der neueste Probeeq-Stand nutzt eine sanfte Lichtkante in der Wortmarke.',
    options: [
      option('Vorschlag übernehmen', 'Glänzende Wortmarke', 'Suna übernimmt den neuesten Familienstand.'),
      option('Alternative wählen', 'Flache Wortmarke', 'Die Wortmarke bleibt ohne Lichtkante.'),
    ],
  },
  'U6.10': {
    title: 'Braucht ein unbestätigter Name einen eigenen Zustand?',
    prompt: 'Bisher ist dieser Fall nicht gestaltet, weil sein Aussehen noch nicht beschrieben ist.',
    options: [
      option('Vorschlag übernehmen', 'Keinen eigenen Zustand zeigen', 'Suna zeigt nur beschriebene, belegte Zustände.'),
      option('Alternative wählen', 'Eigenen Zustand gestalten', 'Beschreibe im Textfeld, wie er aussehen soll.'),
    ],
  },
  'U9.1': {
    title: 'Fünf oder sieben Rollen?',
    prompt: 'Figma zeigt fünf Rollen. Der ältere technische Entwurf nennt sieben.',
    options: [
      option('Figma gilt', 'Die fünf Rollen aus Figma', 'Das aktuelle Design bestimmt die Auswahl.'),
      option('Technik gilt', 'Sieben Rollen', 'Die zwei zusätzlichen Rollen kommen zurück.'),
    ],
  },
  'U9.2': {
    title: 'Darf man Bus, Rolle und Position direkt in Suna ändern?',
    prompt: 'Figma zeigt diese Werte in der Kachel. Der technische Entwurf wollte die Bedienung nur in Gen.',
    options: [
      option('Figma gilt', 'Direkt in Suna bedienbar', 'Die Kachel zeigt und ändert ihre eigenen Angaben.'),
      option('Technik gilt', 'Nur in Gen bedienbar', 'Suna bleibt reine Anzeige.'),
    ],
  },
  'U9.3': {
    title: 'Fünf oder zwei Messzustände?',
    prompt: 'Das aktuelle Design unterscheidet genauer als der technische Entwurf.',
    options: [
      option('Figma gilt', 'Fünf Zustände', 'Die Oberfläche zeigt die genaueren Unterschiede.'),
      option('Technik gilt', 'Zwei Zustände', 'Die Anzeige wird einfacher.'),
    ],
  },
  'U9.4': {
    title: 'Drei oder fünf Arbeitsschritte?',
    prompt: 'Figma bündelt den Ablauf stärker als der technische Entwurf.',
    options: [
      option('Figma gilt', 'Drei Schritte', 'Der Ablauf bleibt kompakt.'),
      option('Technik gilt', 'Fünf Schritte', 'Jede Tätigkeit erhält einen eigenen Bereich.'),
    ],
  },
  'U9.5': {
    title: 'Soll Gen das Quellenraster als Hauptübersicht nutzen?',
    prompt: 'Figma zeigt Quellen und Frequenzbereiche in einem Raster. Die ältere Abnahme beschrieb Übersicht und Detail ohne klassische Heatmap.',
    options: [
      option('Figma gilt', 'Raster als Übersicht', 'Das aktuelle Figma-Bild ist die Vorgabe.'),
      option('Technik gilt', 'Übersicht plus Detail ohne Raster', 'Die Hauptansicht wird neu geordnet.'),
    ],
  },
  U10: {
    title: 'Darf die Hör-Markierung starten, wenn FL kein Wiedergabe-Signal liefert?',
    prompt: 'Heute ist sie dann erlaubt. Die strengere Fassung würde sie nur bei eindeutig laufender Wiedergabe freigeben.',
    options: [
      option('Ja, erlaubt lassen', 'Im Zweifel erlauben', 'Die Funktion bleibt verfügbar, wenn FL keine klare Information liefert.'),
      option('Nein, nur mit Signal', 'Im Zweifel sperren', 'Die Funktion startet nur bei eindeutig laufender Wiedergabe.'),
    ],
  },
};

export const taskCopy: Record<string, { title: string; summary: string; timing: string }> = {
  U7: {
    title: 'Alte Plugin-Kopie aus FL entfernen',
    summary: 'Die alte Kopie taucht bei jedem Plugin-Scan doppelt auf. Claude kann den Befehl vorbereiten, du bestätigst den Admin-Schritt.',
    timing: 'Wenn du Zeit hast',
  },
  U8: {
    title: 'Vor der nächsten Installation wissen',
    summary: 'Ein Projekt, das mit dem neuen Stand gespeichert wird, sollte danach nicht mehr mit dem alten Stand geöffnet werden.',
    timing: 'Nur zur Information',
  },
  U11: {
    title: 'Sidechain-Test mit Verzögerung',
    summary: 'Ein kurzer FL-Test prüft später, ob Nebenwege auch mit Latenzausgleich sauber ankommen.',
    timing: 'Wenn du Zeit hast',
  },
  U12: {
    title: 'Altes Design-Repo kann weg',
    summary: 'Die Design-Arbeit liegt jetzt im gemeinsamen Nakama-Repo. Der alte Ordner und das alte GitHub-Repo werden nicht mehr gebraucht.',
    timing: 'Wenn du Zeit hast',
  },
};

export const phaseCopy: Record<string, { title: string; summary: string }> = {
  Vorlauf: {
    title: 'Prüfgrundlage',
    summary: 'Ein gemeinsamer Prüflauf belegt jeden fertigen Schritt.',
  },
  P0: {
    title: 'Bestehendes absichern',
    summary: 'FL Studio wurde vermessen und alte Projekte bleiben erkennbar.',
  },
  P1: {
    title: 'Gemeinsame Regeln',
    summary: 'Die drei Apps und der Broker verstehen dieselben Daten und Zustände.',
  },
  P2: {
    title: 'Messen und speichern',
    summary: 'Audiodaten werden sicher gemessen, verteilt und gespeichert.',
  },
  P3: {
    title: 'Quellenübersicht',
    summary: 'Gen zeigt alle Quellen und sagt ehrlich, wenn Daten fehlen.',
  },
  'P4–P5': {
    title: 'Befunde und Ursachen',
    summary: 'Aus Messungen werden verständliche Befunde und kleine Hörtests.',
  },
  'P6–P7': {
    title: 'Probeeq aktiv machen',
    summary: 'Probeeq wird bedienbar und setzt sichere Vorschläge von Gen um.',
  },
  'P8–P9': {
    title: 'Entmaskierung und Release',
    summary: 'Die Familie wird für lange Sitzungen und die Auslieferung gehärtet.',
  },
};
