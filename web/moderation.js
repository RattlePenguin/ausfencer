import {view, el, link, button, api, table} from './ui.js';
import {act, actionForm} from './forms.js';

export function moderation(data, refresh) {
  view.append(el('h2', 'Profile and privacy review'));
  for (const item of data.profile_queue) {
    const card = el('article', '', {class: 'card'});
    card.append(el('h3', `${item.type} · ${item.kind} · Profile ${item.fencer_id}`), el('p', item.evidence));
    for (const approved of [true, false]) card.append(button(approved ? 'Approve' : 'Reject', async () => {
      await act(item.type === 'claim' ? 'review-claim' : 'review-privacy', {id: Number(item.id), approved}); await refresh();
    }));
    view.append(card);
  }
  view.append(el('h2', 'Result review'), table(['Record', 'Problem', 'Evidence', 'Review'], data.result_queue.map(item => [
    link('Bout', `#bout/${item.bout}`), item.kind, item.detail, button('Review current record', async () => {
      const [bout] = await api(`tracker/bouts/${item.bout}`);
      if (item.kind === 'identity') {
        const data = await api(`tracker/events/${encodeURIComponent(bout.event_id)}`);
        const choices = data.entries.filter(e => e.id).map(e => [e.id, `${e.name} · ${e.club}`]);
        const panel = actionForm('Save reviewed identity correction', [
          ['side', 'Incorrect participant', [['left', bout.left_name], ['right', bout.right_name]]],
          ['fencer', 'Correct event entrant', choices], ['reason', 'Identity evidence and reason']
        ], async values => {
          await act('correct-identity', {...values, fencer: Number(values.fencer), bout: Number(bout.id), revision: Number(bout.revision)});
          await refresh();
        }, true);
        view.append(panel); panel.scrollIntoView({block: 'center'}); return;
      }
      const panel = actionForm('Save reviewed correction', [
        ['left_score', `${bout.left_name} score`, 'number', bout.left_score ?? '', false],
        ['right_score', `${bout.right_name} score`, 'number', bout.right_score ?? '', false],
        ['winner_side', 'Winner', [['left', bout.left_name], ['right', bout.right_name], ['none', 'No winner']], bout.winner_side || 'none'],
        ['status', 'Outcome', [['completed', 'Completed'], ['withdrawal', 'Withdrawal'], ['administrative', 'Administrative'], ['unfinished', 'Unfinished']], bout.status],
        ['reason', 'Review evidence and reason']
      ], async values => {
        await act('correct', {...values, bout: Number(bout.id), revision: Number(bout.revision)}); await refresh();
      }, true);
      view.append(panel); panel.scrollIntoView({block: 'center'});
    })
  ])));
}
