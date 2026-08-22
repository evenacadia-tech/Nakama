async (page) => {
  const S = 'C:/Users/phili/Projekte/Nakama/docs/hub';
  const ctx = page.context();
  await ctx.addInitScript(() => {
    window.claude = { use: async (n) => n === 'artifact' ? { publish: async (html) => { window.__published = html; return { version: 'test-1' }; } } : null };
  });
  const p2 = await ctx.newPage();
  await p2.goto('file:///' + S + '/hub-test.html');
  await p2.waitForSelector('#datei');
  await p2.setInputFiles('#datei', S + '/../../tools/hub/test/beispiel-review.md');
  await p2.waitForFunction(() => !!window.__published, null, { timeout: 10000 });
  const status = await p2.textContent('#status');
  const kopf = await p2.evaluate(() => ({ laenge: window.__published.length, startsDoctype: window.__published.startsWith('<!doctype html>'), scripts: (window.__published.match(/<script/g)||[]).length }));
  const [download] = await Promise.all([
    p2.waitForEvent('download'),
    p2.evaluate(() => { const a = document.createElement('a'); a.href = URL.createObjectURL(new Blob([window.__published], {type: 'text/html'})); a.download = 'hub-published.html'; document.body.appendChild(a); a.click(); })
  ]);
  await download.saveAs(S + '/hub-published.html');
  // Zweite Runde: die veröffentlichte Seite laden und prüfen, ob sie den Eingang zeigt
  const p3 = await ctx.newPage();
  await p3.goto('file:///' + S + '/hub-published.html');
  await p3.waitForSelector('#app');
  const eingang = await p3.evaluate(() => JSON.parse(document.getElementById('hub-state').textContent).eingang);
  const sichtbar = await p3.textContent('#reviews');
  const titel = await p3.title();
  await p2.close(); await p3.close();
  return JSON.stringify({ status, startsDoctype: kopf.startsDoctype, laenge: kopf.laenge, eingangN: eingang.length, name: eingang[0]?.name, textOk: eingang[0]?.text.includes('<script>alert(1)</script>'), zeigtEingang: sichtbar.includes('Im Eingang'), titel, scriptsImDoc: kopf.scripts });
}
