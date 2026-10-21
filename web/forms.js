import {el, api, run} from './ui.js';

export const act = (name, data) => api(`actions/${name}`, {method: 'POST', body: JSON.stringify(data)});

export function actionForm(title, fields, action, open = false) {
  const panel = el('details'); panel.open = open;
  panel.append(el('summary', title));
  const form = el('form');
  for (const field of fields) {
    const [name, title, type = 'text', initial = ''] = field;
    const label = el('label', title);
    const control = Array.isArray(type) ? el('select', '', {name}) : el('input', '', {name, type});
    if (Array.isArray(type)) type.forEach(([value, text]) => control.append(el('option', text, {value})));
    control.value = initial; control.required = true; label.append(control); form.append(label);
  }
  const submit = el('button', title, {type: 'submit'});
  form.append(submit);
  form.addEventListener('submit', event => {
    event.preventDefault();
    run(async () => {
      submit.disabled = true;
      try {
        const data = Object.fromEntries(new FormData(form));
        fields.forEach(([name, , type]) => { if (type === 'number') data[name] = Number(data[name]); });
        await action(data);
        document.querySelector('#message').textContent = 'Saved.';
      } finally { submit.disabled = false; }
    });
  });
  panel.append(form); return panel;
}
