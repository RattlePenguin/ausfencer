import {view, el, api} from './ui.js';
import {act, actionForm} from './forms.js';

export async function contributionForms(refresh, signedIn) {
  view.append(actionForm('Request privacy without an account', [
    ['fencer', 'Profile number from its link', 'number'], ['contact', 'Private contact address'], ['evidence', 'Request and supporting context']
  ], async data => { await act('privacy', data); await refresh(); }));
  if (!signedIn) return;
  const [people, events] = await Promise.all([api('tracker/fencers?limit=100'), api('tracker/events')]);
  const entrants = people.fencers.map(p => [p.id, `${p.name} · ${p.club} · ${p.state}`]);
  view.append(actionForm('Claim or manage a profile', [
    ['fencer', 'Profile number from its link or reviewer', 'number'],
    ['kind', 'Management role', [['self', 'My own profile'], ['guardian', 'Manage my child’s profile']]],
    ['evidence', 'Evidence of identity or authority'],
    ['declaration', 'I understand public visibility and confirm my identity or guardian authority', 'checkbox']
  ], async data => {
    await act('claim', {...data, consent: 'demo-v1'}); await refresh();
  }));
  const operation = crypto.randomUUID();
  view.append(actionForm('Contribute a fictional result', [
    ['event', 'Competition', events.events.map(e => [e.id, `${e.date} · ${e.name}`])],
    ['left', 'Left fencer', entrants], ['right', 'Right fencer', entrants],
    ['stage', 'Round', [['pool', 'Poule'], ['R16', 'Table of 16'], ['QF', 'Quarter-final'], ['SF', 'Semi-final'], ['F', 'Final']]],
    ['pool', 'Poule letter (leave blank for elimination)', 'text', '', false],
    ['left_score', 'Left score (blank if unknown)', 'number', '', false],
    ['right_score', 'Right score (blank if unknown)', 'number', '', false],
    ['winner_side', 'Winner', [['left', 'Left fencer'], ['right', 'Right fencer']]],
    ['evidence', 'Fictional supporting record (optional)', 'text', '', false],
    ['declaration', 'I am submitting fictional demo results', 'checkbox']
  ], async data => {
    await act('submit', {...data, left: Number(data.left), right: Number(data.right), winner: Number(data[data.winner_side]), operation});
    await refresh();
  }));
  const report = location.hash.match(/report-(\d+)/)?.[1] || '';
  view.append(actionForm('Report a result problem', [
    ['bout', 'Bout number from its record link', 'number', report],
    ['kind', 'Problem', [['score', 'Wrong score or outcome'], ['identity', 'Wrong identity association']]],
    ['detail', 'What needs changing, and supporting evidence']
  ], async data => { await act('report', data); await refresh(); }));
}
