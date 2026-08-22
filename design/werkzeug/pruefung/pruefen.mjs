// pruefen.mjs — die Blattpruefung dieses Projekts, dauerhaft.
//
// werkzeug/PRUEFLISTE.md hielt fest: "Die Messungen wurden am 2026-08-20 mit
// Playwright-Skripten ausgefuehrt; die Skripte waren sitzungslokal und sind
// weg, die Regeln nicht." Das war ein benannter Verlust — hier steht er nicht
// noch einmal an.
//
// AUFRUF
//   node werkzeug/pruefung/pruefen.mjs            alle Blaetter
//   node werkzeug/pruefung/pruefen.mjs zustaende  nur eines
//   node werkzeug/pruefung/pruefen.mjs --gegenprobe   beweist, dass sie scheitern kann
//
// Exit 1, sobald ein Blatt tot ist. Ein Gate, das nicht scheitern kann,
// beweist nichts (PRUEFLISTE 7.1) — deshalb die Gegenprobe.
//
// BRAUCHT: playwright-core und eine Chromium-Installation. Beides wird unten
// gesucht; kein Download, keine Build-Kette. Findet es nichts, sagt es das und
// nennt den Weg.

import { createRequire } from 'node:module'
import { readdirSync, existsSync, mkdirSync, writeFileSync, readFileSync } from 'node:fs'
import { basename, resolve, join } from 'node:path'
import { homedir } from 'node:os'

const require = createRequire(import.meta.url)
const WURZEL = resolve(new URL('.', import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1'), '../..')
const WERK = join(WURZEL, 'werkzeug')
const SHOTS = join(WERK, 'pruefung', 'bilder')

/* ---------- Werkzeuge finden, statt Pfade zu behaupten ------------------ */
function ladePlaywright() {
  const orte = [
    'playwright-core', 'playwright',
    join(homedir(), 'Klarheit/node_modules/playwright-core'),
    join(WURZEL, 'node_modules/playwright-core'),
  ]
  for (const o of orte) { try { return require(o) } catch {} }
  console.error('playwright-core nicht gefunden. Gesucht in:\n  ' + orte.join('\n  ')
    + '\nAbhilfe: `npm i playwright-core` irgendwo, dann den Pfad hier ergaenzen.')
  process.exit(2)
}
function findeChromium() {
  const basen = [join(homedir(), 'AppData/Local/ms-playwright'),
                 join(homedir(), '.cache/ms-playwright')]
  for (const b of basen) {
    if (!existsSync(b)) continue
    const kandidaten = readdirSync(b).filter(d => d.startsWith('chromium-')).sort().reverse()
    for (const k of kandidaten) {
      for (const rel of ['chrome-win64/chrome.exe', 'chrome-linux/chrome', 'chrome-mac/Chromium.app/Contents/MacOS/Chromium']) {
        const p = join(b, k, rel); if (existsSync(p)) return p
      }
    }
  }
  for (const p of ['C:/Program Files/Google/Chrome/Application/chrome.exe',
                   '/usr/bin/chromium', '/usr/bin/google-chrome']) if (existsSync(p)) return p
  return null   // playwright sucht dann selbst
}

/* ---------- eine Datei pruefen ------------------------------------------ */
async function pruefeBlatt(browser, datei) {
  const name = basename(datei)
  const ctx = await browser.newContext({ viewport: { width: 1600, height: 1100 } })
  const page = await ctx.newPage()
  const fehler = []
  page.on('pageerror', e => fehler.push(`pageerror: ${e.message.split('\n')[0]}`))
  page.on('console', m => { if (m.type() === 'error') fehler.push(`console: ${m.text().slice(0, 200)}`) })
  let mass = {}
  try {
    await page.goto('file:///' + datei.replace(/\\/g, '/'), { waitUntil: 'load', timeout: 20000 })
    // Zweimal messen, nicht einmal hoffen: auf dem ersten Frame nach grossem
    // innerHTML stehen die Schriftmasse noch nicht (PRUEFLISTE 2.4).
    await page.waitForTimeout(900)
    mass = await page.evaluate(() => {
      // Ein Blatt, das seinen Stand verschweigt, ist ein Blatt, das eine
      // entschiedene Frage wieder aufmacht (PRUEFLISTE 5.1).
      //
      // Bis 2026-08-21 stand hier nur `!!document.querySelector(...)` — die
      // reine EXISTENZ. In formfaktor.html lag das Standband dadurch
      // vollstaendig unter dem fixen Warnband und dem Messschirm: 920
      // Stichpunkte, 0 sichtbar, und die Pruefung meldete "heil". Ein
      // Element, das da ist und niemand sieht, sagt nichts.
      /* Das Standband wird ueber #standband gefunden, sonst ueber sein
         GERECHNETES Polster. Der alte Weg war ein Zeichenketten-Vergleich im
         style-Attribut — und der zerbrach, sobald irgendein Skript eine
         Eigenschaft am selben Element setzte: der Browser serialisiert das
         Attribut dann neu und schreibt "padding: 7px 26px" mit Leerzeichen.
         Gemessen am 2026-08-21 an formfaktor.html, unmittelbar nachdem dort
         ein margin-top gesetzt wurde. Neue Blaetter tragen id="standband". */
      const sb = document.querySelector('#standband')
        || [...document.body.children].find(e =>
             e.tagName === 'DIV' && getComputedStyle(e).padding === '7px 26px')
      if (!sb) return { knoten: document.body.querySelectorAll('*').length,
                        text: (document.body.innerText || '').trim().length,
                        standband: false }
      const r = sb.getBoundingClientRect()
      let sichtbar = 0, geprueft = 0
      const decker = {}
      for (let y = r.top + 2; y < Math.min(r.bottom, innerHeight) - 2; y += 4)
        for (let x = r.left + 8; x < Math.min(r.right, innerWidth) - 8; x += 40) {
          geprueft++
          const el = document.elementFromPoint(x, y)
          if (el === sb || sb.contains(el)) sichtbar++
          else { const k = el ? (el.id || (typeof el.className === 'string'
                   ? el.className.split(' ')[0] : '') || el.tagName) : 'ausserhalb'
                 decker[k] = (decker[k] || 0) + 1 }
        }
      return {
        knoten: document.body.querySelectorAll('*').length,
        text: (document.body.innerText || '').trim().length,
        standband: true,
        sbGeprueft: geprueft, sbSichtbar: sichtbar,
        sbProzent: geprueft ? Math.round(sichtbar / geprueft * 100) : 0,
        sbDecker: Object.entries(decker).sort((a, b) => b[1] - a[1])
          .slice(0, 3).map(([k, n]) => `${k}×${n}`).join(', ')
      }
    })
    if (mass.knoten < 20) fehler.push(`leer: nur ${mass.knoten} Knoten im body`)
    if (mass.text < 40) fehler.push(`stumm: nur ${mass.text} Zeichen Text`)
    if (!mass.standband) fehler.push('kein Standband — das Blatt sagt nicht, was es ist')
    else if (!mass.sbGeprueft) fehler.push('Standband hat keine Flaeche — nichts zu sehen')
    else if (mass.sbProzent < 100)
      fehler.push(`Standband nur zu ${mass.sbProzent} % sichtbar `
        + `(${mass.sbSichtbar}/${mass.sbGeprueft} Stichpunkte), verdeckt von ${mass.sbDecker}`)
    mkdirSync(SHOTS, { recursive: true })
    await page.screenshot({ path: join(SHOTS, name.replace('.html', '.png')) })
  } catch (e) {
    fehler.push(`abbruch: ${String(e.message).split('\n')[0]}`)
  }
  await ctx.close()
  return { name, fehler, mass }
}

/* ---------- Gegenprobe: kann die Pruefung ueberhaupt scheitern? --------- */
async function gegenprobe(browser) {
  const tmp = join(WERK, 'pruefung', '.gegenprobe')
  mkdirSync(tmp, { recursive: true })
  const echt = readFileSync(join(WERK, 'zustaende.html'), 'utf8')
  // Das Band strukturell herausschneiden, nicht per Muster: die Blaetter haben
  // CRLF-Zeilenenden, und ein Muster mit \n am Ende greift dort ins Leere —
  // die Gegenprobe meldete dann "kein Fehler erkannt", obwohl sie den Fall gar
  // nicht erzeugt hatte. Ein Gate, das seinen eigenen Testfall verfehlt, ist
  // dieselbe Sorte Blindheit, die es finden soll.
  const ohneBand = (() => {
    const a = echt.indexOf('<!-- Standband')
    if (a < 0) return null
    const e = echt.indexOf('</div>', echt.indexOf('<div', a))
    return e < 0 ? null : echt.slice(0, a) + echt.slice(e + 6)
  })()
  /* Der vierte Fall gehoert zum Sichtbarkeits-Riegel (2026-08-21): ein
     Standband, das DA ist und trotzdem niemand sieht. Genau dieser Zustand
     lief in formfaktor.html monatelang als "heil" durch, weil die Pruefung
     nur die Existenz abfragte. Eine Gegenprobe, die den neuen Riegel nicht
     abdeckt, laesst ihn unbewiesen. */
  const verdeckt = echt.replace('</body>',
    '<div style="position:fixed;inset:0;background:#000;z-index:9999"></div></body>')
  const faelle = {
    'syntax.html': echt.replace('<script>', '<script>\nconst x = ;\n'),   // tonlos tot
    'leer.html': '<!doctype html><html><body></body></html>',              // zeichnet nichts
    ...(ohneBand ? { 'ohneband.html': ohneBand } : {}),
    ...(verdeckt !== echt ? { 'verdecktesband.html': verdeckt } : {}),
  }
  if (!ohneBand) console.log('WARNUNG: Standband-Fall nicht erzeugbar — Vorlage geaendert?')
  if (verdeckt === echt) console.log('WARNUNG: Verdeckungsfall nicht erzeugbar — kein </body>?')
  let bestanden = 0
  for (const [n, inhalt] of Object.entries(faelle)) {
    writeFileSync(join(tmp, n), inhalt)
    const r = await pruefeBlatt(browser, join(tmp, n))
    const ok = r.fehler.length > 0
    console.log(`${ok ? 'ok  ' : 'FEHL'}  ${n} wird gemeldet: ${r.fehler[0] || '(nichts)'}`)
    if (ok) bestanden++
  }
  console.log(`\nGEGENPROBE: ${bestanden}/${Object.keys(faelle).length} kuenstliche Fehler erkannt`)
  return bestanden === Object.keys(faelle).length
}

/* ---------- Ablauf ------------------------------------------------------ */
const { chromium } = ladePlaywright()
const exe = findeChromium()
const browser = await chromium.launch(exe ? { executablePath: exe } : {})

const arg = process.argv[2]
if (arg === '--gegenprobe') {
  const ok = await gegenprobe(browser)
  await browser.close()
  process.exit(ok ? 0 : 1)
}

const dateien = arg
  ? [join(WERK, arg.endsWith('.html') ? arg : arg + '.html')]
  : readdirSync(WERK).filter(f => f.endsWith('.html')).sort().map(f => join(WERK, f))

let kaputt = 0
for (const d of dateien) {
  const r = await pruefeBlatt(browser, d)
  if (r.fehler.length) { kaputt++; console.log(`FAIL ${r.name}: ${r.fehler.join(' | ')}`) }
  else console.log(`OK   ${r.name}  (${r.mass.knoten} Knoten, ${r.mass.text} Zeichen, `
    + `Standband ${r.mass.sbSichtbar}/${r.mass.sbGeprueft} Stichpunkte sichtbar)`)
}
await browser.close()
console.log(`\nERGEBNIS: ${dateien.length - kaputt}/${dateien.length} Blaetter heil`)
console.log(`Bilder: werkzeug/pruefung/bilder/`)
process.exit(kaputt ? 1 : 0)
