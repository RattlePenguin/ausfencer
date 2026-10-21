import {view, el, link, button, api, table} from './ui.js';
import {act, actionForm} from './forms.js';
import {contributionForms} from './contribute.js';
import {moderation} from './moderation.js';

export async function account() {
  view.replaceChildren(el('h1', 'My demo account'));
  let data;
  if (sessionStorage.getItem('demo-session')) {
    try { data = await api('account'); }
    catch (error) { if (error.status !== 403) throw error; sessionStorage.removeItem('demo-session'); }
  }
  if (!data) {
    view.append(el('p', 'Use the demo session token supplied by your reviewer. Google and email-link sign-in are still being built.'),
      actionForm('Sign in to demo', [['token', 'Demo session token', 'password']], async ({token}) => {
        sessionStorage.setItem('demo-session', token.trim());
        try { await api('account'); } catch (error) { sessionStorage.removeItem('demo-session'); throw error; }
        await account();
      }, true));
  } else {
    view.append(el('p', `${data.account[0].label} · ${data.account[0].role}`), button('Sign out', async () => {
      await act('logout', {}); sessionStorage.removeItem('demo-session'); await account();
    }), el('h2', 'Profile management'));
    for (const claim of data.claims) {
      const card = el('article', '', {class: 'card'});
      card.append(el('h3', claim.name), el('p', `${claim.kind} claim · ${claim.state}`));
      if (claim.state === 'approved') {
        card.append(button(claim.visible === '1' ? 'Hide my profile' : 'Show my profile publicly', async () => {
          await act('visibility', {fencer: Number(claim.fencer_id), visible: claim.visible !== '1'}); await account();
        }), button('Withdraw management and hide', async () => {
          await act('withdraw', {fencer: Number(claim.fencer_id)}); await account();
        }));
      }
      view.append(card);
    }
    view.append(el('h2', 'My contributions'), table(['Record', 'Status', 'Submitted'], data.contributions.map(c => [link('Bout', `#bout/${c.bout}`), c.state, c.created_at])),
      el('h2', 'My reports'), table(['Record', 'Kind', 'Status'], data.reports.map(r => [link('Bout', `#bout/${r.bout}`), r.kind, r.state])));
  }
  if (data?.account[0].role === 'moderator') moderation(data, account);
  await contributionForms(account, Boolean(data));
}
