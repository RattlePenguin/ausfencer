import {view, el, link, api, table} from './ui.js';
import {person} from './profiles.js';

const entrant = entry => entry.id ? link(entry.name, `#fencer/${entry.id}`) : el('span', 'Private fencer');
function poolTotals(entry, bouts) {
  let wins = 0, matches = 0, scored = 0, received = 0;
  for (const bout of bouts) {
    if (bout.disputed === '1' || bout.status !== 'completed') continue;
    const side = bout.left_seed === entry.seed ? 'left' : bout.right_seed === entry.seed ? 'right' : null;
    if (!side) continue;
    matches++; if (bout.winner_side === side) wins++;
    scored += Number(bout[`${side}_score`] || 0);
    received += Number(bout[`${side === 'left' ? 'right' : 'left'}_score`] || 0);
  }
  return {wins, matches, scored, received, indicator: scored - received};
}

export async function eventView(id) {
  const data = await api(`tracker/events/${encodeURIComponent(id)}`), event = data.events[0];
  view.replaceChildren(el('h1', event.name), el('p', `${event.date} · ${event.weapon} · ${event.category} · ${event.location}`),
    el('p', `Fictional demo · Coverage: ${event.coverage} · ${event.timezone}`, {class: 'muted'}));
  view.append(el('h2', 'Entrants'), table(['Fencer', 'Source-listed name', 'Club', 'State / country', 'Poule'],
    data.entries.map(entry => [entrant(entry), entry.source_name || '—', entry.club || '—', `${entry.state} ${entry.country}`, entry.pool || '—'])));
  const poolBouts = data.bouts.filter(b => b.round === 'pool');
  for (const pool of [...new Set(data.entries.map(e => e.pool).filter(Boolean))].sort()) {
    const entries = data.entries.filter(e => e.pool === pool);
    view.append(el('h2', `Poule ${pool}`));
    const rows = entries.map(entry => {
      const cells = entries.map(opponent => {
        if (entry.seed === opponent.seed) return '×';
        const b = poolBouts.find(b => (b.left_seed === entry.seed && b.right_seed === opponent.seed) ||
          (b.right_seed === entry.seed && b.left_seed === opponent.seed));
        if (!b) return '—';
        const side = b.left_seed === entry.seed ? 'left' : 'right';
        return `${b.disputed === '1' ? '?' : b.winner_side === side ? 'V' : 'D'}${b[`${side}_score`] ?? '—'}`;
      });
      const total = poolTotals(entry, poolBouts);
      return [entrant(entry), ...cells, total.wins, total.matches, total.indicator, total.scored];
    });
    view.append(table(['Fencer', ...entries.map((_, index) => String(index + 1)), 'V', 'M', 'Indicator', 'Hits scored'], rows));
  }
  if (poolBouts.length) {
    const ranking = data.entries.map(entry => ({entry, ...poolTotals(entry, poolBouts)}));
    ranking.sort((a, b) => (b.wins / (b.matches || 1) - a.wins / (a.matches || 1)) ||
      b.indicator - a.indicator || b.scored - a.scored || Number(a.entry.seed) - Number(b.entry.seed));
    view.append(el('h2', 'Post-poule rankings'), table(['Rank', 'Fencer', 'V/M', 'Indicator', 'Hits scored', 'Recorded seed'],
      ranking.map((r, index) => [index + 1, entrant(r.entry), `${r.wins}/${r.matches}`, r.indicator, r.scored, r.entry.seed])));
  }
  view.append(el('h2', 'Direct elimination'));
  const byes = data.entries.filter(e => e.bye === '1');
  if (byes.length) {
    const line = el('p', 'Table of 16 byes: ');
    byes.forEach((entry, index) => { if (index) line.append(', '); line.append(entrant(entry)); });
    view.append(line);
  }
  for (const [stage, title] of [['R16', 'Table of 16'], ['QF', 'Quarter-finals'], ['SF', 'Semi-finals'], ['F', 'Final']]) {
    const bouts = data.bouts.filter(b => b.round === stage);
    if (!bouts.length) continue;
    view.append(el('h3', title), table(['Left', 'Score', 'Right', 'Winner', 'Record'], bouts.map(b => [person(b, 'left'),
      `${b.left_score ?? '—'}–${b.right_score ?? '—'}`, person(b, 'right'), b.winner_side ? person(b, b.winner_side) : '—', link('Details', `#bout/${b.id}`)])));
  }
  const placing = [...data.entries].sort((a, b) => Number(a.place) - Number(b.place) || Number(a.seed) - Number(b.seed));
  view.append(el('h2', 'Final placings'), table(['Place', 'Fencer', 'Club'], placing.map(e => [e.place, entrant(e), e.club || '—'])));
  view.append(el('p', 'Byes are administrative advances. Shared third places have no bronze bout. Matrices and totals use the same canonical bouts as profiles.', {class: 'muted'}));
}
