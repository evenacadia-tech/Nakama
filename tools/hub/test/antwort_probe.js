// Antwort-Probe — laeuft im Playwright-MCP (browser_run_code_unsafe, filename=…).
// Stubbt window.claude, setzt drei Antworten (Gesamturteil U2.0, Punkt U2.2, Karte U3),
// prueft die Leiste und den Browser-Entwurf (localStorage), speichert, und legt das
// Volldokument als Download nach docs/hub/hub-published.html.
// Danach: py -3.13 tools/hub/hub_eingang.py docs/hub/hub-published.html  → antworten in hub.json (Status neu)
async (page) => {
  const S = 'C:/Users/phili/Projekte/Nakama/docs/hub';
  const ctx = page.context();
  await ctx.addInitScript(() => {
    window.claude = { use: async (n) => n === 'artifact' ? { publish: async (html) => { window.__published = html; return { version: 'test-2' }; } } : null };
  });
  const p = await ctx.newPage({ viewport: { width: 1100, height: 900 } });
  await p.goto('file:///' + S + '/hub-test.html');
  await p.waitForSelector('.antwort[data-id="U2.0"]');
  const leisteVorher = await p.evaluate(() => getComputedStyle(document.getElementById('leiste')).display);
  await p.check('.antwort[data-id="U2.0"] input[value="Entwurf lassen"]');
  await p.fill('.antwort[data-id="U2.0"] textarea', 'Ja, die Richtung stimmt — „so weiter“ <test>');
  await p.check('.antwort[data-id="U2.2"] input[value="Alternative"]');
  await p.check('.antwort[data-id="U3"] input[value="Ja"]');
  await p.fill('.antwort[data-id="U3"] textarea', 'ja, rein in die Truhe');
  await p.waitForTimeout(200);
  const leiste = await p.evaluate(() => ({ display: getComputedStyle(document.getElementById('leiste')).display, zaehl: document.getElementById('leiste-zaehl').textContent, entwurf: JSON.parse(localStorage.getItem('hub-antwort-entwurf')||'{}') }));
  // Entwurf ueberlebt ein Neuladen?
  await p.reload(); await p.waitForSelector('.antwort[data-id="U2.0"]');
  const nachReload = await p.evaluate(() => ({ u20: document.querySelector('.antwort[data-id="U2.0"] textarea').value, u22: document.querySelector('.antwort[data-id="U2.2"] input:checked')?.value, zaehl: document.getElementById('leiste-zaehl').textContent }));
  await (await p.$('.karte.jetzt')).screenshot({ path: 'C:/Users/phili/Projekte/Nakama/.playwright-mcp/hub-antwort-karte.png' });
  await p.click('#leiste-speichern');
  await p.waitForFunction(() => !!window.__published, null, { timeout: 10000 });
  const meld = await p.textContent('#leiste-meld');
  const pub = await p.evaluate(() => { const s = JSON.parse(window.__published.match(/<script type="application\/json" id="hub-state">([\s\S]*?)<\/script>/)[1]); return { antworten: s.antworten, laenge: window.__published.length, lsNachher: localStorage.getItem('hub-antwort-entwurf') }; });
  const [download] = await Promise.all([
    p.waitForEvent('download'),
    p.evaluate(() => { const a = document.createElement('a'); a.href = URL.createObjectURL(new Blob([window.__published], {type: 'text/html'})); a.download = 'hub-published.html'; document.body.appendChild(a); a.click(); })
  ]);
  await download.saveAs(S + '/hub-published.html');
  await p.close();
  return JSON.stringify({ leisteVorher, leiste, nachReload, meld, pub });
}
