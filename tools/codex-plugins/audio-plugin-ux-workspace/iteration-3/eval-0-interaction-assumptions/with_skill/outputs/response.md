Keine zusätzliche Geschmacks- oder Richtungsrunde. Vor dem ersten Figma-Eingriff braucht die Seite aber diesen kompakten Strukturvertrag; er öffnet deine Gestaltung nicht erneut.

**Human Need und Fokus**

Produzent arbeitet in FL Studio mit geteilter Aufmerksamkeit zwischen Gehör, DAW und Plugin → er will das richtige Band der richtigen Spur verändern, ohne einen Draft oder eine Vergleichsspur für den bestätigten Klang zu halten → dafür müssen Ziel, Zustandsautorität und Rückweg während der Geste sichtbar bleiben → die bestätigte Kurve der gewählten Sonde beziehungsweise des Masters besitzt den Ruhefokus; bei Auswahl übernimmt das Band den Arbeitsfokus, ohne den Gesamtgraphen zu verdrängen → prüfbar daran, dass der Nutzer vor der Geste Ziel und hörbaren Zustand benennen und danach die Änderung zurücknehmen kann.

**Figma-Struktur**

- **Persistent:** dominanter EQ-Graph, gewähltes Ziel, eindeutiges Bearbeitungsziel bei zwei Kurven, bestätigte Kurve, Bypass, Undo und relevante Ausnahmezustände.
- **Kontextuell:** Werte und Präzisionswege des gewählten Bands sowie ein vorhandener Draft mit Audition, Apply und Reject.
- **Bedingt:** zweite EQ-Spur, Dynamikdetails und das eingeklappte Global-Panel.
- **Transient:** Frequenz-, Gain- und Q-Wert während Hover, Zug und Feinjustage.
- **Exceptional:** Broker-/Sonden-Ausfall, Stale durch externe Automation, Schutzverletzung und fehlendes Signal.
- **Unavailable:** Ein im Bild vorgesehener, technisch noch nicht verfügbarer Handgriff bleibt mit Grund gesperrt. Konkret zeigt die aktuelle Bildreferenz Mix, während der aktuelle Verhaltensvertrag Mix weiterhin als `PLANNED` führt; die Struktur darf den Platz reservieren, aber keinen funktionierenden Regler behaupten. ([Bildautorität](C:/Users/phili/Documents/Nakama/design/abnahmen/2026-08-25-rework-referenz-drei-designs.md), [UX-Vertrag](C:/Users/phili/Documents/Nakama/UX-CONTRACT.md))

Die Arbeitsfläche bleibt 760×430; Quellenwahl liegt direkt auf der EQ-Seite, Master bleibt ein eigenes Ziel und die optionale zweite Spur liegt im selben Graph. Diese aktuellen Vorgaben stammen aus [Größen- und Logikentscheid](C:/Users/phili/Documents/Nakama/design/abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md) und [EQ-Spielregeln](C:/Users/phili/Documents/Nakama/design/docs/ui-spielregeln-eq-zentrale.md). Visuell werden ausschließlich die drei aktuellen Rework-PNGs übersetzt; frühere Figma-Nodes sind dafür Verlauf. ([Designrouter](C:/Users/phili/Documents/Nakama/design/LIES-MICH.md))

**Mapping und Icons**

Horizontal = Frequenz und nach oben = positiver Gain bleiben als dein aktueller Entscheid gesetzt. Das ist die Nakama-Zuordnung, kein behauptetes Naturgesetz. Griff, Kurve und Zahlenwert müssen gemeinsam reagieren; BAND-Auswahl, Tastatur und Zahleneingabe führen zum gleichen Ergebnis. Icon-only lehne ich ab: Undo, Bypass und Schutz bekommen sichtbaren Namen oder Zustand sowie semantische Name-/Role-/Value-/Action-Informationen.

Der Forschungsweg war ausdrücklich:

`wissen/LIES-MICH.md` → `wissen/AGENTS.md` → `wissen/INDEX.md` → `wissen/ux/LIES-MICH.md` → Eintrag `w-2026-08-26-audio-interaktion-kontext-statt-annahmen`, Pfad `wissen/ux/2026-08-26-audio-interaktion-kontext-statt-annahmen.md`, Prüfstand `quellen-geprueft`.

Dort macht die sichtbare Folie bei [32:40 „Does UP always equal MORE?“](https://www.youtube.com/watch?v=E3zUIR8xkrw&t=1960s) räumliche Zuordnungen zur prüfbaren Annahme. Das ist keine Stilvorlage. Der daraus abgeleitete Test: ohne Coaching Richtung und Icon-Funktion vorhersagen lassen; Zögern, Fehlvorhersage und Recovery notieren; bei Fehlern beschriften oder das Symbol ändern.

Damit sind Fokus, Sichtbarkeit, Mapping, Platzierung und Gegenbeweis vor Figma festgelegt. **Open** bleibt nur, ob die Icons in Nakamas Zielkontext tatsächlich verstanden werden; „jeder versteht sie“ wird erst nach diesem Test zur belegten Aussage.
