// Upload-Probe, Schritt 2 — laedt das von der Seite veroeffentlichte Volldokument
// (docs/hub/hub-published.html, aus Schritt 1 + storage_zu_html.py) und prueft,
// dass Review, Bild und Notiz im #hub-state stehen und der Eingang sichtbar ist.
async (page) => {
  const S = 'C:/Users/phili/Projekte/Nakama/docs/hub';
  const p3 = await page.context().newPage();
  await p3.goto('file:///' + S + '/hub-published.html');
  await p3.waitForSelector('#app');
  const eingang = await p3.evaluate(() => JSON.parse(document.getElementById('hub-state').textContent).eingang);
  const sichtbar = await p3.textContent('#reviews');
  const titel = await p3.title();
  const bildGezeigt = await p3.evaluate(() => { const i = document.querySelector('#reviews .vorschau img'); return i ? (i.complete && i.naturalWidth > 0 ? i.naturalWidth + 'x' + i.naturalHeight : 'laedt nicht') : 'kein Bild'; });
  await p3.close();
  const bild = eingang.find(x => x.typ === 'bild'), text = eingang.find(x => x.typ === 'text');
  return JSON.stringify({ schritt: 2, eingangN: eingang.length, textName: text && text.name, textOk: !!text && text.text.includes('<script>alert(1)</script>'), bildName: bild && bild.name, bildSrcOk: !!bild && bild.src.startsWith('data:image/png;base64,'), notiz: text && text.notiz, bildGezeigt, zeigtEingang: sichtbar.includes('Im Eingang'), titel });
}
