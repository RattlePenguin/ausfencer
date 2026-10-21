export const view = document.querySelector('#view');
export function el(tag, text = '', attributes = {}) {
  const node = document.createElement(tag);
  node.textContent = text;
  for (const [key, value] of Object.entries(attributes)) node.setAttribute(key, value);
  return node;
}
export const link = (text, href) => el('a', text, {href});
export function button(text, action) {
  const node = el('button', text, {type: 'button'});
  node.addEventListener('click', () => run(action));
  return node;
}
export async function api(path, options = {}) {
  const headers = {'Content-Type': 'application/json', ...options.headers};
  const token = sessionStorage.getItem('demo-session');
  if (token) headers.Authorization = `Bearer ${token}`;
  const response = await fetch(`/api/${path}`, {...options, headers, cache: 'no-store'});
  if (!response.ok) {
    const error = new Error(await response.text()); error.status = response.status; throw error;
  }
  return response.json();
}
export async function run(action) {
  const message = document.querySelector('#message');
  message.textContent = '';
  try { await action(); }
  catch (error) { message.textContent = error.message; message.className = 'error'; }
}
export function table(headers, rows) {
  const scroll = el('div', '', {class: 'scroll'}), table = el('table');
  const head = el('tr');
  headers.forEach(title => head.append(el('th', title, {scope: 'col'})));
  const thead = el('thead'); thead.append(head); table.append(thead);
  const body = el('tbody');
  for (const row of rows) {
    const tr = el('tr');
    for (const value of row) { const td = el('td'); td.append(value instanceof Node ? value : document.createTextNode(value ?? '—')); tr.append(td); }
    body.append(tr);
  }
  table.append(body); scroll.append(table); return scroll;
}
