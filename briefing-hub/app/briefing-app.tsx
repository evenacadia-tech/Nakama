'use client';

import { FormEvent, useEffect, useMemo, useState } from 'react';
import {
  DecisionPackage,
  decisionPackages,
  friendlyQuestions,
  phaseCopy,
  taskCopy,
} from '@/data/friendly-copy';

type View = 'today' | 'decisions' | 'decision' | 'status' | 'design';

type AnswerRecord = {
  choice: string;
  note: string;
  author: string;
  status: string;
  updatedAt: number;
};

type AddedItem = {
  id: number;
  kind: string;
  title: string;
  summary: string;
  reason: string;
  urgency: string;
  author: string;
  status: string;
  createdAt: number;
  updatedAt: number;
};

type PlanRow = {
  id: string;
  ticket: string;
  status: 'erledigt' | 'gebaut' | 'naechster' | 'offen';
  datum: string;
  text: string;
  beleg?: string;
};

type PlanPhase = {
  phase: string;
  titel: string;
  ziel: string;
  zeilen: PlanRow[];
};

type DesignApp = {
  key: 'gen' | 'probeeq' | 'suna';
  name: string;
  rolle: string;
  groesse: string;
  stand_text: string;
  status: string;
  offen: string;
  wo: string;
};

export type HubData = {
  stand: string;
  stand_notiz: string;
  naechster_schritt: {
    technik: string;
    design: string;
    dich: string;
  };
  bei_dir: Array<{ id: string }>;
  plan: PlanPhase[];
  design: {
    massstab: string;
    phase: string;
    apps: DesignApp[];
    quelle: {
      titel: string;
      url: string;
      wort: string;
      datum: string;
    };
  };
  antworten?: Record<
    string,
    {
      wahl?: string;
      text?: string;
      status?: string;
    }
  >;
};

type BriefingAppProps = {
  initialHub: HubData;
};

const navigation: Array<{ id: Exclude<View, 'decision'>; label: string }> = [
  { id: 'today', label: 'Heute' },
  { id: 'decisions', label: 'Entscheiden' },
  { id: 'status', label: 'Stand' },
  { id: 'design', label: 'Design' },
];

const designPresentation: Record<DesignApp['key'], { image: string; summary: string }> = {
  gen: {
    image: '/images/nakama-gen.png',
    summary: 'Zeigt Quellen, Befunde und den kleinsten sinnvollen Hörtest. Gen berät, entscheidet aber nicht allein.',
  },
  probeeq: {
    image: '/images/nakama-probeeq.png',
    summary: 'Ein vollwertiger EQ, der Vorschläge von Gen übernehmen kann und jederzeit manuell bedienbar bleibt.',
  },
  suna: {
    image: '/images/nakama-suna.png',
    summary: 'Eine kleine passive Kachel. Sie zeigt Verbindung, Rolle, Position und Messstand einer Quelle.',
  },
};

function cleanText(value: string) {
  return value.replace(/\s*[—–]\s*/g, ' - ').replace(/\s+/g, ' ').trim();
}

function germanDate(value: string) {
  const [year, month, day] = value.split('-').map(Number);
  if (!year || !month || !day) return value;
  return new Intl.DateTimeFormat('de-DE', {
    day: 'numeric',
    month: 'long',
    year: 'numeric',
  }).format(new Date(Date.UTC(year, month - 1, day)));
}

function initialAnswerMap(hub: HubData): Record<string, AnswerRecord> {
  return Object.fromEntries(
    Object.entries(hub.antworten ?? {})
      .filter(([, answer]) => Boolean(answer.wahl))
      .map(([id, answer]) => [
        id,
        {
          choice: answer.wahl ?? '',
          note: answer.text ?? '',
          author: 'Phil',
          status: answer.status ?? 'neu',
          updatedAt: 0,
        },
      ]),
  );
}

function packageProgress(decision: DecisionPackage, answers: Record<string, AnswerRecord>) {
  const answered = decision.questionIds.filter((id) => answers[id]?.choice && answers[id].choice !== 'Später').length;
  return { answered, total: decision.questionIds.length };
}

function answerIsOpen(answer?: AnswerRecord) {
  return !answer?.choice || answer.choice === 'Später';
}

function Topbar({
  view,
  answers,
  items,
  onNavigate,
  onNewItem,
}: {
  view: View;
  answers: Record<string, AnswerRecord>;
  items: AddedItem[];
  onNavigate: (view: Exclude<View, 'decision'>) => void;
  onNewItem: () => void;
}) {
  const openQuestions = decisionPackages.reduce((sum, item) => {
    const progress = packageProgress(item, answers);
    return sum + progress.total - progress.answered;
  }, 0) + items.filter((item) => item.kind === 'Entscheidung' && answerIsOpen(answers[`item.${item.id}`])).length;

  return (
    <header className="topbar">
      <button className="brand" type="button" onClick={() => onNavigate('today')} aria-label="Nakama Briefing, Startseite">
        <span className="brand-mark" aria-hidden="true">N</span>
        <span>
          <strong>Nakama</strong>
          <small>Briefing</small>
        </span>
      </button>

      <nav className="primary-nav" aria-label="Hauptnavigation">
        {navigation.map((item) => {
          const active = view === item.id || (view === 'decision' && item.id === 'decisions');
          return (
            <button
              className={`nav-link${active ? ' is-active' : ''}`}
              type="button"
              key={item.id}
              onClick={() => onNavigate(item.id)}
              aria-current={active ? 'page' : undefined}
            >
              {item.label}
              {item.id === 'decisions' && (
                <span aria-label={`${openQuestions} offene Einzelfragen`}>{openQuestions}</span>
              )}
            </button>
          );
        })}
      </nav>

      <button className="quiet-action" type="button" onClick={onNewItem}>
        Neuen Punkt anlegen
      </button>
    </header>
  );
}

function Composer({
  onClose,
  onCreated,
}: {
  onClose: () => void;
  onCreated: (item: AddedItem) => void;
}) {
  const [form, setForm] = useState({
    author: 'Phil',
    kind: 'Entscheidung',
    urgency: 'wenn du dazu kommst',
    title: '',
    summary: '',
    reason: '',
  });
  const [state, setState] = useState<'idle' | 'saving' | 'error'>('idle');
  const [error, setError] = useState('');

  async function submit(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();
    setState('saving');
    setError('');

    try {
      const response = await fetch('/api/items', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(form),
      });
      const body = await response.json() as AddedItem & { error?: string };
      if (!response.ok) throw new Error(body.error ?? 'Speichern fehlgeschlagen.');
      onCreated(body);
      onClose();
    } catch (caught) {
      setState('error');
      setError(caught instanceof Error ? caught.message : 'Speichern fehlgeschlagen.');
    }
  }

  return (
    <section className="composer" aria-labelledby="composer-title">
      <div className="composer-heading">
        <div>
          <h2 id="composer-title">Neuen Punkt ins Briefing legen</h2>
          <p>Ein klarer Titel und zwei kurze Sätze reichen. Mehr Verwaltung braucht es nicht.</p>
        </div>
        <button className="text-button" type="button" onClick={onClose}>Schließen</button>
      </div>

      <form onSubmit={submit}>
        <div className="form-row form-row-three">
          <label>
            Wer legt ihn an?
            <select value={form.author} onChange={(event) => setForm({ ...form, author: event.target.value })}>
              <option>Phil</option>
              <option>Claude</option>
              <option>Codex</option>
            </select>
          </label>
          <label>
            Was ist es?
            <select value={form.kind} onChange={(event) => setForm({ ...form, kind: event.target.value })}>
              <option>Entscheidung</option>
              <option>Update</option>
              <option>Blocker</option>
            </select>
          </label>
          <label>
            Wann zählt es?
            <select value={form.urgency} onChange={(event) => setForm({ ...form, urgency: event.target.value })}>
              <option value="jetzt">Jetzt</option>
              <option value="wenn du dazu kommst">Wenn du Zeit hast</option>
              <option value="wissen">Nur wissen</option>
              <option value="später">Später</option>
            </select>
          </label>
        </div>

        <label>
          Kurzer Titel
          <input
            required
            minLength={4}
            maxLength={160}
            value={form.title}
            onChange={(event) => setForm({ ...form, title: event.target.value })}
          />
        </label>

        <label>
          Worum geht es?
          <textarea
            required
            minLength={4}
            maxLength={2000}
            rows={3}
            value={form.summary}
            onChange={(event) => setForm({ ...form, summary: event.target.value })}
          />
        </label>

        <label>
          Warum ist es wichtig? <span className="optional">Optional</span>
          <textarea
            maxLength={1200}
            rows={2}
            value={form.reason}
            onChange={(event) => setForm({ ...form, reason: event.target.value })}
          />
        </label>

        {error && <p className="form-error" role="alert">{error}</p>}

        <div className="form-actions">
          <button className="primary-action" type="submit" disabled={state === 'saving'}>
            {state === 'saving' ? 'Wird gespeichert' : 'Punkt speichern'}
          </button>
          <button className="secondary-action" type="button" onClick={onClose}>Abbrechen</button>
        </div>
      </form>
    </section>
  );
}

function AddedDecisionAnswer({
  item,
  saved,
  onSaved,
}: {
  item: AddedItem;
  saved?: AnswerRecord;
  onSaved: (id: string, answer: AnswerRecord) => void;
}) {
  const questionId = `item.${item.id}`;
  const [open, setOpen] = useState(false);
  const [choice, setChoice] = useState(saved?.choice ?? '');
  const [note, setNote] = useState(saved?.note ?? '');
  const [state, setState] = useState<'idle' | 'saving' | 'error'>('idle');
  const [error, setError] = useState('');

  useEffect(() => {
    setChoice(saved?.choice ?? '');
    setNote(saved?.note ?? '');
  }, [saved?.choice, saved?.note]);

  async function save(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();
    setError('');
    if (!choice) {
      setError('Bitte wähle eine Antwort.');
      return;
    }
    if (choice === 'Eigene Antwort' && note.trim().length < 3) {
      setError('Schreibe bitte kurz dazu, wie du es möchtest.');
      return;
    }

    setState('saving');
    try {
      const response = await fetch('/api/answers', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ questionId, choice, note, author: 'Phil' }),
      });
      const body = await response.json() as AnswerRecord & { error?: string };
      if (!response.ok) throw new Error(body.error ?? 'Speichern fehlgeschlagen.');
      onSaved(questionId, body);
      setState('idle');
      setOpen(false);
    } catch (caught) {
      setState('error');
      setError(caught instanceof Error ? caught.message : 'Speichern fehlgeschlagen.');
    }
  }

  if (!open) {
    return (
      <div className="quick-answer-summary">
        {saved?.choice && <span>Antwort: {saved.choice === 'Eigene Antwort' ? saved.note : saved.choice}</span>}
        <button className="secondary-action" type="button" onClick={() => setOpen(true)}>
          {saved?.choice ? 'Antwort ändern' : 'Antworten'}
        </button>
      </div>
    );
  }

  return (
    <form className="quick-answer" onSubmit={save}>
      <label>
        Deine Antwort
        <select value={choice} onChange={(event) => setChoice(event.target.value)}>
          <option value="">Bitte wählen</option>
          <option value="Ja">Ja</option>
          <option value="Nein">Nein</option>
          <option value="Eigene Antwort">Anders</option>
          <option value="Später">Später</option>
        </select>
      </label>
      <label>
        Kurzer Satz dazu <span className="optional">Optional, außer bei „Anders“</span>
        <textarea
          rows={2}
          maxLength={2000}
          value={note}
          onChange={(event) => setNote(event.target.value)}
        />
      </label>
      {error && <p className="form-error" role="alert">{error}</p>}
      <div className="form-actions">
        <button className="primary-action" type="submit" disabled={state === 'saving'}>
          {state === 'saving' ? 'Wird gespeichert' : 'Antwort speichern'}
        </button>
        <button className="text-button" type="button" onClick={() => setOpen(false)}>Abbrechen</button>
      </div>
    </form>
  );
}

function TodayView({
  hub,
  answers,
  items,
  onOpenDecision,
  onNavigate,
  onSaved,
}: {
  hub: HubData;
  answers: Record<string, AnswerRecord>;
  items: AddedItem[];
  onOpenDecision: (packageId: string, questionIndex?: number) => void;
  onNavigate: (view: Exclude<View, 'decision'>) => void;
  onSaved: (id: string, answer: AnswerRecord) => void;
}) {
  const focus = decisionPackages[0];
  const progress = packageProgress(focus, answers);
  const open = progress.total - progress.answered;
  const nextOpen = Math.max(0, focus.questionIds.findIndex((id) => !answers[id] || answers[id].choice === 'Später'));
  const tasks = hub.bei_dir
    .map((item) => ({ id: item.id, ...taskCopy[item.id] }))
    .filter((item) => item.title)
    .slice(0, 4);

  return (
    <>
      <section className="intro" aria-labelledby="page-title">
        <div>
          <p className="date-line">Stand {germanDate(hub.stand)}</p>
          <h1 id="page-title">Aktuelles Briefing</h1>
          <p className="intro-copy">
            Für den nächsten Designschritt ist zuerst das Gesamturteil zu Nakama Gen offen.
            Danach folgen die sichtbaren Details einzeln.
          </p>
        </div>
        <div className="truth-note">
          <strong>Der Stand ist aktuell.</strong>
          <span>Figma ist die verbindliche Designquelle. Dunkel gilt, hell ist geplant.</span>
        </div>
      </section>

      <section className="focus-card" aria-labelledby="focus-title">
        <div className="focus-copy">
          <div className="focus-meta">
            <span className="status-label">Jetzt entscheiden</span>
            <span>{open} offen, {progress.total} insgesamt</span>
          </div>
          <h2 id="focus-title">Passt Gen als Ganzes?</h2>
          <p>
            Schau zuerst nur auf Material, Anordnung, Dichte und Farbwelt.
            Kleine Details müssen dich an dieser Stelle noch nicht aufhalten.
          </p>
          <div className="reason">
            <strong>Warum jetzt?</strong>
            <span>Das Gesamturteil gibt die Richtung für den nächsten Designschritt frei.</span>
          </div>
          <button className="primary-action" type="button" onClick={() => onOpenDecision(focus.id, nextOpen)}>
            {progress.answered ? 'Bei der nächsten Frage weiter' : 'Entscheidung öffnen'}
          </button>
        </div>

        <figure className="focus-visual">
          <img src={focus.image} alt={focus.imageAlt} />
          <figcaption>Aktueller Figma-Stand von Nakama Gen</figcaption>
        </figure>
      </section>

      <section className="lower-grid">
        <article className="next-work" aria-labelledby="next-work-title">
          <div className="section-heading">
            <h2 id="next-work-title">Was parallel passiert</h2>
            <span>Ohne etwas von dir zu brauchen</span>
          </div>
          <div className="work-list">
            <div className="work-row">
              <span className="work-label">Technik</span>
              <div>
                <h3>Erste große Prüfrunde</h3>
                <p>Der bisherige Kern wird unabhängig geprüft. Danach folgt der gemeinsame Kern der drei Plugins.</p>
              </div>
            </div>
            <div className="work-row">
              <span className="work-label">Design</span>
              <div>
                <h3>Gen wird als Erstes übersetzt</h3>
                <p>Der dunkle Figma-Stand liegt bereit. Änderungen aus deinen Antworten fließen direkt ein.</p>
              </div>
            </div>
            <div className="work-row">
              <span className="work-label">Danach</span>
              <div>
                <h3>Der gemeinsame Plugin-Kern</h3>
                <p>Gen, Probeeq und Suna teilen sich dieselbe sichere Grundlage, behalten aber ihre eigene Identität.</p>
              </div>
            </div>
          </div>
        </article>

        <aside className="after-list" aria-labelledby="after-title">
          <div className="section-heading">
            <h2 id="after-title">Wenn du Zeit hast</h2>
            <span>Nichts davon blockiert heute</span>
          </div>
          <ol>
            {tasks.map((item) => (
              <li key={item.id}>
                <div>
                  <strong>{item.title}</strong>
                  <span>{item.timing}</span>
                </div>
              </li>
            ))}
          </ol>
          <button className="text-link" type="button" onClick={() => onNavigate('decisions')}>
            Alle Entscheidungen ansehen
          </button>
        </aside>
      </section>

      {items.length > 0 && (
        <section className="new-items" aria-labelledby="new-items-title">
          <div className="section-heading">
            <h2 id="new-items-title">Neu im Briefing</h2>
            <span>Von Phil, Claude und Codex</span>
          </div>
          <div className="new-item-list">
            {items.slice(0, 5).map((item) => (
              <article key={item.id}>
                <div className="item-meta">
                  <span>{item.kind}</span>
                  <span>{item.author}</span>
                </div>
                <h3>{cleanText(item.title)}</h3>
                <p>{cleanText(item.summary)}</p>
                {item.kind === 'Entscheidung' && (
                  <AddedDecisionAnswer item={item} saved={answers[`item.${item.id}`]} onSaved={onSaved} />
                )}
              </article>
            ))}
          </div>
        </section>
      )}
    </>
  );
}

function DecisionsView({
  answers,
  items,
  onOpenDecision,
  onSaved,
}: {
  answers: Record<string, AnswerRecord>;
  items: AddedItem[];
  onOpenDecision: (packageId: string, questionIndex?: number) => void;
  onSaved: (id: string, answer: AnswerRecord) => void;
}) {
  const addedDecisions = items.filter((item) => item.kind === 'Entscheidung');
  const packageQuestionTotal = decisionPackages.reduce((sum, decision) => sum + decision.questionIds.length, 0);
  const packageAnswered = decisionPackages.reduce((sum, decision) => sum + packageProgress(decision, answers).answered, 0);
  const addedAnswered = addedDecisions.filter((item) => !answerIsOpen(answers[`item.${item.id}`])).length;
  const totalQuestions = packageQuestionTotal + addedDecisions.length;
  const answeredQuestions = packageAnswered + addedAnswered;
  const openQuestions = totalQuestions - answeredQuestions;

  return (
    <section className="view-section" aria-labelledby="decisions-title">
      <div className="view-heading">
        <p className="date-line">Deine Entscheidungen</p>
        <h1 id="decisions-title">Offene Entscheidungen</h1>
        <p className="view-description">Alle Einzelfragen bleiben sichtbar. Beim Antworten öffnet sich jeweils nur eine, damit die Seite übersichtlich bleibt.</p>
        <p className="decision-count">
          <strong>{openQuestions} offene Einzelfragen</strong>
          <span>{totalQuestions} insgesamt, gebündelt in {decisionPackages.length} Themen</span>
        </p>
      </div>

      <div className="decision-list">
        {decisionPackages.map((decision) => {
          const progress = packageProgress(decision, answers);
          const open = progress.total - progress.answered;
          const nextOpen = Math.max(0, decision.questionIds.findIndex((id) => !answers[id] || answers[id].choice === 'Später'));
          const complete = progress.answered === progress.total;
          return (
            <article className={`decision-row${decision.id === 'U2' ? ' is-primary' : ''}`} key={decision.id}>
              {decision.image && (
                <img src={decision.image} alt={decision.imageAlt} />
              )}
              <div className="decision-row-copy">
                <div className="item-meta">
                  <span>{complete ? 'Beantwortet' : decision.timing}</span>
                  <span>{open} offen, {progress.total} insgesamt</span>
                </div>
                <h2>{decision.title}</h2>
                <p>{decision.summary}</p>
                <details className="question-index">
                  <summary>
                    {progress.total === 1 ? 'Einzelfrage anzeigen' : `Alle ${progress.total} Einzelfragen anzeigen`}
                  </summary>
                  <ol>
                    {decision.questionIds.map((questionId, index) => {
                      const answered = !answerIsOpen(answers[questionId]);
                      return (
                        <li key={questionId}>
                          <button type="button" onClick={() => onOpenDecision(decision.id, index)}>
                            <span>{friendlyQuestions[questionId].title}</span>
                            <small>{answered ? 'Beantwortet' : 'Offen'}</small>
                          </button>
                        </li>
                      );
                    })}
                  </ol>
                </details>
              </div>
              <button className={decision.id === 'U2' ? 'primary-action' : 'secondary-action'} type="button" onClick={() => onOpenDecision(decision.id, nextOpen)}>
                {complete ? 'Antworten prüfen' : progress.answered ? 'Weiter beantworten' : 'Öffnen'}
              </button>
            </article>
          );
        })}
      </div>

      {addedDecisions.length > 0 && (
        <section className="added-decisions" aria-labelledby="added-decisions-title">
          <div className="section-heading">
            <h2 id="added-decisions-title">Neu hinzugefügt</h2>
            <span>Von Phil, Claude und Codex</span>
          </div>
          <div className="new-item-list">
            {addedDecisions.map((item) => (
              <article key={item.id}>
                <div className="item-meta">
                  <span>{item.urgency}</span>
                  <span>{item.author}</span>
                </div>
                <h3>{cleanText(item.title)}</h3>
                <p>{cleanText(item.summary)}</p>
                <AddedDecisionAnswer item={item} saved={answers[`item.${item.id}`]} onSaved={onSaved} />
              </article>
            ))}
          </div>
        </section>
      )}
    </section>
  );
}

function DecisionDetail({
  decision,
  index,
  answers,
  onIndexChange,
  onSaved,
  onBack,
}: {
  decision: DecisionPackage;
  index: number;
  answers: Record<string, AnswerRecord>;
  onIndexChange: (index: number) => void;
  onSaved: (id: string, answer: AnswerRecord) => void;
  onBack: () => void;
}) {
  const questionId = decision.questionIds[index];
  const question = friendlyQuestions[questionId];
  const saved = answers[questionId];
  const [choice, setChoice] = useState(saved?.choice ?? '');
  const [note, setNote] = useState(saved?.note ?? '');
  const [state, setState] = useState<'idle' | 'saving' | 'error'>('idle');
  const [error, setError] = useState('');

  useEffect(() => {
    setChoice(saved?.choice ?? '');
    setNote(saved?.note ?? '');
    setState('idle');
    setError('');
  }, [questionId, saved?.choice, saved?.note]);

  const choices = [
    ...question.options,
    { value: 'Eigene Antwort', label: 'Anders', text: 'Beschreibe deine Lösung kurz im Textfeld.' },
    { value: 'Später', label: 'Später', text: 'Die Frage bleibt offen und kommt wieder.' },
  ];

  async function save(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();
    setError('');
    if (!choice) {
      setError('Bitte wähle eine Antwort.');
      return;
    }
    if (choice === 'Eigene Antwort' && note.trim().length < 3) {
      setError('Schreibe bitte kurz dazu, wie du es stattdessen möchtest.');
      return;
    }

    setState('saving');
    try {
      const response = await fetch('/api/answers', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ questionId, choice, note, author: 'Phil' }),
      });
      const body = await response.json() as AnswerRecord & { error?: string };
      if (!response.ok) throw new Error(body.error ?? 'Speichern fehlgeschlagen.');
      onSaved(questionId, body);

      if (index < decision.questionIds.length - 1) {
        onIndexChange(index + 1);
      } else {
        onBack();
      }
    } catch (caught) {
      setState('error');
      setError(caught instanceof Error ? caught.message : 'Speichern fehlgeschlagen.');
    }
  }

  return (
    <section className="decision-detail" aria-labelledby="question-title">
      <div className="decision-toolbar">
        <button className="text-button" type="button" onClick={onBack}>Zurück zu allen Entscheidungen</button>
        <span>{index + 1} von {decision.questionIds.length}</span>
      </div>

      <div className="question-layout">
        <form className="question-form" onSubmit={save}>
          <div className="question-heading">
            <span>{decision.title}</span>
            <h1 id="question-title">{question.title}</h1>
            <p>{question.prompt}</p>
          </div>

          <fieldset>
            <legend>Deine Antwort</legend>
            <div className={`answer-grid${question.options.length > 2 ? ' has-three' : ''}`}>
              {choices.map((item) => (
                <label className={`answer-option${choice === item.value ? ' is-selected' : ''}`} key={item.value}>
                  <input
                    type="radio"
                    name={`answer-${questionId}`}
                    value={item.value}
                    checked={choice === item.value}
                    onChange={() => setChoice(item.value)}
                  />
                  <span>
                    <strong>{item.label}</strong>
                    <small>{item.text}</small>
                  </span>
                </label>
              ))}
            </div>
          </fieldset>

          <label className="note-field">
            Dein Satz dazu <span className="optional">Optional, außer bei „Anders“</span>
            <textarea
              rows={4}
              maxLength={2000}
              value={note}
              onChange={(event) => setNote(event.target.value)}
              placeholder="Was soll Claude und Codex dazu wissen?"
            />
          </label>

          {error && <p className="form-error" role="alert">{error}</p>}

          <div className="question-actions">
            <button className="primary-action" type="submit" disabled={state === 'saving'}>
              {state === 'saving'
                ? 'Wird gespeichert'
                : index < decision.questionIds.length - 1
                  ? 'Speichern und weiter'
                  : 'Antwort speichern'}
            </button>
            {index > 0 && (
              <button className="secondary-action" type="button" onClick={() => onIndexChange(index - 1)}>
                Vorherige Frage
              </button>
            )}
          </div>
        </form>

        <aside className="question-context">
          {decision.image && <img src={decision.image} alt={decision.imageAlt} />}
          <div>
            <strong>Du musst nicht alles auf einmal lösen.</strong>
            <p>Jede gespeicherte Antwort bleibt erhalten. „Später“ lässt die Frage bewusst offen.</p>
          </div>
        </aside>
      </div>
    </section>
  );
}

function StatusView({ hub }: { hub: HubData }) {
  const rows = hub.plan.flatMap((phase) => phase.zeilen);
  const totals = {
    done: rows.filter((row) => row.status === 'erledigt').length,
    built: rows.filter((row) => row.status === 'gebaut').length,
    next: rows.filter((row) => row.status === 'naechster').length,
    open: rows.filter((row) => row.status === 'offen').length,
  };

  return (
    <section className="view-section" aria-labelledby="status-title">
      <div className="view-heading status-heading">
        <div>
          <p className="date-line">Aktueller Bauzustand</p>
          <h1 id="status-title">Stand der Umsetzung</h1>
          <p>Die Hostgrenzen sind gemessen, alte Projekte sind abgesichert. Jetzt folgt die unabhängige Prüfung dieses Fundaments.</p>
        </div>
        <div className="next-step-note">
          <strong>Als Nächstes</strong>
          <span>Eine frische Prüfrunde über den bisherigen Kern. Danach beginnt der gemeinsame Plugin-Kern.</span>
        </div>
      </div>

      <div className="status-numbers" aria-label="Fortschritt in Zahlen">
        <div><strong>{totals.done}</strong><span>abgeschlossen</span></div>
        <div><strong>{totals.built}</strong><span>gebaut, Prüfung offen</span></div>
        <div><strong>{totals.open}</strong><span>später offen</span></div>
      </div>

      <div className="phase-list">
        {hub.plan.map((phase) => {
          const copy = phaseCopy[phase.phase] ?? { title: cleanText(phase.titel), summary: cleanText(phase.ziel) };
          const done = phase.zeilen.filter((row) => row.status === 'erledigt').length;
          const built = phase.zeilen.filter((row) => row.status === 'gebaut').length;
          const current = phase.zeilen.some((row) => row.status === 'naechster');
          const complete = done === phase.zeilen.length;
          const state = complete ? 'Abgeschlossen' : current ? 'Jetzt' : done || built ? 'In Vorbereitung' : 'Später';

          return (
            <article className={`phase-row${current ? ' is-current' : ''}`} key={phase.phase}>
              <div className="phase-code">{cleanText(phase.phase)}</div>
              <div>
                <h2>{copy.title}</h2>
                <p>{copy.summary}</p>
              </div>
              <div className="phase-state">
                <strong>{state}</strong>
                <span>{done + built} von {phase.zeilen.length} gebaut</span>
              </div>
            </article>
          );
        })}
      </div>

      <p className="agent-note">Claude und Codex sehen die vollständigen technischen Zeilen weiterhin in der Maschinenansicht.</p>
    </section>
  );
}

function DesignView({ hub }: { hub: HubData }) {
  const [gen, probeeq, suna] = hub.design.apps;

  function DesignCard({ app, large = false }: { app: DesignApp; large?: boolean }) {
    const presentation = designPresentation[app.key];
    return (
      <article className={`design-card${large ? ' is-large' : ''}`}>
        <figure>
          <img src={presentation.image} alt={`Aktueller dunkler Entwurf von ${app.name}`} />
        </figure>
        <div className="design-copy">
          <div className="item-meta">
            <span>{app.status}</span>
            <span>{cleanText(app.groesse)}</span>
          </div>
          <h2>{app.name}</h2>
          <p>{presentation.summary}</p>
          <div className="design-open">
            <strong>Noch offen</strong>
            <span>{cleanText(app.offen).replace(/\s*·\s*/g, ', ')}</span>
          </div>
        </div>
      </article>
    );
  }

  return (
    <section className="view-section" aria-labelledby="design-title">
      <div className="view-heading design-heading">
        <div>
          <p className="date-line">Verbindlicher Figma-Stand</p>
          <h1 id="design-title">Aktueller Stand der drei Apps</h1>
          <p>Dunkel ist heute verbindlich. Eine helle Fassung ist für jede App geplant, aber noch nicht abgenommen.</p>
        </div>
        <a className="secondary-action" href={hub.design.quelle.url} target="_blank" rel="noreferrer">
          Figma-Depot öffnen
        </a>
      </div>

      <div className="design-layout">
        <DesignCard app={gen} large />
        <div className="design-pair">
          <DesignCard app={probeeq} />
          <DesignCard app={suna} />
        </div>
      </div>
    </section>
  );
}

function Footer() {
  return (
    <footer className="site-footer">
      <span>Eine Seite für Phil, Claude und Codex.</span>
      <a href="/api/hub" target="_blank" rel="noreferrer">Maschinenansicht für Claude und Codex</a>
    </footer>
  );
}

export default function BriefingApp({ initialHub }: BriefingAppProps) {
  const [hub, setHub] = useState(initialHub);
  const [answers, setAnswers] = useState<Record<string, AnswerRecord>>(() => initialAnswerMap(initialHub));
  const [items, setItems] = useState<AddedItem[]>([]);
  const [view, setView] = useState<View>('today');
  const [selectedPackageId, setSelectedPackageId] = useState('U2');
  const [questionIndex, setQuestionIndex] = useState(0);
  const [composerOpen, setComposerOpen] = useState(false);
  const [announcement, setAnnouncement] = useState('');

  const selectedPackage = useMemo(
    () => decisionPackages.find((item) => item.id === selectedPackageId) ?? decisionPackages[0],
    [selectedPackageId],
  );

  useEffect(() => {
    let active = true;
    fetch('/api/hub', { cache: 'no-store' })
      .then((response) => response.json())
      .then((body: { hub?: HubData; answers?: Record<string, AnswerRecord>; items?: AddedItem[] }) => {
        if (!active) return;
        if (body.hub) setHub(body.hub);
        if (body.answers) setAnswers((current) => ({ ...current, ...body.answers }));
        if (body.items) setItems(body.items);
      })
      .catch(() => {
        if (active) setAnnouncement('Der letzte bekannte Stand ist geöffnet. Neue Antworten können gerade nicht geladen werden.');
      });
    return () => {
      active = false;
    };
  }, []);

  function navigate(next: Exclude<View, 'decision'>) {
    setView(next);
    setComposerOpen(false);
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }

  function openDecision(packageId: string, index = 0) {
    const decision = decisionPackages.find((item) => item.id === packageId) ?? decisionPackages[0];
    setSelectedPackageId(decision.id);
    setQuestionIndex(Math.min(Math.max(index, 0), decision.questionIds.length - 1));
    setComposerOpen(false);
    setView('decision');
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }

  function saveAnswer(id: string, answer: AnswerRecord) {
    setAnswers((current) => ({ ...current, [id]: answer }));
    setAnnouncement('Antwort gespeichert. Claude und Codex können sie jetzt aufgreifen.');
  }

  function addItem(item: AddedItem) {
    setItems((current) => [item, ...current]);
    setAnnouncement('Der neue Punkt liegt im gemeinsamen Briefing.');
  }

  return (
    <main className="site-shell">
      <Topbar
        view={view}
        answers={answers}
        items={items}
        onNavigate={navigate}
        onNewItem={() => setComposerOpen(true)}
      />

      <div className="page-frame">
        {announcement && (
          <div className="announcement" role="status">
            <span>{announcement}</span>
            <button type="button" onClick={() => setAnnouncement('')} aria-label="Hinweis schließen">Schließen</button>
          </div>
        )}

        {composerOpen && <Composer onClose={() => setComposerOpen(false)} onCreated={addItem} />}

        {view === 'today' && (
          <TodayView
            hub={hub}
            answers={answers}
            items={items}
            onOpenDecision={openDecision}
            onNavigate={navigate}
            onSaved={saveAnswer}
          />
        )}
        {view === 'decisions' && (
          <DecisionsView answers={answers} items={items} onOpenDecision={openDecision} onSaved={saveAnswer} />
        )}
        {view === 'decision' && (
          <DecisionDetail
            decision={selectedPackage}
            index={questionIndex}
            answers={answers}
            onIndexChange={setQuestionIndex}
            onSaved={saveAnswer}
            onBack={() => navigate('decisions')}
          />
        )}
        {view === 'status' && <StatusView hub={hub} />}
        {view === 'design' && <DesignView hub={hub} />}

        <Footer />
      </div>
    </main>
  );
}
