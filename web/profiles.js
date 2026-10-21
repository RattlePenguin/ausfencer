import {view, el, link, api, run, table} from './ui.js';

const percentage = value => value == null ? '—' : `${(100 * value).toFixed(1)}%`;
export const person = (bout, side) => bout[`${side}_id`]
  ? link(bout[`${side}_name`], `#fencer/${bout[`${side}_id`]}`) : el('span', 'Private fencer');

export async function profile(id, values = {}) {
  const data = await api(`tracker/fencers/${encodeURIComponent(id)}?${new URLSearchParams(values)}`);
  const peers = await api('tracker/fencers?limit=100');
  const p = data.profile[0], stats = data.statistics;
  view.replaceChildren(el('h1', p.name), el('p', `${p.club} · ${p.state} · ${p.country}`, {class: 'muted'}));
  const filters = el('form', '', {'aria-label': 'History filters'});
  const choices = {
    weapon: [['', 'All weapons'], ['epee', 'Épée'], ['foil', 'Foil'], ['sabre', 'Sabre']],
    stage: [['', 'All rounds'], ['pool', 'Poule'], ['de', 'Direct elimination']],
    coverage: [['', 'All coverage'], ['complete', 'Complete bouts'], ['partial', 'Partial bouts'], ['results-only', 'Placings only'], ['unresolved', 'Unresolved entrants']],
    trust: [['', 'All trust labels'], ['synthetic', 'Synthetic fixture'], ['community', 'Community submitted'], ['organiser', 'Organiser supplied']],
    opponent: [['', 'All opponents'], ...peers.fencers.filter(other => other.id !== id).map(other => [other.id, `${other.name} · ${other.club}`])]
  };
  for (const [name, options] of Object.entries(choices)) {
    const label = el('label', name[0].toUpperCase() + name.slice(1)), select = el('select', '', {name});
    options.forEach(([value, text]) => select.append(el('option', text, {value})));
    select.value = values[name] || ''; label.append(select); filters.append(label);
  }
  for (const [name, title, type] of [['year', 'Calendar year', 'number'], ['category', 'Category', 'text'], ['from', 'From date', 'date'], ['to', 'To date', 'date']]) {
    const label = el('label', title), input = el('input', '', {name, type});
    input.value = values[name] || ''; label.append(input); filters.append(label);
  }
  filters.append(el('button', 'Apply filters', {type: 'submit'}));
  filters.addEventListener('submit', event => { event.preventDefault(); run(() => profile(id, Object.fromEntries(new FormData(filters)))); });
  const totals = el('section', '', {class: 'stats', 'aria-label': 'Recorded totals'});
  for (const [title, value] of [['Wins / losses', `${stats.wins} / ${stats.losses}`], ['Win rate', percentage(stats.win_rate)],
    ['Touches scored / received', `${stats.scored} / ${stats.received}`], ['Touch differential', stats.differential], ['Touch share', percentage(stats.touch_share)]]) {
    const card = el('article', title); card.append(el('strong', value)); totals.append(card);
  }
  view.append(filters, el('h2', 'Recorded demo totals'), totals,
    el('p', `${stats.wins + stats.losses} contested bouts · ${stats.score_known} with known scores · ${stats.disputed} disputed · ${stats.administrative} administrative · ${stats.unfinished} unfinished`, {class: 'muted'}));
  view.append(el('h2', 'Calendar years'), table(['Year', 'Wins', 'Losses', 'Win rate', 'Scored', 'Received'],
    Object.entries(data.annual || {}).sort().reverse().map(([year, value]) => [year, value.wins, value.losses, percentage(value.win_rate), value.scored, value.received])));
  view.append(el('h2', 'Event placings'), table(['Date', 'Event', 'Place', 'Event-time club', 'Coverage'],
    data.events.map(e => [e.date, link(e.name, `#event/${encodeURIComponent(e.id)}`), e.place, e.club, e.coverage])));
  view.append(el('h2', 'Bout history'), table(['Date', 'Event', 'Opponents', 'Score', 'Result', 'Trust', 'Record'], data.bouts.map(b => {
    const opponents = el('span'); opponents.append(person(b, 'left'), ' / ', person(b, 'right'));
    return [b.date, link(b.event_name, `#event/${encodeURIComponent(b.event_id)}`), opponents,
      `${b.scored ?? '—'}–${b.received ?? '—'}`, b.disputed === '1' ? 'Disputed' : b.status === 'completed' ? b.result : b.status,
      b.trust, link('Details', `#bout/${b.id}`)];
  })));
  view.append(link('Manage a profile or request privacy', '#account'));
}

export async function boutView(id) {
  const [b] = await api(`tracker/bouts/${encodeURIComponent(id)}`);
  view.replaceChildren(el('h1', 'Bout record'));
  const title = el('h2'); title.append(person(b, 'left'), ` ${b.left_score ?? '—'}–${b.right_score ?? '—'} `, person(b, 'right'));
  view.append(title, link(`${b.event_name} · ${b.date}`, `#event/${encodeURIComponent(b.event_id)}`),
    el('p', `${b.weapon} · ${b.round}${b.pool ? ` ${b.pool}` : ''} · ${b.status} · ${b.disputed === '1' ? 'disputed' : 'undisputed'}`),
    el('p', `Provenance: ${b.provenance} · Updated ${b.updated_at} UTC · Revision ${b.revision}`, {class: 'muted'}),
    link('Report a score or identity problem', `#account/report-${b.id}`));
}
