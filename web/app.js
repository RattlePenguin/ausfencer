import {view, el, link, button, api, run} from './ui.js';

async function search(query = '', page = 1) {
  const data = await api(`tracker/fencers?${new URLSearchParams({q: query, page, limit: 12})}`);
  view.replaceChildren(el('h1', 'Every bout has a story.'), el('p', 'Find fictional competitors and explore their recorded results.', {class: 'muted'}));
  const form = el('form'), label = el('label', 'Fencer name or alias');
  const input = el('input', '', {type: 'search', name: 'q', placeholder: 'Try Mira Vale or Esmé', 'aria-label': 'Fencer name or alias'});
  input.value = query; label.append(input); form.append(label, el('button', 'Search', {type: 'submit'}));
  form.addEventListener('submit', event => { event.preventDefault(); run(() => search(input.value)); });
  const cards = el('section', '', {class: 'cards', 'aria-label': 'Search results'});
  for (const person of data.fencers) {
    const card = el('article', '', {class: 'card'});
    card.append(link(person.name, `#fencer/${person.id}`), el('p', `${person.club} · ${person.state} · ${person.country}`));
    cards.append(card);
  }
  if (!data.fencers.length) cards.append(el('p', 'No matching public profiles.'));
  const previous = button('Previous', () => search(query, page - 1)); previous.disabled = page === 1;
  const next = button('Next', () => search(query, page + 1)); next.disabled = data.fencers.length < 12;
  const paging = el('div', '', {class: 'filters'}); paging.append(previous, el('span', `Page ${page}`), next);
  view.append(form, cards, paging);
}

async function route() {
  const [kind, rawId] = location.hash.slice(1).split('/');
  const id = rawId ? decodeURIComponent(rawId) : '';
  view.replaceChildren(el('p', 'Loading…'));
  if (kind === 'events') {
    const data = await api('tracker/events'); view.replaceChildren(el('h1', 'Demo events'));
    for (const event of data.events) {
      const row = el('p');
      row.append(link(`${event.date} · ${event.name} · ${event.weapon} · ${event.coverage}`, `#event/${encodeURIComponent(event.id)}`));
      view.append(row);
    }
  } else if (kind === 'fencer') await (await import('./profiles.js')).profile(id);
  else if (kind === 'event') await (await import('./events.js')).eventView(id);
  else if (kind === 'bout') await (await import('./profiles.js')).boutView(id);
  else if (kind === 'account') await (await import('./account.js')).account();
  else await search();
  view.focus({preventScroll: true});
}
window.addEventListener('hashchange', () => run(route));
run(route);
