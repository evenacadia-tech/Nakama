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
//                                        | formfaktor | kachel
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
      // offsetHeight, NICHT getBoundingClientRect: die Kacheln stehen unter
      // transform:scale(). Die erste Fassung dieser Sonde verglich zwei Werte
      // mit demselben Fehler und meldete zufrieden "0 Abweichungen".
      const rect = s => { const e = a.querySelector(s); return e ? e.offsetHeight : 0 }
      return { id: a.dataset.id,
               gezeigtKurve: zahl('kurve'), echtKurve: rect('.kurve'),
               gezeigtBand: zahl('bandkasten'), echtBand: rect('.bandliste') }
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
  await page.evaluate(() => { window.WELT.slots = 8; window.zeichnen() })
  await page.waitForTimeout(400)
  const beiKlein = await lies()
  await page.evaluate(() => {
    document.querySelectorAll('.rahmen').forEach(r => r.style.setProperty('--s', '1'))
    window.zeichnen()
  })
  await page.waitForTimeout(400)
  const beiGross = await lies()
  const wandert = beiKlein.filter(v => {
    const g = beiGross.find(x => x.id === v.id)
    return Math.abs(v.gezeigtKurve - g.gezeigtKurve) > 1
        || Math.abs(v.gezeigtBand - g.gezeigtBand) > 1
  })
  wandert.forEach(v => { schief++
    const g = beiGross.find(x => x.id === v.id)
    sag(false, `${v.id}: Zahl haengt am Massstab — verkleinert Kurve ${v.gezeigtKurve}/Band `
      + `${v.gezeigtBand}, 1:1 Kurve ${g.gezeigtKurve}/Band ${g.gezeigtBand}`) })
  if (fehler.length) { schief++; sag(false, 'JS-Fehler: ' + fehler[0]) }
  sag(schief === 0, `zahlen: ${vorher.length} Anordnungen gegengerechnet, `
    + `${mitBand.length} auf Aenderung geprueft, ${beiKlein.length} auf `
    + `Massstabsunabhaengigkeit geprueft — ${schief} Abweichungen`)
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
   Der Grenzfall ist die VORGABE, nicht der Sonderfall (docs/sondenplan.md §6). */
async function probeGrenzfall() {
  const { ctx, page } = await oeffne('sonde-messung.html')
  const w = await page.evaluate(() => window.MESSUNG.welt)
  const ok = w.slots === 8 && w.dynamik === true && w.schutzGetroffen === true
  sag(ok, ok ? `grenzfall=vorgabe (${w.slots} Slots, Dynamik an, Schutzbereich getroffen)`
            : `grenzfall NICHT Vorgabe: ${JSON.stringify(w)}`)
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

  let schief = 0
  for (const f of faelle) {
    const r = await page.evaluate(async w => {
      Object.assign(window.WELT, w)
      window.zeichnen()
      await new Promise(a => requestAnimationFrame(() => requestAnimationFrame(a)))
      const wf = document.querySelector('.anordnung[data-id="A"] .wf')
      const draft = wf.querySelector('[data-baustein^="Draft vom Main"]')
      const lebendeDraftGriffe = draft
        ? [...draft.querySelectorAll('[data-griff]')].map(g => g.textContent.trim()) : []
      const zellen = k => [...wf.querySelectorAll('.slotz .' + k)]
        .map(c => c.textContent.trim())
      const dynFelder = ['thr', 'rng', 'atk', 'hld', 'rel'].flatMap(zellen)
      const eqFelder = ['typ', 'f', 'q'].flatMap(zellen)
      return {
        lebendeDraftGriffe,
        dynLebendig: dynFelder.filter(t => t !== '—').length,
        eqLebendig: eqFelder.filter(t => t !== '—').length,
        zeilen: wf.querySelectorAll('.slotz').length
      }
    }, f)
    const wo = `bypass=${f.bypass} verbunden=${f.verbunden} dyn=${f.dynamik} slots=${f.slots}`
    // 1. Ohne Main oder bei Bypass darf kein Draft-Griff mehr leben.
    if ((f.bypass || !f.verbunden) && r.lebendeDraftGriffe.length) { schief++
      sag(false, `${wo}: Draft-Griffe noch aktiv (${r.lebendeDraftGriffe.join(', ')})`) }
    // 2. Bei Bypass ist JEDER Bandparameter tot.
    if (f.bypass && (r.eqLebendig || r.dynLebendig)) { schief++
      sag(false, `${wo}: ${r.eqLebendig} EQ- und ${r.dynLebendig} Dynamikwerte stehen noch da`) }
    // 3. Ohne Dynamik sind die fuenf Dynamikwerte je Slot tot.
    if (!f.dynamik && r.dynLebendig) { schief++
      sag(false, `${wo}: ${r.dynLebendig} Dynamikwerte trotz abgeschalteter Dynamik`) }
    // 4. Gegenprobe nach unten: im Schoenfall MUSS etwas leben, sonst prueft
    //    diese Schleife nur, dass alles leer ist.
    if (!f.bypass && f.dynamik && !r.dynLebendig) { schief++
      sag(false, `${wo}: keine Dynamikwerte im Schoenfall — die Probe prueft ins Leere`) }
    if (r.zeilen !== f.slots) { schief++
      sag(false, `${wo}: ${r.zeilen} Bandzeilen statt ${f.slots}`) }
  }
  if (fehler.length) { schief++; sag(false, 'JS-Fehler: ' + fehler[0]) }
  sag(schief === 0, `zustaende: ${faelle.length} Grenzfaelle durchgeschaltet — `
    + `${schief} Abweichungen`)
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

async function probeKachel() {
  const { ctx, page } = await oeffne('formfaktor.html')
  const r = await page.evaluate(() => {
    const kn = [...document.querySelectorAll('button')]
      .find(b => /Passive-Kachel/.test(b.textContent))
    if (kn) kn.click()
    return new Promise(aufl => setTimeout(() => {
      const f = document.querySelector('#passiv0')
      aufl(f ? { da: true, text: f.innerText.replace(/\s+/g, ' ').trim(),
                 fehlt: +f.dataset.fehlt } : { da: false })
    }, 500))
  })
  const grenz = r.da && /Backing Vox/.test(r.text) && /POST/.test(r.text)
  sag(grenz, grenz ? `kachel=grenzfall ("${r.text}", Ueberlauf ${r.fehlt} px)`
                   : `kachel NICHT im Grenzfall: ${JSON.stringify(r)}`)
  await ctx.close()
}

/* ------------------------------------------------------------------ Ablauf */
const PROBEN = {
  zahlen: probeZahlen, waage: probeWaage, anordnungen: probeAnordnungen,
  'gegenprobe-ueberlauf': probeGegenUeberlauf,
  'gegenprobe-verdeckung': probeGegenVerdeckung,
  ratsche: probeRatsche, deckel: probeDeckel, beleg: probeBeleg,
  grenzfall: probeGrenzfall, zustaende: probeZustaende,
  formfaktor: probeFormfaktor, kachel: probeKachel
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
