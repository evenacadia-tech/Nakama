// sondenprobe.mjs — die Inhaltspruefung der Messbank.
//
// pruefen.mjs fragt: LEBT das Blatt? (JS-Fehler, stumm, Standband)
// Diese hier fragt: SAGT es die Wahrheit? — und zwar so, dass jede Antwort
// scheitern kann. Ein Gate, das nicht scheitern kann, beweist nichts
// (werkzeug/PRUEFLISTE.md 7.1); deshalb sind zwei der Proben Gegenproben,
// die einen kuenstlichen Fehler einbauen und verlangen, dass er gemeldet wird.
//
// AUFRUF
//   node werkzeug/pruefung/sondenprobe.mjs alles
//   node werkzeug/pruefung/sondenprobe.mjs zahlen
//   node werkzeug/pruefung/sondenprobe.mjs waage | anordnungen | ratsche
//                                        | deckel | beleg | grenzfall
//                                        | zustaende
//                                        | gegenprobe-ueberlauf
//                                        | gegenprobe-verdeckung
//                                        | formfaktor | kachel | export
//                                        | annahmen | maschine
//
// Exit 1 bei Befund. BRAUCHT: playwright-core + Chromium (wird gesucht,
// nicht behauptet — dieselbe Suche wie in pruefen.mjs).

import { createRequire } from 'node:module'
import { readdirSync, existsSync } from 'node:fs'
import { resolve, join } from 'node:path'
import { homedir } from 'node:os'

const require = createRequire(import.meta.url)
const WURZEL = resolve(new URL('.', import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1'), '../..')
const WERK = join(WURZEL, 'werkzeug')

function ladePlaywright() {
  const orte = ['playwright-core', 'playwright',
    join(homedir(), 'Klarheit/node_modules/playwright-core'),
    join(WURZEL, 'node_modules/playwright-core')]
  for (const o of orte) { try { return require(o) } catch {} }
  console.error('playwright-core nicht gefunden. Gesucht in:\n  ' + orte.join('\n  '))
  process.exit(2)
}
function findeChromium() {
  const basen = [join(homedir(), 'AppData/Local/ms-playwright'),
                 join(homedir(), '.cache/ms-playwright')]
  for (const b of basen) {
    if (!existsSync(b)) continue
    for (const k of readdirSync(b).filter(d => d.startsWith('chromium-')).sort().reverse())
      for (const rel of ['chrome-win64/chrome.exe', 'chrome-linux/chrome',
                         'chrome-mac/Chromium.app/Contents/MacOS/Chromium']) {
        const pf = join(b, k, rel); if (existsSync(pf)) return pf
      }
  }
  for (const pf of ['C:/Program Files/Google/Chrome/Application/chrome.exe',
                    '/usr/bin/chromium', '/usr/bin/google-chrome']) if (existsSync(pf)) return pf
  return null
}

const { chromium } = ladePlaywright()
const exe = findeChromium()
const browser = await chromium.launch(exe ? { executablePath: exe } : {})

async function oeffne(datei, breite = 1600, hoehe = 1400) {
  const ctx = await browser.newContext({ viewport: { width: breite, height: hoehe } })
  const page = await ctx.newPage()
  const fehler = []
  page.on('pageerror', e => fehler.push('pageerror: ' + e.message.split('\n')[0]))
  page.on('console', m => { if (m.type() === 'error') fehler.push('console: ' + m.text().slice(0, 160)) })
  await page.goto('file:///' + join(WERK, datei).replace(/\\/g, '/'),
    { waitUntil: 'load', timeout: 20000 })
  await page.waitForTimeout(900)          // zweiter Frame: Schriftmasse stehen
  return { ctx, page, fehler }
}

const befunde = []
const sag = (ok, zeile) => { console.log((ok ? 'ok   ' : 'FEHL ') + zeile); if (!ok) befunde.push(zeile) }

/* ---------------------------------------------------------------- zahlen --
   Behauptung des Blattes: "Alle Zahlen sind hier im Browser gemessen, keine
   steht im Quelltext." Zwei Beweisschritte, denn der erste allein wuerde eine
   geschickte Faelschung nicht finden:
     1. Die angezeigte Zahl == die unabhaengig gelesene Rechteckhoehe.
     2. Aendert man den Inhalt, MUESSEN sich die Zahlen aendern. Eine hart
        geschriebene Zahl bleibt stehen — das ist der Falsifikationsschritt. */
async function probeZahlen() {
  const { ctx, page, fehler } = await oeffne('sonde-messung.html')
  const lies = () => page.evaluate(() =>
    [...document.querySelectorAll('.anordnung')].map(a => {
      const zahl = k => { const e = a.querySelector(`[data-mess="${k}"]`)
        return e ? parseFloat(e.textContent) : null }
      /* Der unabhaengige Weg zur Zahl. Er darf NICHT derselbe sein wie im
         Blatt, sonst vergleicht die Probe eine Rechnung mit sich selbst —
         genau der Fehler, an dem die erste Fassung dieser Sonde scheiterte.
         Das Blatt nimmt clientHeight bzw. offsetHeight; hier wird das
         Rechteck genommen und durch den am Element selbst gelesenen Massstab
         geteilt (die Kacheln stehen unter transform:scale()).
         Fuer die KURVE wird zusaetzlich der Rahmen abgezogen: dort zaehlt der
         Zeichenraum. Fuer den BANDKASTEN nicht: dort zaehlt der Platzbedarf. */
      const skal = e => e.offsetWidth ? e.getBoundingClientRect().width / e.offsetWidth : 1
      const aussen = s => { const e = a.querySelector(s)
        return e ? e.getBoundingClientRect().height / skal(e) : 0 }
      const innen = s => { const e = a.querySelector(s); if (!e) return 0
        const st = getComputedStyle(e)
        return e.getBoundingClientRect().height / skal(e)
             - parseFloat(st.borderTopWidth) - parseFloat(st.borderBottomWidth) }
      return { id: a.dataset.id,
               gezeigtKurve: zahl('kurve'), echtKurve: innen('.kurve'),
               gezeigtBand: zahl('bandkasten'), echtBand: aussen('.bandliste') }
    }))
  const vorher = await lies()
  let schief = 0
  for (const a of vorher) {
    if (Math.abs(a.gezeigtKurve - a.echtKurve) > 1) { schief++
      sag(false, `${a.id}: Kurve gezeigt ${a.gezeigtKurve}, gemessen ${a.echtKurve}`) }
    if (Math.abs(a.gezeigtBand - a.echtBand) > 1) { schief++
      sag(false, `${a.id}: Bandkasten gezeigt ${a.gezeigtBand}, gemessen ${a.echtBand}`) }
  }
  // Falsifikation: auf einen Slot umschalten. Wer die Zahl aus dem Quelltext
  // nimmt, merkt davon nichts.
  await page.evaluate(() => { window.WELT.slots = 1; window.zeichnen() })
  await page.waitForTimeout(400)
  const nachher = await lies()
  const mitBand = vorher.filter(a => a.echtBand > 0).map(a => a.id)
  const unveraendert = mitBand.filter(id => {
    const v = vorher.find(x => x.id === id), n = nachher.find(x => x.id === id)
    return v.gezeigtBand === n.gezeigtBand
  })
  if (unveraendert.length) { schief++
    sag(false, `Bandkasten-Zahl aendert sich nicht bei 1 statt 8 Slots: ${unveraendert.join(', ')}`) }
  // Dritter Beweisschritt: der Massstab der Kachel darf die Zahlen NICHT
  // veraendern. Wer getBoundingClientRect nimmt, faellt genau hier durch.
  //
  // Bis 2026-08-21 war dieser Schritt WIRKUNGSLOS: --s wurde INLINE auf die
  // vorhandenen .rahmen gesetzt und unmittelbar danach zeichnen() gerufen —
  // das ersetzt genau diese Knoten (ziel.innerHTML = ''), der Massstab stand
  // sofort wieder auf .52 und beide Messungen lagen im selben Massstab.
  // Gemessen: .52 → 1 → .52. Jetzt haengt der Massstab an einer STILREGEL,
  // die den Neubau ueberlebt, und wird nachgelesen statt geglaubt.
  const massstab = () => page.evaluate(() =>
    parseFloat(getComputedStyle(document.querySelector('.rahmen'))
      .getPropertyValue('--s')) || 1)
  await page.evaluate(() => { window.WELT.slots = 8; window.zeichnen() })
  await page.waitForTimeout(400)
  const beiKlein = await lies()
  const sKlein = await massstab()
  await page.evaluate(() => {
    const st = document.createElement('style')
    st.id = 'probe-massstab'
    st.textContent = '.rahmen{--s:1 !important}'
    document.head.appendChild(st)
    window.zeichnen()
  })
  await page.waitForTimeout(400)
  const beiGross = await lies()
  const sGross = await massstab()
  // Hat der Massstab sich WIRKLICH geaendert? Ohne diese Frage vergleicht der
  // Schritt zweimal denselben Zustand und meldet zufrieden 0 Abweichungen —
  // genau der Fehler, gegen den die ganze Sonde gebaut wurde.
  if (!(sKlein < 0.99 && sGross > 0.99)) { schief++
    sag(false, `Massstab hat sich nicht geaendert (${sKlein} → ${sGross}) — `
      + 'dieser Schritt prueft ins Leere') }
  const abweichung = (a, b) => Math.abs(a.gezeigtKurve - b.gezeigtKurve) > 1
                            || Math.abs(a.gezeigtBand - b.gezeigtBand) > 1
  const wandert = beiKlein.filter(v => abweichung(v, beiGross.find(x => x.id === v.id)))
  wandert.forEach(v => { schief++
    const g = beiGross.find(x => x.id === v.id)
    sag(false, `${v.id}: Zahl haengt am Massstab — verkleinert Kurve ${v.gezeigtKurve}/Band `
      + `${v.gezeigtBand}, 1:1 Kurve ${g.gezeigtKurve}/Band ${g.gezeigtBand}`) })

  // Gegenprobe zum dritten Schritt: KANN er scheitern? Die Layoutmasse werden
  // kuenstlich massstabsabhaengig gemacht — genau die Regression, gegen die er
  // gebaut ist (jemand tauscht offsetHeight gegen getBoundingClientRect) — und
  // er muss sie melden. Danach wird geheilt und nachgesehen, dass die Zahlen
  // zurueckkommen; eine Gegenprobe, die den Patienten behaelt, ist keine.
  const gegen = await page.evaluate(async () => {
    // offsetHeight sitzt auf HTMLElement.prototype, clientHeight auf
    // Element.prototype — den Besitzer suchen, nicht raten.
    const besitzer = n => [HTMLElement.prototype, Element.prototype]
      .find(p => Object.getOwnPropertyDescriptor(p, n))
    const merk = ['offsetHeight', 'clientHeight'].map(n => {
      const p = besitzer(n)
      return [p, n, Object.getOwnPropertyDescriptor(p, n)]
    })
    for (const [p, n] of merk)
      Object.defineProperty(p, n, {
        configurable: true, get() { return this.getBoundingClientRect().height } })
    document.getElementById('probe-massstab').remove()      // zurueck auf klein
    window.zeichnen()
    await new Promise(a => setTimeout(a, 400))
    const lesen = () => [...document.querySelectorAll('.anordnung')].map(a => {
      const e = a.querySelector('[data-mess="kurve"]')
      return { id: a.dataset.id, k: e ? parseFloat(e.textContent) : null } })
    const klein = lesen()
    const st = document.createElement('style')
    st.id = 'probe-massstab2'
    st.textContent = '.rahmen{--s:1 !important}'
    document.head.appendChild(st)
    window.zeichnen()
    await new Promise(a => setTimeout(a, 400))
    const gross = lesen()
    for (const [p, n, d] of merk) Object.defineProperty(p, n, d)
    st.remove()
    window.zeichnen()
    await new Promise(a => setTimeout(a, 400))
    return { klein, gross, geheilt: lesen() }
  })
  const erkannt = gegen.klein.some((v, i) => Math.abs(v.k - gegen.gross[i].k) > 1)
  if (!erkannt) { schief++
    sag(false, 'BLIND: kuenstlich massstabsabhaengige Masse wurden NICHT gemeldet') }
  const geheilt = gegen.geheilt.every((v, i) =>
    Math.abs(v.k - beiKlein.find(x => x.id === v.id).gezeigtKurve) <= 1)
  if (!geheilt) { schief++
    sag(false, `Gegenprobe heilt nicht: ${JSON.stringify(gegen.geheilt)}`) }

  if (fehler.length) { schief++; sag(false, 'JS-Fehler: ' + fehler[0]) }
  sag(schief === 0, `zahlen: ${vorher.length} Anordnungen gegengerechnet, `
    + `${mitBand.length} auf Aenderung geprueft, ${beiKlein.length} bei echtem `
    + `Massstabswechsel ${sKlein}→${sGross} geprueft, Gegenprobe `
    + `${erkannt ? 'erkannt' : 'BLIND'} und ${geheilt ? 'geheilt' : 'NICHT geheilt'} `
    + `— ${schief} Abweichungen`)
  await ctx.close()
}

/* ----------------------------------------------------------------- waage --
   Beide Richtungen, nie nur die Hoehe (PRUEFLISTE 2.2). */
async function probeWaage() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const r = await page.evaluate(() =>
    [...document.querySelectorAll('#waage tr')].map(tr => ({
      name: tr.dataset.name,
      h: parseFloat(tr.querySelector('[data-mess="h"]')?.textContent),
      b: parseFloat(tr.querySelector('[data-mess="b"]')?.textContent)
    })))
  const luecken = r.filter(x => !(x.h > 0) || !(x.b > 0))
  luecken.forEach(x => sag(false, `Waage unvollstaendig: ${x.name} h=${x.h} b=${x.b}`))
  sag(luecken.length === 0 && r.length > 0,
    `waage: ${r.length} Bausteine, je Hoehe UND Breite — ${luecken.length ? 'unvollstaendig' : 'vollstaendig'}`)
  await ctx.close()
}

/* ----------------------------------------------------------- anordnungen --*/
async function probeAnordnungen() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const m = await page.evaluate(() => window.MESSUNG)
  const noetig = ['kurve', 'bandkasten', 'pxDb', 'draft', 'ueberlauf', 'verdeckung', 'deckel']
  const luecken = m.anordnungen.filter(a => noetig.some(k => a[k] === undefined || a[k] === null))
  luecken.forEach(a => sag(false, `Anordnung ${a.id} meldet nicht alles`))
  console.log('     ' + m.anordnungen.map(a =>
    `${a.id}: Kurve ${a.kurve} · Band ${a.bandkasten} · ${a.pxDb} px/dB · Draft ${a.draft} px`
  ).join('\n     '))
  sag(luecken.length === 0,
    `anordnungen=${m.anordnungen.length} ${luecken.length ? 'unvollstaendig' : 'vollstaendig'}`)
  await ctx.close()
}

/* --------------------------------------------------- gegenprobe ueberlauf --
   Kann ueberlauf() ueberhaupt scheitern? Ein kuenstlich zu grosser Block wird
   in ein Blatt gesetzt; wird er nicht gemeldet, ist die Messung blind. */
async function probeGegenUeberlauf() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const r = await page.evaluate(() => {
    const wf = document.querySelector('.anordnung .wf')
    const vorher = window.PRUEFUNG.ueberlauf(wf).fehlt
    const opfer = wf.querySelector('.liste')
    const boese = document.createElement('div')
    boese.style.cssText = 'height:400px;width:2000px'
    boese.textContent = 'kuenstlicher Ueberlauf'
    opfer.appendChild(boese)
    const nachher = window.PRUEFUNG.ueberlauf(wf)
    boese.remove()
    const zurueck = window.PRUEFUNG.ueberlauf(wf).fehlt
    return { vorher, nachher, zurueck }
  })
  const erkannt = r.nachher.fehlt > r.vorher + 50
  const heilt = r.zurueck <= r.vorher + 1
  sag(erkannt && heilt, erkannt
    ? (heilt ? `ok — kuenstlicher Ueberlauf gemeldet: ${r.nachher.fehlt} px in ${r.nachher.wo} (vorher ${r.vorher}, danach wieder ${r.zurueck})`
             : `Warnung bleibt haengen: nach dem Entfernen noch ${r.zurueck} px`)
    : `BLIND: kuenstlicher Ueberlauf nicht gemeldet (${r.vorher} → ${r.nachher.fehlt})`)
  await ctx.close()
}

/* -------------------------------------------------- gegenprobe verdeckung --
   Zwei kuenstliche Faelle, weil verdeckung() zwei Sorten kennt:
     RANDFARBE — eine Outline reicht in die Nachbarzeile (der echte Fall aus
                 zustaende.html: outline-offset 1 px auf 2 px Zeilenabstand).
     AUFLAGE   — ein absolut gesetztes Element liegt auf fremdem Text. */
async function probeGegenVerdeckung() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const r = await page.evaluate(() => {
    const wf = document.querySelector('.anordnung .wf')
    const vorher = window.PRUEFUNG.verdeckung(wf).tiefe
    // Fall 1: Randfarbe
    const zeile = wf.querySelector('.slotz')
    const altStil = zeile ? zeile.getAttribute('style') : null
    if (zeile) zeile.style.cssText = 'outline:2px solid #fff;outline-offset:8px'
    const rand = window.PRUEFUNG.verdeckung(wf)
    if (zeile) altStil === null ? zeile.removeAttribute('style') : zeile.setAttribute('style', altStil)
    // Fall 2: Auflage
    const ziel = wf.querySelector('.streifen')
    const deck = document.createElement('div')
    deck.style.cssText = 'position:absolute;left:10px;top:10px;width:120px;height:20px;background:#fff'
    deck.textContent = 'Auflage'
    wf.style.position = 'relative'
    wf.appendChild(deck)
    const auflage = window.PRUEFUNG.verdeckung(wf)
    deck.remove()
    const zurueck = window.PRUEFUNG.verdeckung(wf).tiefe
    return { vorher, rand, auflage, zurueck, hatZeile: !!zeile, hatZiel: !!ziel }
  })
  const randOk = r.rand.tiefe > r.vorher + 0.5 && r.rand.art === 'Randfarbe'
  const aufOk = r.auflage.tiefe > r.vorher + 0.5 && r.auflage.art === 'Auflage'
  const heilt = r.zurueck <= r.vorher + 0.5
  sag(randOk && aufOk && heilt,
    (randOk && aufOk && heilt)
      ? `ok — kuenstliche Verdeckung gemeldet: Randfarbe ${r.rand.tiefe} px (${r.rand.wo}), `
        + `Auflage ${r.auflage.tiefe} px (${r.auflage.wo}); Ruhelage wieder ${r.zurueck}`
      : `BLIND: Randfarbe=${r.rand.tiefe}/${r.rand.art} Auflage=${r.auflage.tiefe}/${r.auflage.art} `
        + `Ruhelage=${r.zurueck} (vorher ${r.vorher})`)
  await ctx.close()
}

/* ---------------------------------------------------------------- ratsche --
   Zweimal messen muss dasselbe ergeben. Fehlt listenZuruecksetzen(), misst
   der zweite Durchgang bereits Gekuerztes als natuerliche Hoehe. */
async function probeRatsche() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const eins = await page.evaluate(() => JSON.stringify(window.MESSUNG.anordnungen))
  await page.evaluate(() => window.zeichnen())
  await page.waitForTimeout(400)
  const zwei = await page.evaluate(() => JSON.stringify(window.MESSUNG.anordnungen))
  await page.evaluate(() => window.zeichnen())
  await page.waitForTimeout(400)
  const drei = await page.evaluate(() => JSON.stringify(window.MESSUNG.anordnungen))
  const a = JSON.parse(eins), b = JSON.parse(zwei), c = JSON.parse(drei)
  let ab = 0
  a.forEach((x, i) => {
    for (const k of ['kurve', 'bandkasten', 'ueberlauf']) {
      if (x[k] !== b[i][k] || x[k] !== c[i][k]) { ab++
        sag(false, `${x.id}.${k}: ${x[k]} → ${b[i][k]} → ${c[i][k]}`) }
    }
  })
  sag(ab === 0, `ratsche: drei Durchgaenge, ${ab} Abweichungen`)
  await ctx.close()
}

/* ----------------------------------------------------------------- deckel --*/
async function probeDeckel() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const r = await page.evaluate(() => {
    const schlimm = []
    document.querySelectorAll('.anordnung .wf').forEach(wf => {
      wf.querySelectorAll('.slotz, .sb, .sb1, .uv, .zeile').forEach(z => {
        const d = z.scrollHeight - z.clientHeight
        if (d > 0.5) schlimm.push({ kl: z.className, d })
      })
    })
    return schlimm
  })
  r.forEach(x => sag(false, `Deckel: ${x.kl} ${x.d} px`))
  sag(r.length === 0, `deckel: ${r.length} Deckel unter der Inhaltshoehe`)
  await ctx.close()
}

/* ------------------------------------------------------------------ beleg --*/
async function probeBeleg() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const r = await page.evaluate(() => {
    document.body.classList.add('vertragAn')
    const chips = [...document.querySelectorAll('.vertrag span span')]
    const main = ['Befundliste', 'Assistent', 'Heatmap', 'Übersichtskarte',
                  'Quellenliste', 'A/B-Urteil', 'Detail einer Quelle', 'Rollenkarte',
                  'Bedienebenen', 'Wegweiser', 'Zoomleiste', 'Dichtestufe',
                  'Einstiegszeile', 'Ein Satz']
    return {
      gesamt: chips.length,
      ohne: chips.filter(c => c.classList.contains('ohnebeleg')).map(c => c.textContent),
      mainReste: chips.map(c => c.textContent).filter(t => main.some(m => t.includes(m)))
    }
  })
  r.ohne.forEach(n => sag(false, `ohne Beleg: ${n}`))
  r.mainReste.forEach(n => sag(false, `Main-Baustein steht noch im Editor-Blatt: ${n}`))
  sag(r.ohne.length === 0 && r.mainReste.length === 0 && r.gesamt > 0,
    `beleg: ${r.gesamt} Bausteine, ${r.ohne.length} ohne Beleg, ${r.mainReste.length} Main-Reste`)
  await ctx.close()
}

/* -------------------------------------------------------------- grenzfall --
   Der Grenzfall ist die VORGABE, nicht der Sonderfall (docs/sondenplan.md §6).
   ZWEI Grenzfaelle, die in entgegengesetzte Richtungen zeigen — deshalb zwei
   Urteile. Bis 2026-08-21 stand hier ein Vergleich gegen drei Literale, die
   zwanzig Zeilen ueber ihrer Quelle standen; im Blatt selbst war das Tor ein
   hart geschriebenes `z(true, …)`. Beides konnte nicht scheitern. */
async function probeGrenzfall() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const m = await page.evaluate(() => ({
    welt: window.MESSUNG.welt,
    platz: window.MESSUNG.platzGrenzfall,
    ehr: window.MESSUNG.ehrlichkeit
  }))
  sag(m.platz.ok, m.platz.ok
    ? `grenzfall/platz = Vorgabe (${m.welt.slots} Slots, Dynamik an, `
      + 'Schutzbereich getroffen, Ehrlichkeitsschalter neutral)'
    : `grenzfall/platz NICHT Vorgabe: ${m.platz.fehlt.join(' · ')}`)
  sag(m.ehr.ok, m.ehr.ok
    ? `grenzfall/ehrlichkeit = sauber (${m.ehr.zellen} Zellen in Stellung `
      + '"Bypass + getrennt, Dynamik AN" tot, '
      + `${m.ehr.eqLebtImSchoenfall} EQ-Zellen leben in der Gegenstellung)`
    : `grenzfall/ehrlichkeit: ${m.ehr.reste.length} von ${m.ehr.zellen} Zellen zeigen `
      + `weiter Werte — ${[...new Set(m.ehr.reste)].slice(0, 6).join(', ')}`)
  await ctx.close()
}

/* --------------------------------------------------------------- zustaende --
   Im Grenzfall pruefen, nicht im Schoenfall (PRUEFLISTE 7.4). Alle sechzehn
   Kombinationen der Ehrlichkeitsschalter werden durchgeschaltet, und in jeder
   gilt: was nichts bewirkt, ist abgeschaltet und zeigt "—". Ein Knopf, der
   sich druecken laesst und nichts tut, ist eine Luege (PRUEFLISTE 1).

   Diese Probe ist nachtraeglich entstanden: beim Durchschalten von Hand fiel
   auf, dass die Draft-Griffe bei BYPASS aktiv blieben — vorhoerbar waere dort
   nichts, das Geraet ist aus der Kette. Von Hand gefunden heisst: beim
   naechsten Mal nicht gefunden. Deshalb hier. */
async function probeZustaende() {
  const { ctx, page, fehler } = await oeffne('sonde-messung.html')
  const faelle = []
  for (const bypass of [false, true])
    for (const verbunden of [true, false])
      for (const dynamik of [true, false])
        for (const slots of [8, 1])
          faelle.push({ bypass, verbunden, dynamik, slots })

  let schief = 0, geprueft = 0
  for (const f of faelle) {
    const alle = await page.evaluate(async w => {
      Object.assign(window.WELT, w)
      window.zeichnen()
      await new Promise(a => requestAnimationFrame(() => requestAnimationFrame(a)))
      // Einheiten stehen ausschliesslich an WERTEN. Die Slot-Nummer im
      // Streifenkopf ("● 1 HP") ist Identitaet und darf bleiben — deshalb ist
      // das Kriterium Hz/kHz/dB/ms und nicht "enthaelt eine Ziffer".
      const EINHEIT = /\b(Hz|kHz|dB|ms)\b/
      return [...document.querySelectorAll('.anordnung')].map(an => {
        const wf = an.querySelector('.wf')
        const draft = wf.querySelector('[data-baustein^="Draft vom Main"]')
        const roh = k => [...wf.querySelectorAll('.slotz .' + k)].map(c => c.textContent.trim())
        const zellen = k => roh(k).filter(t => t !== '—')
        // dy gehoert zu den DYNAMIK-Zellen: es ist der Schalter, an dem die
        // fuenf anderen haengen. Steht er auf "dyn", waehrend seine fuenf
        // Abhaengigen "—" zeigen, luegt die Zeile ueber sich selbst.
        const dyn = ['dy', 'thr', 'rng', 'atk', 'hld', 'rel'].flatMap(zellen)
        const eq  = ['typ', 'f', 'q', 'g', 'ch'].flatMap(zellen)
        const streifen = [...wf.querySelectorAll('.sb, .sb1')]
          .map(s => s.textContent.replace(/\s+/g, ' ').trim())
          .filter(t => EINHEIT.test(t))
        return {
          id: an.dataset.id,
          lebendeDraftGriffe: draft
            ? [...draft.querySelectorAll('[data-griff]')].map(g => g.textContent.trim()) : [],
          dyn, eq, streifen,
          /* Wieviele Zellen es UEBERHAUPT gibt, tot oder lebendig. Ohne diese
             Zahl verlangt die Gegenprobe nach unten Leben an Stellen, die es
             formbedingt nicht gibt: B und C zeigen nur die sechs EQ-Werte und
             haben gar keine Dynamikzelle. Gemessen, nicht angenommen. */
          dynZellen: ['dy', 'thr', 'rng', 'atk', 'hld', 'rel'].flatMap(roh).length,
          eqZellen:  ['typ', 'f', 'q', 'g', 'ch'].flatMap(roh).length,
          // Eine Anordnung ohne Bandkasten (Fall E) zeigt bewusst keinen Slot.
          hatBandliste: !!wf.querySelector('.bandliste'),
          // Ein Slot wird in DREI Formen dargestellt — Zeile, dreizeiliger und
          // einzeiliger Streifen. Nur ihre Summe sagt, ob acht Slots da sind.
          darstellungen: wf.querySelectorAll('.slotz').length
                       + wf.querySelectorAll('.sb').length
                       + wf.querySelectorAll('.sb1').length
        }
      })
    }, f)
    for (const r of alle) {
      geprueft++
      const wo = `${r.id} bypass=${f.bypass} verbunden=${f.verbunden} `
               + `dyn=${f.dynamik} slots=${f.slots}`
      const kurz = l => l.slice(0, 3).map(t => `"${t}"`).join(', ')
      // 1. Ohne Main oder bei Bypass darf kein Draft-Griff mehr leben.
      if ((f.bypass || !f.verbunden) && r.lebendeDraftGriffe.length) { schief++
        sag(false, `${wo}: Draft-Griffe noch aktiv (${r.lebendeDraftGriffe.join(', ')})`) }
      // 2. Bei Bypass ist JEDER Bandparameter tot — auch der dyn-Schalter und
      //    auch die Werte in den Kontextstreifen. Genau hier war die alte
      //    Fassung blind: sie las nur Fall A und darin nur .slotz.
      if (f.bypass && (r.eq.length || r.dyn.length || r.streifen.length)) { schief++
        sag(false, `${wo}: ${r.eq.length} EQ-, ${r.dyn.length} Dynamikwerte und `
          + `${r.streifen.length} Streifen stehen noch da — `
          + `${kurz([...r.eq, ...r.dyn, ...r.streifen])}`) }
      // 3. Ohne Dynamik sind dyn-Schalter und die fuenf Dynamikwerte tot.
      if (!f.dynamik && r.dyn.length) { schief++
        sag(false, `${wo}: ${r.dyn.length} Dynamikwerte trotz abgeschalteter `
          + `Dynamik — ${kurz(r.dyn)}`) }
      // 4. Gegenprobe nach unten: im Schoenfall MUSS etwas leben, sonst prueft
      //    diese Schleife nur, dass alles leer ist. Fall E hat bewusst keinen
      //    Bandkasten und ist davon ausgenommen.
      if (!f.bypass && f.dynamik && r.dynZellen > 0 && !r.dyn.length) { schief++
        sag(false, `${wo}: keine der ${r.dynZellen} Dynamikzellen lebt im Schoenfall `
          + '— die Probe prueft ins Leere') }
      if (!f.bypass && r.eqZellen > 0 && !r.eq.length) { schief++
        sag(false, `${wo}: keine der ${r.eqZellen} EQ-Zellen lebt im Schoenfall `
          + '— die Probe prueft ins Leere') }
      // 5. So viele Slot-Darstellungen wie Slots, egal in welcher Form.
      const erwartet = r.hatBandliste ? f.slots : 0
      if (r.darstellungen !== erwartet) { schief++
        sag(false, `${wo}: ${r.darstellungen} Slot-Darstellungen statt ${erwartet}`) }
    }
  }
  if (fehler.length) { schief++; sag(false, 'JS-Fehler: ' + fehler[0]) }
  sag(schief === 0, `zustaende: ${faelle.length} Grenzfaelle x ${geprueft / faelle.length} `
    + `Anordnungen = ${geprueft} Prueflinge, alle Zellformen — ${schief} Abweichungen`)
  await ctx.close()
}

/* -------------------------------------------------------------- maschine --
   Codex-Befund 6: Draft, Discard, Undo und Neutralise waren keine ehrliche
   Zustandsmaschine. Ein echter Klick auf Discard liess die Vorschlagszeile
   Wort fuer Wort stehen, Neutralise hatte gar keinen Handler, der Verlauf war
   eine feste Liste, und Undo im Ruhezustand tat nichts (Nachtrag E3).

   Die tragende Regel dieser Probe ist allgemein und nicht auf die vier Griffe
   zugeschnitten: JEDER lebende Griff muss etwas Sichtbares aendern. Ein Griff,
   der sich druecken laesst und nichts tut, ist eine Luege in der Oberflaeche
   (PRUEFLISTE 1). Geprueft wird gegen den gerenderten Text, nicht gegen LAGE —
   sonst bestaetigt sich der Zustand selbst. */
async function probeMaschine() {
  const { ctx, page, fehler } = await oeffne('sonde-messung.html')
  const bild = () => page.evaluate(() => {
    const wf = document.querySelector('.anordnung[data-id="A"] .wf')
    const t = s => { const e = wf.querySelector(s)
      return e ? e.innerText.replace(/\s+/g, ' ').trim() : null }
    return {
      draft: t('[data-baustein^="Draft vom Main"]'),
      history: t('[data-baustein^="Undo-Verlauf"]'),
      reiter: [...wf.querySelectorAll('.eb')].filter(e => e.classList.contains('an'))
        .map(e => e.textContent.trim()),
      gains: [...wf.querySelectorAll('.slotz .g')].map(e => e.textContent.trim()),
      lebende: [...wf.querySelectorAll('[data-griff]')].map(g => g.dataset.griff),
      ganzesBlatt: wf.innerText.replace(/\s+/g, ' ').trim()
    }
  })
  const klick = w => page.evaluate(w => {
    const wf = document.querySelector('.anordnung[data-id="A"] .wf')
    const g = [...wf.querySelectorAll('[data-griff]')].find(e => e.dataset.griff === w)
    if (!g) return false
    g.click(); return true
  }, w)
  const chrome = z => page.evaluate(z => {
    // Der Knopf steht in der Chrome-Gruppe "Vorschlag" im Fuss.
    const alle = [...document.querySelectorAll('#fuss button')]
      .filter(b => b.textContent.trim() === z)
    if (!alle.length) return false
    alle[0].click(); return true
  }, z)
  let schief = 0
  const pruefe = (ok, t) => { if (!ok) { schief++; sag(false, t) } }
  const nach = async (was, fn) => { await fn(); await page.waitForTimeout(350)
                                    return { was, bild: await bild() } }

  const start = await bild()
  pruefe(start.reiter.length === 1,
    `Start: ${start.reiter.length} aktive Reiter statt genau einem`)
  pruefe(/from Main/.test(start.draft), `Start: Vorschlagszeile ist "${start.draft}"`)

  // 1. Discard MUSS die Zeile aendern und alle drei Griffe toeten.
  const nachDiscard = await nach('discard', () => klick('discard'))
  pruefe(nachDiscard.bild.draft !== start.draft,
    `Discard aendert die Vorschlagszeile nicht: "${nachDiscard.bild.draft}"`)
  pruefe(/discarded/.test(nachDiscard.bild.draft || ''),
    `Discard sagt nicht, dass verworfen wurde: "${nachDiscard.bild.draft}"`)
  pruefe(!nachDiscard.bild.lebende.some(g => ['hold','apply','discard'].includes(g)),
    `nach Discard leben noch Vorschlagsgriffe: ${nachDiscard.bild.lebende.join(', ')}`)

  // 2. Gegenpfad: zurueck nach "offen" (Werkzeug-Chrome).
  const zurueck = await nach('chrome:offen', () => chrome('offen'))
  pruefe(zurueck.bild.draft === start.draft,
    `Rueckweg fuehrt nicht in den Ausgangszustand: "${zurueck.bild.draft}"`)

  // 3. Apply: Zeile aendert sich, Verlauf WAECHST, ein Reiter bleibt aktiv.
  const nachApply = await nach('apply', () => klick('apply'))
  pruefe(/applied/.test(nachApply.bild.draft || ''),
    `Apply aendert die Zeile nicht: "${nachApply.bild.draft}"`)
  pruefe(nachApply.bild.history !== zurueck.bild.history,
    `Apply laesst den Verlauf unveraendert: "${nachApply.bild.history}"`)
  pruefe(/6 confirmed steps/.test(nachApply.bild.history || ''),
    `Verlauf zaehlt nach Apply nicht mit: "${nachApply.bild.history}"`)
  pruefe(nachApply.bild.reiter.length === 1,
    `nach Apply ${nachApply.bild.reiter.length} aktive Reiter — `
    + 'ein Zustand ohne Reiter ist genau Codex-Befund 10')

  // 4. Undo nimmt es zurueck — Zeile UND Verlauf.
  const nachUndo = await nach('undo', () => klick('undo'))
  pruefe(nachUndo.bild.draft === start.draft,
    `Undo stellt die Vorschlagszeile nicht her: "${nachUndo.bild.draft}"`)
  pruefe(nachUndo.bild.history === zurueck.bild.history,
    `Undo stellt den Verlauf nicht her: "${nachUndo.bild.history}"`)

  // 5. Neutralise: alle Gains auf 0 — und kein "−0.0 dB" (Codex-Befund 10).
  const nachNeutral = await nach('neutralise', () => klick('neutralise'))
  const gains = nachNeutral.bild.gains.filter(g => g !== '—')
  pruefe(gains.length > 0, 'nach Neutralise gibt es keinen einzigen Gain-Wert')
  pruefe(gains.every(g => g === '0.0 dB'),
    `Neutralise stellt nicht alle Gains auf null: ${[...new Set(gains)].join(', ')}`)
  pruefe(!nachNeutral.bild.ganzesBlatt.includes('−0.0'),
    'im Blatt steht „−0.0" — Null hat kein Vorzeichen')
  pruefe(!nachNeutral.bild.lebende.includes('neutralise'),
    'Neutralise bleibt nach dem Neutralisieren lebendig, tut aber nichts mehr')

  // 6. Gegenpfad zu Neutralise.
  const nachUndo2 = await nach('undo', () => klick('undo'))
  pruefe(nachUndo2.bild.gains.join() === start.gains.join(),
    `Undo stellt die Gains nicht her: ${nachUndo2.bild.gains.join(' ')}`)

  // 7. Verlauf leerklicken: dann MUSS Undo tot sein (Nachtrag E3).
  for (let i = 0; i < 12; i++) {
    const b = await bild()
    if (!b.lebende.includes('undo')) break
    await klick('undo'); await page.waitForTimeout(120)
  }
  const leer = await bild()
  pruefe(!leer.lebende.includes('undo'),
    'Undo lebt noch, obwohl der Verlauf leer ist — ein Knopf ohne Wirkung')
  pruefe(/0 confirmed steps/.test(leer.history || ''),
    `Verlauf meldet nicht null: "${leer.history}"`)

  // 8. Echtes Halten mit pointerdown/pointerup — der einzige Griff, den ein
  //    Klick nicht erreicht. Er ist die Mechanik, die die Spezifikation
  //    woertlich verlangt: gehalten = hoerbar, losgelassen = weg.
  await chrome('offen'); await page.waitForTimeout(300)
  const vorHalten = await bild()
  await page.evaluate(() => {
    const g = document.querySelector('.anordnung[data-id="A"] [data-griff="hold"]')
    g.dispatchEvent(new PointerEvent('pointerdown', {bubbles:true, cancelable:true}))
  })
  await page.waitForTimeout(300)
  const gehalten = await bild()
  await page.evaluate(() => {
    const g = document.querySelector('.anordnung[data-id="A"] [data-griff="hold"]')
    g.dispatchEvent(new PointerEvent('pointerup', {bubbles:true}))
  })
  await page.waitForTimeout(300)
  const losgelassen = await bild()
  pruefe(gehalten.reiter.join() === 'Draft',
    `gehalten: aktiver Reiter ist "${gehalten.reiter.join()}" statt "Draft"`)
  pruefe(losgelassen.reiter.join() === vorHalten.reiter.join(),
    `losgelassen: Reiter kehrt nicht zurueck (${losgelassen.reiter.join()})`)

  if (fehler.length) { schief++; sag(false, 'maschine JS-Fehler: ' + fehler[0]) }
  sag(schief === 0, 'maschine: offen→verworfen→offen→angewandt→offen, neutralisieren '
    + 'und zuruecknehmen, Verlauf leergeklickt, Halten per pointerdown/-up — '
    + `${schief} Abweichungen`)
  await ctx.close()
}

/* --------------------------------------------------------------- annahmen --
   Codex-Befund 5: der "laengstmoegliche SLOT" berief sich im Kommentar auf die
   Spezifikation. Die nennt fuer die Bandparameter aber genau eine Zahl —
   "Gain (manuell bis ±12 dB)". Q 24, −60 dB, 300/500/3000 ms und die
   Lesbarkeitsschwelle 15,4 px sind frei gesetzt.
   Diese Probe haelt fest, dass jede frei gesetzte Zahl im Blatt SICHTBAR als
   Annahme benannt ist. Wer eine neue erfindet, muss sie benennen — sonst
   faellt hier auf, was sonst still einzieht. */
async function probeAnnahmen() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const r = await page.evaluate(async () => {
    const s = document.querySelector('.annahme')
    if (!s) return { da: false }
    /* Sichtbar heisst gesehen, nicht vorhanden: derselbe Fehler wie beim
       Standband (Codex-Befund 8). ABER: elementFromPoint ist FENSTERrelativ.
       Der Fuss dieses Blattes steht bei y≈2460 im Dokument, also unter dem
       Falz — ohne Scrollen traf der Test eine beliebige Bandzeile und meldete
       "nicht zu sehen". Beim ersten Lauf gemessen. Erst hinsehen, dann
       urteilen; "unter dem Falz" ist kein Verdeckungsbefund. */
    s.scrollIntoView({ block: 'center' })
    await new Promise(a => requestAnimationFrame(() => requestAnimationFrame(a)))
    const rc = s.getBoundingClientRect()
    const mitte = document.elementFromPoint(rc.left + rc.width / 2,
                                            rc.top + rc.height / 2)
    return {
      da: true, text: s.textContent.replace(/\s+/g, ' ').trim(),
      erwartet: (s.dataset.annahmen || '').split(',').filter(Boolean),
      flaeche: rc.width > 40 && rc.height > 8,
      sichtbar: !!mitte && (mitte === s || s.contains(mitte)),
      // Die Zahlen, die tatsaechlich im Blatt gesetzt sind
      werte: window.LAENGSTER ? Object.keys(window.LAENGSTER) : []
    }
  })
  let schief = 0
  const pruefe = (ok, t) => { if (!ok) { schief++; sag(false, t) } }
  pruefe(r.da, 'annahmen: kein sichtbarer Testannahme-Hinweis im Blatt')
  if (r.da) {
    pruefe(r.flaeche && r.sichtbar,
      `annahmen: Hinweis steht im DOM, ist aber nicht zu sehen `
      + `(Flaeche ${r.flaeche}, sichtbar ${r.sichtbar})`)
    // Jede angemeldete Annahme muss auch im Text stehen.
    const worte = { qN: 'Q 24', thr: '−60 dB', rng: '−18 dB', atk: '300 ms',
                    hld: '500 ms', rel: '3000 ms', lesbar: '15.4 px' }
    for (const k of r.erwartet)
      pruefe(worte[k] && r.text.includes(worte[k]),
        `annahmen: "${k}" ist angemeldet, steht aber nicht im sichtbaren Text`)
    pruefe(r.text.includes('±12 dB'),
      'annahmen: der Hinweis nennt nicht, welche Zahl AUS der Spezifikation kommt')
    pruefe(r.erwartet.length >= 7,
      `annahmen: nur ${r.erwartet.length} angemeldet, erwartet sind die sechs `
      + 'Parametergrenzen plus die Lesbarkeitsschwelle')
  }
  /* Gegenprobe: kann dieser Riegel scheitern? Der Hinweis wird kurz versteckt
     — mit visibility, nicht display, damit er seine Flaeche behaelt und
     wirklich die SICHTBARKEIT geprueft wird und nicht das Vorhandensein. */
  const gegen = await page.evaluate(async () => {
    const s = document.querySelector('.annahme')
    s.style.visibility = 'hidden'
    s.scrollIntoView({ block: 'center' })
    await new Promise(a => requestAnimationFrame(() => requestAnimationFrame(a)))
    const rc = s.getBoundingClientRect()
    const m = document.elementFromPoint(rc.left + rc.width / 2, rc.top + rc.height / 2)
    const erkannt = !(m === s || s.contains(m))
    s.style.visibility = ''
    await new Promise(a => requestAnimationFrame(() => requestAnimationFrame(a)))
    const rc2 = s.getBoundingClientRect()
    const m2 = document.elementFromPoint(rc2.left + rc2.width / 2, rc2.top + rc2.height / 2)
    return { erkannt, geheilt: m2 === s || s.contains(m2) }
  })
  pruefe(gegen.erkannt, 'BLIND: ein versteckter Annahme-Hinweis wird nicht gemeldet')
  pruefe(gegen.geheilt, 'Gegenprobe heilt nicht: der Hinweis bleibt unsichtbar')
  sag(schief === 0, `annahmen: ${r.erwartet?.length} frei gesetzte Zahlen sichtbar `
    + `benannt, Sachquelle genannt, Gegenprobe `
    + `${gegen.erkannt ? 'erkannt' : 'BLIND'} und `
    + `${gegen.geheilt ? 'geheilt' : 'NICHT geheilt'} — ${schief} Abweichungen`)
  await ctx.close()
}

/* ------------------------------------------------------------- formfaktor --*/
async function probeFormfaktor() {
  const { ctx, page, fehler } = await oeffne('formfaktor.html')
  const r = await page.evaluate(() => {
    const kn = [...document.querySelectorAll('button')]
      .find(b => /Active-Probe/.test(b.textContent))
    if (kn) kn.click()
    return new Promise(aufl => setTimeout(() => {
      const f = document.querySelector('#aktiv')
      if (!f) return aufl({ da: false })
      aufl({
        da: true,
        slots: f.querySelectorAll('.ap-slot').length,
        werte: (f.innerText.match(/dB|Hz|ms/g) || []).length,
        fehlt: +f.dataset.fehlt,
        titel: f.querySelector('.titel em')?.textContent || ''
      })
    }, 500))
  })
  if (fehler.length) sag(false, 'formfaktor JS-Fehler: ' + fehler[0])
  const modelliert = r.da && r.slots >= 8 && r.werte >= 30
  sag(modelliert && !fehler.length,
    modelliert ? `aktiv=modelliert (${r.slots} Slot-Zeilen, ${r.werte} Werte, `
                 + `Ueberlauf ${r.fehlt} px — "${r.titel}")`
               : `aktiv NICHT modelliert: ${JSON.stringify(r)}`)
  await ctx.close()
}

/* ----------------------------------------------------------------- export --
   Der Ergebnisexport ist das EINZIGE, was dieses Blatt ueberlebt. Bis
   2026-08-21 schrieb er fuer Active und Passive nur Name, Breite, Hoehe —
   also genau die drei Zahlen, die schon vorher im Quelltext standen, und
   keinen einzigen Messwert. Ein gespeichertes Ergebnis las sich positiv,
   waehrend die Kachel 23 px nach unten ueberlief.
   Diese Probe vergleicht das exportierte JSON gegen den LEBENDEN Zustand —
   nicht gegen sich selbst. */
async function probeExport() {
  const { ctx, page, fehler } = await oeffne('formfaktor.html')
  const r = await page.evaluate(async () => {
    const an = t => { const b = [...document.querySelectorAll('button')]
      .find(b => new RegExp(t).test(b.textContent)); if (b) b.click() }
    an('Active-Probe'); an('Passive-Kachel')
    await new Promise(a => setTimeout(a, 700))
    document.querySelector('#fertig').click()
    await new Promise(a => setTimeout(a, 200))
    // Unabhaengiger Weg zur Wahrheit: direkt am DOM nachrechnen, nicht die
    // dataset-Felder lesen, aus denen der Export selbst gebaut wurde.
    const live = id => {
      const f = document.getElementById(id); if (!f) return null
      const leib = f.querySelector('.leib')
      let hoch = 0, breit = 0
      const pruef = x => { hoch = Math.max(hoch, x.scrollHeight - x.clientHeight)
                           breit = Math.max(breit, x.scrollWidth - x.clientWidth) }
      pruef(leib); leib.querySelectorAll('*').forEach(pruef)
      const kv = f.querySelector('.kurve')
      return { hoch, breit, kurve: kv ? kv.clientHeight : null }
    }
    return { json: JSON.parse(document.querySelector('#ausgabe').value),
             aktiv: live('aktiv'), passiv: live('passiv0') }
  })
  let schief = 0
  const j = r.json
  const pruefe = (ok, text) => { if (!ok) { schief++; sag(false, text) } }
  pruefe(j.version >= 2, `Exportversion ${j.version} — Messwerte kamen erst mit 2`)
  for (const [name, e, l] of [['activeProbe', j.activeProbe, r.aktiv],
                              ['passiveKachel', j.passiveKachel, r.passiv]]) {
    if (!e) { pruefe(false, `${name} fehlt im Export`); continue }
    pruefe(e.ueberlauf && typeof e.ueberlauf.px === 'number',
      `${name}: kein Ueberlauf im Export`)
    pruefe(e.ueberlauf.hoch === l.hoch && e.ueberlauf.breit === l.breit,
      `${name}: Export sagt ${e.ueberlauf.hoch}/${e.ueberlauf.breit} px `
      + `(hoch/breit), lebendig gemessen ${l.hoch}/${l.breit}`)
    // Die Richtung ist der Befund, nicht die Zahl: "23 px" ohne "hoch" ist
    // nicht verwertbar (Codex-Befund 9).
    const erwartet = l.hoch > 1 && l.breit > 1 ? 'hoch und breit'
                   : l.hoch > 1 ? 'hoch' : l.breit > 1 ? 'breit' : 'keine'
    pruefe(e.ueberlauf.richtung === erwartet,
      `${name}: Richtung "${e.ueberlauf.richtung}" statt "${erwartet}"`)
    pruefe(e.kurvenraumPx === l.kurve,
      `${name}: Kurvenraum ${e.kurvenraumPx} im Export, ${l.kurve} lebendig`)
  }
  // Gegenprobe nach unten: der Export MUSS mindestens einen echten Befund
  // tragen, sonst prueft diese Probe leere Felder.
  const befundGetragen = j.passiveKachel && j.passiveKachel.ueberlauf.px > 1
  pruefe(befundGetragen, 'der Export traegt keinen einzigen Ueberlauf — '
    + 'diese Probe prueft ins Leere')
  if (fehler.length) { schief++; sag(false, 'export JS-Fehler: ' + fehler[0]) }
  sag(schief === 0, `export: v${j.version}, Active `
    + `${j.activeProbe?.ueberlauf.px} px / Kurve ${j.activeProbe?.kurvenraumPx} px `
    + `(${j.activeProbe?.urteil}), Kachel ${j.passiveKachel?.ueberlauf.px} px `
    + `${j.passiveKachel?.ueberlauf.richtung} (${j.passiveKachel?.urteil}) `
    + `— ${schief} Abweichungen`)
  await ctx.close()
}

/* Die Kachelprobe pruefte bis 2026-08-21 nur, ob "Backing Vox" und "POST" im
   Text vorkommen — also ob der Grenzfall GEMEINT war, nicht was er ergibt.
   Der Code behauptete daneben, bei 260x84 entscheide die Breite. Gemessen ist
   der Ueberlauf rein vertikal. Eine Probe, die den Text prueft und die Zahl
   danebenstehen laesst, haelt so einen Widerspruch nicht auf. */
async function probeKachel() {
  const { ctx, page } = await oeffne('formfaktor.html')
  const r = await page.evaluate(() => {
    const kn = [...document.querySelectorAll('button')]
      .find(b => /Passive-Kachel/.test(b.textContent))
    if (kn) kn.click()
    return new Promise(aufl => setTimeout(() => {
      const f = document.querySelector('#passiv0')
      if (!f) return aufl({ da: false })
      // Unabhaengiger Weg: selbst nachrechnen statt dataset lesen.
      const leib = f.querySelector('.leib')
      let hoch = 0, breit = 0, woH = ''
      const pruef = x => {
        const h = x.scrollHeight - x.clientHeight, b = x.scrollWidth - x.clientWidth
        if (h > hoch) { hoch = h; woH = x.className || x.tagName }
        if (b > breit) breit = b
      }
      pruef(leib); leib.querySelectorAll('*').forEach(pruef)
      aufl({ da: true, text: f.innerText.replace(/\s+/g, ' ').trim(),
             fehlt: +f.dataset.fehlt, hoch, breit, woH,
             gemeldetHoch: +f.dataset.hoch, gemeldetBreit: +f.dataset.breit,
             richtung: f.dataset.richtung })
    }, 500))
  })
  let schief = 0
  const pruefe = (ok, t) => { if (!ok) { schief++; sag(false, t) } }
  pruefe(r.da, 'kachel: #passiv0 fehlt')
  if (r.da) {
    pruefe(/Backing Vox/.test(r.text) && /POST/.test(r.text),
      `kachel NICHT im Grenzfall: "${r.text}"`)
    pruefe(r.gemeldetHoch === r.hoch && r.gemeldetBreit === r.breit,
      `kachel: Blatt meldet ${r.gemeldetHoch}/${r.gemeldetBreit} px (hoch/breit), `
      + `unabhaengig gemessen ${r.hoch}/${r.breit}`)
    // Der eigentliche Befund: die Richtung. Ein Ueberlauf ohne Richtung sagt
    // nicht, ob eine Zeile weniger oder ein kuerzerer Name hilft.
    pruefe(r.hoch > 1, `kachel: kein vertikaler Ueberlauf mehr (${r.hoch} px) — `
      + 'entweder ist der Grenzfall entschaerft oder die Messung ist blind')
    pruefe(r.breit <= 1, `kachel: ${r.breit} px Ueberlauf in der BREITE — `
      + 'das waere neu, der Engpass war bisher rein vertikal')
    pruefe(r.richtung === 'hoch',
      `kachel: Richtung "${r.richtung}" statt "hoch"`)
  }
  sag(schief === 0, `kachel=grenzfall, Ueberlauf ${r.hoch} px hoch / ${r.breit} px `
    + `breit (${r.woH}) — der Engpass ist die HOEHE, nicht die Breite; `
    + `${schief} Abweichungen`)
  await ctx.close()
}

/* ------------------------------------------------------------------ Ablauf */
const PROBEN = {
  zahlen: probeZahlen, waage: probeWaage, anordnungen: probeAnordnungen,
  'gegenprobe-ueberlauf': probeGegenUeberlauf,
  'gegenprobe-verdeckung': probeGegenVerdeckung,
  ratsche: probeRatsche, deckel: probeDeckel, beleg: probeBeleg,
  grenzfall: probeGrenzfall, zustaende: probeZustaende,
  formfaktor: probeFormfaktor, kachel: probeKachel, export: probeExport,
  annahmen: probeAnnahmen, maschine: probeMaschine
}
const was = process.argv[2] || 'alles'
const lauf = was === 'alles' ? Object.keys(PROBEN) : [was]
for (const n of lauf) {
  if (!PROBEN[n]) { console.error('Unbekannte Probe: ' + n + '\nBekannt: '
    + Object.keys(PROBEN).join(', ') + ', alles'); await browser.close(); process.exit(2) }
  await PROBEN[n]()
}
await browser.close()
if (befunde.length) { console.log(`\nBEFUNDE: ${befunde.length}`); process.exit(1) }
console.log(`\nALLES SAUBER (${lauf.length} Probe${lauf.length > 1 ? 'n' : ''})`)
