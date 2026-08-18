<script lang="ts">
	// EQ-Copilot M2 — Sensorübersicht (Plan §11 M2: „Sensorübersicht in der App").
	// Zeigt den Live-Stand des Brokers: Messpunkte gruppiert nach FL-Prozess,
	// PRE/POST-Paare mit ehrlicher Timing-Stufe, Profilbindung, Aggregat-Snapshot.
	// Sprache: Lernsprache wie im Plugin (memory anleitungen-in-lernsprache) —
	// Rollen heißen hier wie in der Plugin-Combo („Misst diese Spur" …).
	// Getrennte/stale Messpunkte bleiben SICHTBAR — nie still weg (Plan §8.4).
	import { invoke } from "@tauri-apps/api/core";
	import { stacks } from "$lib/store.svelte";

	interface MessStand {
		zustand: string;
		metrics_version: string;
		aktiv_s: number;
		gesamt_s: number;
		lufs_i: number | null;
		lufs_s: number | null;
		true_peak_db: number | null;
		crest_db: number | null;
		projekt_fenster: { von_samples: number; bis_samples: number; spruenge: number } | null;
	}
	interface Sensor {
		sensor_id: string;
		role: string;
		label: string;
		pair_id: string | null;
		profil_id: string | null;
		plugin_version: string;
		protokoll_version: number;
		host_pid: number;
		samplerate: number;
		verbunden: boolean;
		konflikt: boolean;
		stale: boolean;
		lebende: number;
		heartbeats: number;
		getrennt_seit_ms: number | null;
		messung: MessStand | null;
	}
	interface SessionInfo {
		host_pid: number;
		sensoren: number;
		verbundene: number;
		samplerates: number[];
	}
	interface PaarStatus {
		pair_id: string;
		pre_sensor_id: string | null;
		post_sensor_id: string | null;
		status: string;
		timing: string;
		grund: string;
	}
	interface BrokerStatus {
		laeuft: boolean;
		broker_version: string;
		protokoll_version: number;
		pipe_name: string;
		pakete_verworfen: number;
		fehler: string[];
		sensoren: Sensor[];
		sessions: SessionInfo[];
		paare: PaarStatus[];
	}

	let status = $state<BrokerStatus | null>(null);
	let meldung = $state("");

	async function laden() {
		try {
			status = await invoke<BrokerStatus>("eq_copilot_status");
		} catch (e) {
			meldung = `Status nicht lesbar: ${e}`;
		}
	}

	$effect(() => {
		laden();
		const t = setInterval(laden, 1000);
		return () => clearInterval(t);
	});

	// Rollen exakt wie in der Plugin-Combo — EIN Vokabular, zwei Oberflächen.
	const rolleText = (r: string) =>
		r === "hub" ? "Sammelpunkt (Master)"
		: r === "pre" ? "VOR dem EQ"
		: r === "post" ? "NACH dem EQ"
		: "Misst diese Spur";

	const zustandText = (s: Sensor): string => {
		if (!s.verbunden) return "getrennt";
		if (s.stale) return "sendet nicht mehr";
		if (s.protokoll_version <= 1) return "altes Plugin (v1) — ohne Messdaten";
		const m = s.messung;
		if (!m || m.zustand === "keine_daten") return "wartet auf Musik";
		if (m.zustand === "sammelt") return "hört zu …";
		return "messbereit";
	};

	const dbTxt = (v: number | null, einheit: string) =>
		v === null ? "–" : `${v.toFixed(1)} ${einheit}`;

	const seitText = (ms: number | null): string => {
		if (!ms) return "";
		const min = Math.round((Date.now() - ms) / 60000);
		return min < 1 ? "gerade eben" : min === 1 ? "vor 1 min" : `vor ${min} min`;
	};

	const timingText = (p: PaarStatus) =>
		p.status === "unvollstaendig" ? "unvollständig"
		: p.timing === "ausgerichtet" ? "Passagen deckungsgleich"
		: p.timing === "wahrscheinlich" ? "vorher/nachher wahrscheinlich"
		: "nicht vergleichbar";

	function sensorName(id: string | null): string {
		if (!id || !status) return "?";
		const s = status.sensoren.find((x) => x.sensor_id === id);
		return s ? s.label || rolleText(s.role) : "?";
	}

	async function binden(s: Sensor, profilId: string) {
		try {
			await invoke("eq_copilot_profil_binden", {
				sensorId: s.sensor_id,
				profilId: profilId === "" ? null : profilId
			});
			await laden();
		} catch (e) {
			meldung = `Bindung fehlgeschlagen: ${e}`;
		}
	}

	async function aggregat() {
		try {
			const pfad = await invoke<string>("eq_copilot_aggregat_schreiben", {
				profilId: null,
				hostPid: null
			});
			meldung = `Gemeinsamer Snapshot festgehalten: ${pfad}`;
		} catch (e) {
			meldung = `Snapshot fehlgeschlagen: ${e}`;
		}
	}

	// Profile mit stabiler Kennung — nur die sind bindbar (F3.1-Zusage).
	const bindbareProfile = $derived(stacks.list.filter((s) => s.profile_id));
	const profilName = (id: string | null) =>
		id ? (stacks.list.find((s) => s.profile_id === id)?.name ?? "unbekanntes Profil") : "";

	const sensorenVon = (pid: number) =>
		(status?.sensoren ?? []).filter((s) => s.host_pid === pid);
</script>

<div class="wrap">
	{#if !status}
		<p class="muted">Lade Brokerstatus …</p>
	{:else}
		<div class="kopfzeile">
			{#if status.laeuft}
				<span class="pille gruen">Verbindung zu FL bereit</span>
			{:else}
				<span class="pille rot">Broker läuft nicht</span>
			{/if}
			<span class="muted klein">Protokoll v{status.protokoll_version} · Broker {status.broker_version}</span>
			<button class="ghost sm" onclick={aggregat} disabled={!status.sensoren.some((s) => s.verbunden)}
				title="Messstand ALLER Messpunkte in eine gemeinsame Datei festhalten (zeitausgerichtet)">
				Gemeinsamen Snapshot festhalten
			</button>
		</div>

		{#if status.fehler.length}
			<details class="fehler">
				<summary>{status.fehler.length} Meldung{status.fehler.length === 1 ? "" : "en"}</summary>
				<ul>{#each status.fehler.slice(-8) as f}<li>{f}</li>{/each}</ul>
			</details>
		{/if}

		{#if !status.sensoren.length}
			<p class="muted pad">
				Noch keine Messpunkte. Lade das EQ-Copilot-Plugin auf eine Spur oder den
				Master in FL — es meldet sich hier von selbst.
			</p>
		{/if}

		{#each status.sessions as sess (sess.host_pid)}
			<div class="session">
				<div class="session-kopf">
					FL-Prozess {sess.host_pid}
					<span class="muted klein">
						· {sess.verbundene}/{sess.sensoren} verbunden
						{#if sess.samplerates.length === 1}· {sess.samplerates[0] / 1000} kHz{/if}
						{#if sess.samplerates.length > 1}· ⚠ uneinheitliche Samplerates{/if}
					</span>
				</div>
				{#each sensorenVon(sess.host_pid) as s (s.sensor_id)}
					<div class="sensor" class:tot={!s.verbunden} class:warnung={s.stale || s.konflikt}>
						<div class="s-zeile">
							<span class="rolle">{rolleText(s.role)}</span>
							<span class="s-name">{s.label || "(ohne Namen)"}</span>
							{#if s.pair_id}<span class="paar-tag" title="Paar-Kennung">{s.pair_id}</span>{/if}
							<span class="zustand"
								class:rot={s.stale || s.konflikt}
								class:grau={!s.verbunden}>
								{zustandText(s)}{#if !s.verbunden && s.getrennt_seit_ms}&nbsp;({seitText(s.getrennt_seit_ms)}){/if}
							</span>
						</div>
						{#if s.konflikt}
							<div class="konflikt">
								⚠ Zwei Instanzen melden dieselbe Kennung (nach Duplizieren normal) —
								öffne EINE davon in FL und gib ihr dort eine neue Kennung.
							</div>
						{/if}
						<div class="s-zeile klein">
							{#if s.messung && s.verbunden && s.protokoll_version >= 2}
								<span class="mess">Lautheit {dbTxt(s.messung.lufs_i, "LUFS")}</span>
								<span class="mess">Spitze {dbTxt(s.messung.true_peak_db, "dBTP")}</span>
								<span class="mess">{Math.round(s.messung.aktiv_s)} s Musik gehört</span>
								{#if s.messung.projekt_fenster && s.messung.projekt_fenster.spruenge > 0}
									<span class="mess warn" title="Loop oder Sprung im Transport — die Messung deckt keine durchgehende Passage ab">
										{s.messung.projekt_fenster.spruenge} Sprünge
									</span>
								{/if}
							{/if}
							<span class="binden">
								Song:
								<select
									value={s.profil_id ?? ""}
									onchange={(e) => binden(s, e.currentTarget.value)}
									title="Diesen Messpunkt einem Projektprofil zuordnen — fremde Songs geraten dann nie in dieselbe Auswertung">
									<option value="">— kein Song —</option>
									{#each bindbareProfile as p (p.profile_id)}
										<option value={p.profile_id}>{p.name}</option>
									{/each}
									{#if s.profil_id && !bindbareProfile.some((p) => p.profile_id === s.profil_id)}
										<option value={s.profil_id}>{profilName(s.profil_id)}</option>
									{/if}
								</select>
							</span>
						</div>
					</div>
				{/each}
			</div>
		{/each}

		{#if status.paare.length}
			<div class="paare">
				<div class="session-kopf">Vorher/Nachher-Paare</div>
				{#each status.paare as p (p.pair_id)}
					<div class="paar">
						<span class="ampel"
							class:gruen={p.timing === "ausgerichtet"}
							class:bernstein={p.timing === "wahrscheinlich"}
							class:rot={p.timing === "unklar" || p.status === "unvollstaendig"}></span>
						<span class="paar-name">{p.pair_id}</span>
						<span class="klein muted">{sensorName(p.pre_sensor_id)} → {sensorName(p.post_sensor_id)}</span>
						<span class="paar-status">{timingText(p)}</span>
						<span class="klein muted grund">{p.grund}</span>
					</div>
				{/each}
			</div>
		{/if}

		{#if meldung}
			<p class="meldung">{meldung}</p>
		{/if}
	{/if}
</div>

<style>
	.wrap {
		display: flex;
		flex-direction: column;
		gap: 10px;
		font-size: 13px;
	}
	.kopfzeile {
		display: flex;
		align-items: center;
		gap: 10px;
		flex-wrap: wrap;
	}
	.pille {
		font-size: 11px;
		border-radius: 10px;
		padding: 2px 10px;
	}
	.pille.gruen {
		background: #1f3d29;
		color: #7fd99b;
	}
	.pille.rot {
		background: #3d2424;
		color: #e09a9a;
	}
	.ghost {
		background: #23262e;
		color: #d7dae0;
		border: 1px solid #353a45;
		border-radius: 5px;
		padding: 5px 9px;
		cursor: pointer;
		font: inherit;
	}
	.ghost.sm {
		padding: 3px 8px;
		font-size: 11px;
	}
	.ghost:disabled {
		opacity: 0.4;
		cursor: default;
	}
	.fehler summary {
		cursor: pointer;
		color: #d8c483;
		font-size: 12px;
	}
	.fehler ul {
		margin: 4px 0 0;
		padding-left: 18px;
		color: #b9a76a;
		font-size: 11px;
	}
	.session-kopf {
		font-size: 11px;
		text-transform: uppercase;
		letter-spacing: 0.06em;
		color: #8a92a1;
		margin-bottom: 4px;
	}
	.sensor {
		background: #1b1e25;
		border: 1px solid #262b34;
		border-radius: 6px;
		padding: 7px 10px;
		margin-bottom: 5px;
		display: flex;
		flex-direction: column;
		gap: 4px;
	}
	.sensor.warnung {
		border-color: #7a3a3a;
	}
	.sensor.tot {
		opacity: 0.65;
	}
	.s-zeile {
		display: flex;
		align-items: center;
		gap: 8px;
		flex-wrap: wrap;
	}
	.rolle {
		flex: none;
		font-size: 10.5px;
		background: #243a4a;
		color: #8fc7e6;
		border-radius: 3px;
		padding: 1px 6px;
		white-space: nowrap;
	}
	.s-name {
		font-weight: 600;
		min-width: 0;
		overflow: hidden;
		text-overflow: ellipsis;
	}
	.paar-tag {
		font-size: 10px;
		background: #2a2230;
		color: #b79ac0;
		border-radius: 3px;
		padding: 1px 6px;
	}
	.zustand {
		margin-left: auto;
		font-size: 11.5px;
		color: #7fd99b;
		white-space: nowrap;
	}
	.zustand.rot {
		color: #e09a9a;
	}
	.zustand.grau {
		color: #7f8694;
	}
	.konflikt {
		font-size: 11.5px;
		color: #e0b0b0;
		background: rgba(224, 122, 122, 0.07);
		border: 1px solid #3d2424;
		border-radius: 5px;
		padding: 4px 8px;
		line-height: 1.4;
	}
	.klein {
		font-size: 11px;
	}
	.mess {
		color: #c2c8d2;
		background: #23262e;
		border-radius: 3px;
		padding: 1px 6px;
		white-space: nowrap;
	}
	.mess.warn {
		color: #d8c483;
		background: #3a3320;
	}
	.binden {
		margin-left: auto;
		display: flex;
		align-items: center;
		gap: 5px;
		color: #8a92a1;
	}
	.binden select {
		background: #23262e;
		color: #d7dae0;
		border: 1px solid #353a45;
		border-radius: 4px;
		font-size: 11px;
		padding: 2px 4px;
		max-width: 180px;
	}
	.paar {
		display: flex;
		align-items: center;
		gap: 8px;
		background: #1b1e25;
		border: 1px solid #262b34;
		border-radius: 6px;
		padding: 6px 10px;
		margin-bottom: 4px;
		flex-wrap: wrap;
	}
	.ampel {
		flex: none;
		width: 9px;
		height: 9px;
		border-radius: 50%;
		background: #7f8694;
	}
	.ampel.gruen {
		background: #5f9d6b;
	}
	.ampel.bernstein {
		background: #c98a3c;
	}
	.ampel.rot {
		background: #b04a38;
	}
	.paar-name {
		font-weight: 600;
	}
	.paar-status {
		font-size: 11.5px;
		color: #c2c8d2;
	}
	.grund {
		flex-basis: 100%;
		padding-left: 17px;
	}
	.meldung {
		font-size: 11.5px;
		color: #8fc7e6;
		margin: 0;
		word-break: break-all;
	}
	.muted {
		color: #7f8694;
	}
	.pad {
		padding: 10px 2px;
	}
</style>
