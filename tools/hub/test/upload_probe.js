// Upload-Probe, Schritt 1 — laeuft im Playwright-MCP (browser_run_code_unsafe, filename=…).
// Stubbt window.claude, laedt docs/hub/hub-test.html (Build + Huelle), laedt eine Review
// UND ein Bild mit Notiz hoch und legt das Volldokument, das die Seite veroeffentlichen
// wuerde, als Download nach docs/hub/hub-published.html.
// Dann Schritt 2: tools/hub/test/upload_probe_2.js
async (page) => {
  const S = 'C:/Users/phili/Projekte/Nakama/docs/hub';
  const ctx = page.context();
  await ctx.addInitScript(() => {
    window.claude = { use: async (n) => n === 'artifact' ? { publish: async (html) => { window.__published = html; return { version: 'test-1' }; } } : null };
  });
  const p2 = await ctx.newPage();
  await p2.goto('file:///' + S + '/hub-test.html');
  await p2.waitForSelector('#datei');
  await p2.fill('#notiz', 'Probe: Review und Bild zusammen');
  await p2.setInputFiles('#datei', [S + '/../../tools/hub/test/beispiel-review.md', S + '/bilder/u3-wortmarke-gen-export.png']);
  await p2.waitForFunction(() => !!window.__published, null, { timeout: 10000 });
  const status = await p2.textContent('#status');
  const kopf = await p2.evaluate(() => ({ laenge: window.__published.length, startsDoctype: window.__published.startsWith('<!doctype html>'), scripts: (window.__published.match(/<script/g)||[]).length }));
  // Volldokument auf die Platte: als Download aus der Seite (kein fs im MCP-Prozess)
  const [download] = await Promise.all([
    p2.waitForEvent('download'),
    p2.evaluate(() => { const a = document.createElement('a'); a.href = URL.createObjectURL(new Blob([window.__published], {type: 'text/html'})); a.download = 'hub-published.html'; document.body.appendChild(a); a.click(); })
  ]);
  await download.saveAs(S + '/hub-published.html');
  await p2.close();
  return JSON.stringify({ schritt: 1, status, startsDoctype: kopf.startsDoctype, laenge: kopf.laenge, scriptsImDoc: kopf.scripts });
}
