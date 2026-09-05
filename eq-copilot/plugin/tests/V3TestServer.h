// V3TestServer.h — das GETEILTE v3-Testdouble (NAK-180 Nacharbeit 2, WN-08).
//
// Bis hierher lebte dieser Server ausschliesslich in `IpcTestMain.cpp`. Das
// war der Grund, warum `EqCopSonde013PassageStateTest::r01Resync` den
// Handschlag nur ueber Testhaken fahren konnte: kein Bein ausser B10 hatte
// einen Gegenueber. Damit blieb der ECHTE Lebenszyklus - `start()`, Pipe,
// Hello/Welcome, Heartbeat-Takt, P0-Enqueue, Wire-Commit - in jedem anderen
// Bein ungemessen (EP-18/R3b, Wiederpruefung 1 WN-08).
//
// ⚠️ Dieser Header wird INNERHALB des anonymen Namensraums seines Beins
// eingebunden und setzt voraus, dass davor schon stehen:
//   * `using namespace nakama::ipc;`
//   * <windows.h>, <atomic>, <mutex>, <thread>, <vector>, <string>
//     und juce_core (fuer `testExeErwartung`)
// Er wird bewusst NICHT zu einer eigenen Uebersetzungseinheit: der Server ist
// Testmaterial, keine Bibliothek, und soll in jedem Bein genau so gebaut
// werden, wie dieses Bein uebersetzt.

/// Eigener 32-Zeichen-Fueller. Bewusst NICHT `hex32`: die Beine, die diesen
/// Header einbinden, fuehren unter diesem Namen ihre eigene Funktion mit
/// anderer Signatur, und eine stille Ueberladung waere genau die Art
/// Namenskollision, die erst im dritten Bein auffaellt.
inline std::string v3TestHex32 (char fuellzeichen) { return std::string (32, fuellzeichen); }

/// Die `command_id` aus einem P0-Text - der Server beantwortet damit
/// persistenzpflichtige Befehle. Mit dem Server hierher gezogen (WN-08):
/// eine Haelfte des Doubles ohne die andere ist keine Haelfte.
std::string commandIdAusJson (const std::string& text)
{
    constexpr const char* marker = "\"command_id\":\"";
    const auto anfang = text.find (marker);
    if (anfang == std::string::npos)
        return {};
    const auto wert = anfang + std::char_traits<char>::length (marker);
    if (wert + 32 >= text.size() || text[wert + 32] != '"')
        return {};
    const auto id = text.substr (wert, 32);
    return istHex32 (id) ? id : std::string {};
}

//==============================================================================
// Ein v3-Server auf einer eigenen Testpipe. Er spricht genau den Bootstrap aus
// §53.9 und ist absichtlich klein: er ist der GEGENUEBER des Clients, nicht
// eine zweite Brokerimplementierung.
class TestServer
{
public:
    std::atomic<int> verbindungen { 0 };
    std::atomic<int> p0 { 0 }, p1 { 0 }, p2 { 0 };
    std::atomic<int> abgewiesen { 0 };
    std::atomic<bool> sendeKaputtenFrame { false };
    /// Der Server macht den Bootstrap, schickt sein welcome und liest DANACH
    /// nie wieder. So laeuft der Sendeweg des Clients garantiert voll — ohne
    /// diesen Gegenspieler waere ein P0-Ueberlauf im Betrieb ein Rennen.
    std::atomic<bool> nichtLesen { false };
    /// NAK-134/D-K04, D-K05: nach bestandener Serverpruefung und gelesenem
    /// Hello GAR KEIN welcome schicken. `eineVerbindung` laeuft in seine
    /// Lesefrist und gibt `false` — der Auth-Erfolg allein setzt den Backoff
    /// also nicht zurueck (D-P02).
    std::atomic<bool> welcomeAusbleiben { false };
    /// Welcome-Varianten fuer die Strenge des TelemetryClient (T2-Befund 10):
    /// als P2-Familie statt P0, mit fremder link_id, ohne `protocol`.
    std::atomic<bool> welcomeAlsP2 { false };
    std::atomic<bool> welcomeFremdeLinkId { false };
    std::atomic<bool> welcomeOhneProtokoll { false };
    /// Welcome-Varianten fuer den VERTRAG (T2-Befund 3): typfalsches
    /// `broker_version` (`null`), ein Zusatzfeld trotz
    /// `additionalProperties:false`, und eine Version ueber der Laengengrenze.
    std::atomic<bool> welcomeTypfalsch { false };
    std::atomic<bool> welcomeZusatzfeld { false };
    std::atomic<bool> welcomeLangeVersion { false };
    /// Nach dem welcome einen korrekt gerahmten P2-Frame ueber die CONTROL-
    /// Verbindung schicken (T2-Befund 4).
    std::atomic<bool> sendeP2AufControl { false };
    /// So viele P0-Frames nach dem welcome so schnell wie moeglich
    /// hinterherschicken (T2-Befund 5).
    std::atomic<int> frameFlut { 0 };
    /// Dasselbe auf der TELEMETRIE-Verbindung, aber mit P2-Frames: dort ist
    /// P2 die vertragsgemaesse Familie, und geprueft wird die Rate.
    std::atomic<int> frameFlutTelemetrieP2 { 0 };
    /// Ein einzelner P0-Frame auf der Telemetrieverbindung — dort
    /// vertragswidrig (§33.1), der Client muss schliessen.
    std::atomic<bool> sendeP0AufTelemetrie { false };
    /// Die naechste Telemetrieverbindung einmalig schliessen — der Fall, den
    /// der Leerlauf ohne Lesen nie bemerkte (T2-Befund 2).
    std::atomic<bool> telemetrieSchliessen { false };
    /// Nach der ERSTEN P0-Antwort nicht mehr lesen (Matrix `B-CC-06`): der
    /// ACK liegt dann beim Client, waehrend dessen P1-Weg vollaeuft. Genau in
    /// dieser Lage uebersprang die alte Fassung den Lesepfad.
    std::atomic<bool> nachErsterP0AntwortNichtLesen { false };
    /// Zusammen mit `nichtLesen`: so viele Millisekunden nach dem welcome
    /// einen UNGEFRAGTEN P0-ACK schicken (Matrix `B-CC-07`, NAK-104). Zu
    /// diesem Zeitpunkt steht der Client bereits in seinem Write, weil der
    /// Peer nicht liest — der ACK liegt also beim Client, WAEHREND dessen
    /// Schreibfrist laeuft. Nur so wird der Fehlerpfad nach dem Zeitlimit
    /// wirklich gefahren; `nachErsterP0AntwortNichtLesen` liefert den ACK
    /// dagegen nach einem ERFOLGREICHEN Write.
    std::atomic<int> ackNachNichtLesenMs { 0 };
    /// Phase-B-Antworten auf persistenzpflichtige Befehle. 0 = keine,
    /// 1..5 entsprechen der schemafesten Reihenfolge angewandt, abgelehnt,
    /// konflikt, abgelaufen, idempotent_wiederholt.
    std::atomic<int> commandAckArt { 0 };
    std::atomic<int> commandAckVerzoegerungMs { 0 };
    std::atomic<bool> commandVorAckSchliessen { false };
    std::atomic<bool> commandAckMitEventUuid { false };
    /// 1 = Revision mit fuehrender Null, 2 = numerischer Fehler-state_hash,
    /// 3 = unbekannter Fehlercode, 4 = Erfolgs-ACK ohne Pflicht-Hash.
    std::atomic<int> commandAckVertragsbruch { 0 };
    /// Minor fuer nach dem Welcome erzeugte P0-Antworten. Default 0 haelt
    /// die Rueckwaertskompatibilitaet sichtbar; einzelne Tests setzen 1 bzw.
    /// einen unbekannten Wert.
    std::atomic<int> controlAntwortMinor { 0 };
    std::mutex textMutex;
    std::string letztesControlHello, letztesTelemetryHello, letzterAbweisungsgrund;
    /// Jeder empfangene P0-/P1-Payload, woertlich. Damit laesst sich pruefen,
    /// ob eine bestimmte Nachricht WIRKLICH angekommen ist — eine Zahl allein
    /// sagt nichts darueber, WELCHE fehlt.
    std::vector<std::string> p0Texte, p1Texte;
    std::vector<std::uint8_t> p0Minors, p1Minors;
    /// Vollstaendiger zuletzt empfangener Telemetrie-Wireframe inklusive
    /// u32-Laengenpraefix. Damit misst der Sender-Test schema_minor an Offset
    /// 7 des tatsaechlich geschriebenen Rahmens.
    std::vector<std::uint8_t> letzterTelemetryWire;

    /// Kopplungswerte. Sie sind veraenderlich: eine neu aufgebaute
    /// Control-Verbindung bekommt beim echten Broker eine frische `link_id`,
    /// und genau das muss der TelemetryClient bemerken (T2-Befund 2).
    std::string kopplungLinkId()
    {
        std::lock_guard<std::mutex> l (kopplungMutex);
        return linkId;
    }
    std::string kopplungChallenge()
    {
        std::lock_guard<std::mutex> l (kopplungMutex);
        return challenge;
    }
    void neueKopplung (std::string l2, std::string c2)
    {
        std::lock_guard<std::mutex> l (kopplungMutex);
        linkId = std::move (l2);
        challenge = std::move (c2);
    }

    std::mutex kopplungMutex;
    std::string linkId = v3TestHex32 ('a'), challenge = v3TestHex32 ('b');

    /// Startsynchronisation: `pipeSteht` wird erst gesetzt, wenn der Acceptor
    /// seine erste Instanz angelegt hat; `startGescheitert`, wenn er es nicht
    /// konnte (Name schon belegt).
    std::atomic<bool> pipeSteht { false }, startGescheitert { false };

    explicit TestServer (std::string pipeName) : name (std::move (pipeName)) {}
    ~TestServer() { stoppen(); }

    /// Kehrt erst zurueck, wenn die erste Pipeinstanz WIRKLICH existiert.
    /// Vorher war das ein Rennen: `acceptorLauf` legt sie im Thread an,
    /// waehrend der Aufrufer schon `namedPipeErreichbar` fragt. Fuer die
    /// Tests, die ueber einen `ControlClient` mit Warteschleife verbinden,
    /// blieb das folgenlos; die NAK-123-C-07-Tests ticken den Lifecycle
    /// dagegen sofort einmal an und sahen den Namen mal, mal nicht.
    bool starten()
    {
        laeuft.store (true);
        pipeSteht.store (false);
        startGescheitert.store (false);
        acceptor = std::thread ([this] { acceptorLauf(); });
        for (int i = 0; i < 1000; ++i)
        {
            if (pipeSteht.load() || startGescheitert.load())
                break;
            std::this_thread::sleep_for (std::chrono::milliseconds (2));
        }
        return pipeSteht.load();
    }

    void stoppen()
    {
        if (! laeuft.exchange (false))
        {
            if (acceptor.joinable()) acceptor.join();
            return;
        }
        wecken();
        if (acceptor.joinable())
            acceptor.join();
        {
            std::lock_guard<std::mutex> l (threadMutex);
            for (auto& t : verbindungsThreads)
                if (t.joinable())
                    t.join();
            verbindungsThreads.clear();
        }
    }

    const std::string& pipeName() const { return name; }

private:
    void wecken()
    {
        std::wstring breit (name.begin(), name.end());
        HANDLE h = CreateFileW (breit.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                                SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr);
        if (h != INVALID_HANDLE_VALUE)
            CloseHandle (h);
    }

    void acceptorLauf()
    {
        std::wstring breit (name.begin(), name.end());
        bool erste = true;
        while (laeuft.load())
        {
            HANDLE h = CreateNamedPipeW (
                breit.c_str(),
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | (erste ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0),
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                8, 65536, 65536, 0, nullptr);
            erste = false;
            if (h == INVALID_HANDLE_VALUE)
            {
                startGescheitert.store (true);
                return;
            }
            pipeSteht.store (true);

            OVERLAPPED ov {};
            ov.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);
            BOOL ok = ConnectNamedPipe (h, &ov);
            DWORD f = GetLastError();
            if (! ok && f == ERROR_IO_PENDING)
            {
                WaitForSingleObject (ov.hEvent, INFINITE);
                DWORD dummy = 0;
                ok = GetOverlappedResult (h, &ov, &dummy, FALSE);
                f = ok ? 0 : GetLastError();
            }
            CloseHandle (ov.hEvent);
            if (! ok && f != ERROR_PIPE_CONNECTED)
            {
                CloseHandle (h);
                continue;
            }
            if (! laeuft.load())
            {
                DisconnectNamedPipe (h);
                CloseHandle (h);
                return;
            }
            ++verbindungen;
            std::lock_guard<std::mutex> l (threadMutex);
            verbindungsThreads.emplace_back ([this, h] { verbindungLauf (h); });
        }
    }

    /// Ein Lesevorgang mit Frist auf einem overlapped Serverhandle.
    static int lesen (HANDLE h, std::uint8_t* ziel, DWORD n, int fristMs)
    {
        OVERLAPPED ov {};
        ov.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);
        DWORD gelesen = 0;
        int ergebnis = -1;
        if (ReadFile (h, ziel, n, nullptr, &ov))
        {
            GetOverlappedResult (h, &ov, &gelesen, FALSE);
            ergebnis = static_cast<int> (gelesen);
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject (ov.hEvent, static_cast<DWORD> (fristMs)) == WAIT_OBJECT_0
                && GetOverlappedResult (h, &ov, &gelesen, FALSE))
                ergebnis = static_cast<int> (gelesen);
            else
            {
                CancelIoEx (h, &ov);
                DWORD verworfen = 0;
                GetOverlappedResult (h, &ov, &verworfen, TRUE);
                ergebnis = verworfen > 0 ? static_cast<int> (verworfen) : 0;  // 0 = nichts
            }
        }
        CloseHandle (ov.hEvent);
        return ergebnis;
    }

    static bool schreiben (HANDLE h, const std::uint8_t* daten, std::size_t n)
    {
        OVERLAPPED ov {};
        ov.hEvent = CreateEventW (nullptr, TRUE, FALSE, nullptr);
        DWORD geschrieben = 0;
        bool ok = false;
        if (WriteFile (h, daten, static_cast<DWORD> (n), nullptr, &ov))
            ok = GetOverlappedResult (h, &ov, &geschrieben, FALSE) != 0;
        else if (GetLastError() == ERROR_IO_PENDING)
            ok = WaitForSingleObject (ov.hEvent, 5000) == WAIT_OBJECT_0
              && GetOverlappedResult (h, &ov, &geschrieben, FALSE) != 0;
        CloseHandle (ov.hEvent);
        return ok && geschrieben == n;
    }

    void verbindungLauf (HANDLE h)
    {
        std::vector<std::uint8_t> roh;
        std::uint8_t puffer[4096];

        // ── Bootstrap: u32-Praefix + JSON, hoechstens 16 KiB ──────────────
        std::string helloJson;
        while (laeuft.load())
        {
            if (roh.size() >= 4)
            {
                const std::uint32_t n = static_cast<std::uint32_t> (roh[0])
                                      | (static_cast<std::uint32_t> (roh[1]) << 8)
                                      | (static_cast<std::uint32_t> (roh[2]) << 16)
                                      | (static_cast<std::uint32_t> (roh[3]) << 24);
                if (n == 0 || n > kMaxBootstrapBytes)
                {
                    ++abgewiesen;
                    schliessen (h);
                    return;
                }
                if (roh.size() >= 4 + n)
                {
                    helloJson.assign (reinterpret_cast<const char*> (roh.data() + 4), n);
                    roh.erase (roh.begin(), roh.begin() + 4 + static_cast<std::ptrdiff_t> (n));
                    break;
                }
            }
            const int gelesen = lesen (h, puffer, sizeof (puffer), 3000);
            if (gelesen <= 0)
            {
                schliessen (h);
                return;
            }
            roh.insert (roh.end(), puffer, puffer + gelesen);
        }

        const bool istTelemetry = helloJson.find ("\"telemetry\"") != std::string::npos;
        {
            std::lock_guard<std::mutex> l (textMutex);
            (istTelemetry ? letztesTelemetryHello : letztesControlHello) = helloJson;
        }

        if (istTelemetry)
        {
            // Kopplung: link_id UND challenge muessen aus dem eigenen welcome
            // stammen. Fehlt eines, wird geschlossen (§53.9).
            const bool passt =
                helloJson.find ("\"link_id\":\"" + kopplungLinkId() + "\"") != std::string::npos
                && helloJson.find ("\"challenge\":\"" + kopplungChallenge() + "\"")
                       != std::string::npos;
            if (! passt)
            {
                ++abgewiesen;
                {
                    std::lock_guard<std::mutex> l (textMutex);
                    letzterAbweisungsgrund = "telemetry ungekoppelt";
                }
                schliessen (h);
                return;
            }
        }

        // NAK-134/D-K04, D-K05: die Serverpruefung des Clients ist an dieser
        // Stelle bestanden und sein Hello gelesen — aber es kommt KEIN
        // welcome. Die Verbindung bleibt offen, damit der Client wirklich in
        // seine Lesefrist laeuft statt ein Ende zu sehen.
        if (welcomeAusbleiben.load())
        {
            while (laeuft.load() && welcomeAusbleiben.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (20));
            schliessen (h);
            return;
        }

        // ── welcome, bereits v3-gerahmt ───────────────────────────────────
        const std::string ausgegebeneLink =
            welcomeFremdeLinkId.load() ? v3TestHex32 ('9') : kopplungLinkId();
        std::string welcome = std::string ("{\"type\":\"welcome\"");
        if (! welcomeOhneProtokoll.load())
            welcome += ",\"protocol\":3";
        welcome += ",\"broker_version\":";
        if (welcomeTypfalsch.load())
            welcome += "null";                                  // Typ statt Text
        else if (welcomeLangeVersion.load())
            welcome += "\"" + std::string (65, 'v') + "\"";      // maxLength 64
        else
            welcome += "\"test\"";
        welcome += std::string (",\"broker_epoch\":\"") + v3TestHex32 ('c')
                 + "\",\"link_id\":\"" + ausgegebeneLink
                 + "\",\"challenge\":\"" + kopplungChallenge() + "\"";
        if (welcomeZusatzfeld.load())
            welcome += ",\"extra\":1";                          // additionalProperties:false
        welcome += "}";
        std::vector<std::uint8_t> aus;
        envelopeSchreiben (welcomeAlsP2.load() ? Familie::p2 : Familie::p0, 0,
                           reinterpret_cast<const std::uint8_t*> (welcome.data()),
                           welcome.size(), aus);
        if (sendeKaputtenFrame.load())
            aus[4] = 9;  // encoding unbekannt — der Client muss schliessen
        if (! schreiben (h, aus.data(), aus.size()))
        {
            schliessen (h);
            return;
        }

        // Ein korrekt gerahmter P2-Frame auf der CONTROL-Verbindung: der
        // Client darf ihn NICHT an `beiAntwort` weiterreichen (T2-Befund 4).
        if (sendeP2AufControl.load() && ! istTelemetry)
        {
            std::uint8_t nutz[32];
            std::memset (nutz, 0x11, sizeof (nutz));
            std::vector<std::uint8_t> p2rahmen;
            envelopeSchreiben (Familie::p2, 0, nutz, sizeof (nutz), p2rahmen);
            if (! schreiben (h, p2rahmen.data(), p2rahmen.size()))
            {
                schliessen (h);
                return;
            }
        }

        // Ein P0-Frame auf der Telemetrieverbindung: dort traegt der Vertrag
        // ausschliesslich P2 (T2-Befund 4, Telemetrie-Haelfte).
        if (sendeP0AufTelemetrie.load() && istTelemetry)
        {
            const std::string beliebig = "{\"type\":\"heartbeat_ack\"}";
            std::vector<std::uint8_t> p0rahmen;
            envelopeSchreiben (Familie::p0, 0,
                               reinterpret_cast<const std::uint8_t*> (beliebig.data()),
                               beliebig.size(), p0rahmen);
            if (! schreiben (h, p0rahmen.data(), p0rahmen.size()))
            {
                schliessen (h);
                return;
            }
        }

        // P2-Flut auf der Telemetrieverbindung: vertragsgemaesse Familie,
        // aber ueber der Rate (T2-Befund 5, Telemetrie-Haelfte).
        if (const int flutT = frameFlutTelemetrieP2.load(); flutT > 0 && istTelemetry)
        {
            std::uint8_t nutz[16];
            std::memset (nutz, 0x33, sizeof (nutz));
            std::vector<std::uint8_t> einer;
            envelopeSchreiben (Familie::p2, 0, nutz, sizeof (nutz), einer);
            std::vector<std::uint8_t> haeppchen;
            for (int i = 0; i < 64; ++i)
                haeppchen.insert (haeppchen.end(), einer.begin(), einer.end());
            for (int gesendet = 0; gesendet < flutT && laeuft.load(); gesendet += 64)
                if (! schreiben (h, haeppchen.data(), haeppchen.size()))
                    break;
        }

        // Mehr Frames, als die Nachrichtenrate erlaubt (T2-Befund 5). In
        // Haeppchen, damit der Server nicht 5 s in einem einzigen Write steht,
        // wenn der Client die Verbindung — richtigerweise — schliesst.
        if (const int flut = frameFlut.load(); flut > 0 && ! istTelemetry)
        {
            const std::string ack =
                "{\"type\":\"heartbeat_ack\",\"sequence\":0,\"duplicate_instance_id\":false}";
            std::vector<std::uint8_t> einer;
            envelopeSchreiben (Familie::p0, 0,
                               reinterpret_cast<const std::uint8_t*> (ack.data()),
                               ack.size(), einer);
            std::vector<std::uint8_t> haeppchen;
            for (int i = 0; i < 64; ++i)
                haeppchen.insert (haeppchen.end(), einer.begin(), einer.end());
            for (int gesendet = 0; gesendet < flut && laeuft.load(); gesendet += 64)
                if (! schreiben (h, haeppchen.data(), haeppchen.size()))
                    break;   // der Client hat geschlossen — genau das ist das Ziel
        }

        if (nichtLesen.load())
        {
            // `B-CC-07`: der ACK trifft ein, waehrend der Client schon
            // schreibt. Erst warten, damit sein Sendeweg sicher steht.
            if (const int nachMs = ackNachNichtLesenMs.load();
                nachMs > 0 && ! istTelemetry)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (nachMs));
                const std::string ack =
                    "{\"type\":\"heartbeat_ack\",\"sequence\":7,\"duplicate_instance_id\":false}";
                std::vector<std::uint8_t> rahmen;
                envelopeSchreiben (Familie::p0, 0,
                                   reinterpret_cast<const std::uint8_t*> (ack.data()),
                                   ack.size(), rahmen);
                schreiben (h, rahmen.data(), rahmen.size());
            }
            while (laeuft.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (20));
            schliessen (h);
            return;
        }

        // ── ab hier nur noch v3-Frames ────────────────────────────────────
        StromLeser leser;
        std::vector<std::uint8_t> wirePuffer;
        auto wireMerken = [&] (const std::uint8_t* daten, std::size_t n)
        {
            if (! istTelemetry || n == 0)
                return;
            wirePuffer.insert (wirePuffer.end(), daten, daten + n);
            while (wirePuffer.size() >= 4)
            {
                const std::uint32_t frameLen = static_cast<std::uint32_t> (wirePuffer[0])
                    | (static_cast<std::uint32_t> (wirePuffer[1]) << 8)
                    | (static_cast<std::uint32_t> (wirePuffer[2]) << 16)
                    | (static_cast<std::uint32_t> (wirePuffer[3]) << 24);
                if (frameLen < kKopfBytes || frameLen > kMaxFrameBytes)
                    return;
                const auto gesamt = static_cast<std::size_t> (4u + frameLen);
                if (wirePuffer.size() < gesamt)
                    return;
                {
                    std::lock_guard<std::mutex> l (textMutex);
                    letzterTelemetryWire.assign (wirePuffer.begin(),
                                                   wirePuffer.begin() + gesamt);
                }
                wirePuffer.erase (wirePuffer.begin(), wirePuffer.begin() + gesamt);
            }
        };
        wireMerken (roh.data(), roh.size());
        leser.fuettern (roh.data(), roh.size());
        while (laeuft.load())
        {
            // Einmalig: die Telemetriepipe schliessen. Der echte Broker tut
            // das, wenn die zugehoerige Control-Verbindung endet.
            if (istTelemetry && telemetrieSchliessen.exchange (false))
            {
                schliessen (h);
                return;
            }
            bool weiter = true;
            while (weiter)
            {
                const auto e = leser.naechster();
                if (e.art == StromLeser::Art::unvollstaendig) { weiter = false; break; }
                if (e.art == StromLeser::Art::verstoss)
                {
                    ++abgewiesen;
                    schliessen (h);
                    return;
                }
                if (e.kopf.familie == Familie::p0)
                {
                    ++p0;
                    // heartbeat -> heartbeat_ack, sonst nichts.
                    const std::string text (reinterpret_cast<const char*> (e.payload),
                                            e.payloadLaenge);
                    {
                        std::lock_guard<std::mutex> l (textMutex);
                        p0Texte.push_back (text);
                        p0Minors.push_back (e.kopf.schemaMinor);
                    }
                    if (text.find ("\"heartbeat\"") != std::string::npos)
                    {
                        const std::string ack =
                            "{\"type\":\"heartbeat_ack\",\"sequence\":0,\"duplicate_instance_id\":false}";
                        std::vector<std::uint8_t> antwort;
                        envelopeSchreiben (Familie::p0,
                                           static_cast<std::uint8_t> (controlAntwortMinor.load()),
                                           reinterpret_cast<const std::uint8_t*> (ack.data()),
                                           ack.size(), antwort);
                        if (! schreiben (h, antwort.data(), antwort.size()))
                        {
                            schliessen (h);
                            return;
                        }
                        if (nachErsterP0AntwortNichtLesen.load())
                        {
                            while (laeuft.load())
                                std::this_thread::sleep_for (
                                    std::chrono::milliseconds (20));
                            schliessen (h);
                            return;
                        }
                    }
                    else if (const auto commandId = commandIdAusJson (text);
                             ! commandId.empty())
                    {
                        if (commandVorAckSchliessen.exchange (false))
                        {
                            schliessen (h);
                            return;
                        }

                        const int art = commandAckArt.load();
                        if (art >= 1 && art <= 5)
                        {
                            if (const int pause = commandAckVerzoegerungMs.load(); pause > 0)
                                std::this_thread::sleep_for (
                                    std::chrono::milliseconds (pause));
                            static constexpr const char* ergebnisse[] = {
                                "", "angewandt", "abgelehnt", "konflikt",
                                "abgelaufen", "idempotent_wiederholt"
                            };
                            const int bruch = commandAckVertragsbruch.load();
                            std::string ack = "{\"type\":\"command_ack\",\"command_id\":\""
                                + commandId + "\",\"ergebnis\":\"" + ergebnisse[art]
                                + "\",\"state_revision\":"
                                + (bruch == 1 ? "01" : "7");
                            if ((art == 1 || art == 5) && bruch != 4)
                                ack += ",\"state_hash\":\"" + std::string (64, 'd') + "\"";
                            if (bruch == 2)
                                ack += ",\"state_hash\":17";
                            if (bruch == 3)
                                ack += ",\"code\":\"nicht_im_schema\"";
                            if (commandAckMitEventUuid.load())
                                ack += ",\"event_uuid\":\"" + v3TestHex32 ('e') + "\"";
                            ack += "}";

                            std::vector<std::uint8_t> antwort;
                            envelopeSchreiben (Familie::p0, 0,
                                reinterpret_cast<const std::uint8_t*> (ack.data()),
                                ack.size(), antwort);
                            if (! schreiben (h, antwort.data(), antwort.size()))
                            {
                                schliessen (h);
                                return;
                            }
                        }
                    }
                }
                else if (e.kopf.familie == Familie::p1)
                {
                    ++p1;
                    std::lock_guard<std::mutex> l (textMutex);
                    p1Minors.push_back (e.kopf.schemaMinor);
                    p1Texte.emplace_back (reinterpret_cast<const char*> (e.payload),
                                          e.payloadLaenge);
                }
                else                                    ++p2;
            }

            const int gelesen = lesen (h, puffer, sizeof (puffer), 100);
            if (gelesen < 0)
                break;
            if (gelesen > 0)
            {
                wireMerken (puffer, static_cast<std::size_t> (gelesen));
                leser.fuettern (puffer, static_cast<std::size_t> (gelesen));
            }
        }
        schliessen (h);
    }

    static void schliessen (HANDLE h)
    {
        FlushFileBuffers (h);
        DisconnectNamedPipe (h);
        CloseHandle (h);
    }

    std::string name;
    std::atomic<bool> laeuft { false };
    std::thread acceptor;
    std::mutex threadMutex;
    std::vector<std::thread> verbindungsThreads;
};

std::atomic<unsigned> namensFolge { 0 };

std::string testPipeName (const char* fall)
{
    // Im PROBE-Namensraum, demselben, den die beiden Probeprogramme als
    // einzigen zulassen (§48.3 / T2-Befund 7 vom 2026-08-29).
    return std::string (kPipePraefixProbe) + "test."
         + std::to_string ((int) GetCurrentProcessId()) + "."
         + std::to_string ((int) namensFolge.fetch_add (1)) + "." + fall;
}

ServerErwartung testExeErwartung (std::uint32_t pid = 0)
{
    const auto exe = juce::File::getSpecialLocation (
        juce::File::currentExecutableFile).getFullPathName();
    return serverErwartungFuerTestdatei (std::wstring (exe.toWideCharPointer()), pid);
}

