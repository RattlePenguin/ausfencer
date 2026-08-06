let currentBout = null;
let timerInterval = null;
let defaultDurationSeconds = 180;

/**
 * Initializes the app on page load.
 */
document.addEventListener('DOMContentLoaded', () => {
	// Check if there is an active bout stored in localStorage
	const savedLocalBout = localStorage.getItem('activeBout');
	if (savedLocalBout) {
		try {
			currentBout = JSON.parse(savedLocalBout);
			render();
		} catch (e) {
			console.error('Failed to parse cached bout state:', e);
			showNewBoutModal();
		}
	} else {
		showNewBoutModal();
	}
});

/**
 * Renders the screen with current bout data.
 */
function render() {
	if (!currentBout) return;

	document.getElementById('leftName').textContent = currentBout.leftFencerName || 'Left';
	document.getElementById('rightName').textContent = currentBout.rightFencerName || 'Right';
	document.getElementById('leftScore').textContent = currentBout.leftScore ?? 0;
	document.getElementById('rightScore').textContent = currentBout.rightScore ?? 0;

	document.getElementById('leftYellow').textContent = currentBout.leftYellowCards ?? 0;
	document.getElementById('leftRed').textContent = currentBout.leftRedCards ?? 0;
	document.getElementById('rightYellow').textContent = currentBout.rightYellowCards ?? 0;
	document.getElementById('rightRed').textContent = currentBout.rightRedCards ?? 0;

	document.getElementById('boutWeapon').textContent = currentBout.weapon || 'Foil';
	
	const statusElem = document.getElementById('boutStatus');
	statusElem.textContent = currentBout.status || 'NotStarted';
	statusElem.className = 'badge badge-status ' + (currentBout.status || '').toLowerCase().replace(/([a-z])([A-Z])/g, '$1-$2');

	const startBtn = document.getElementById('startStopBtn');
	if (currentBout.timeRunning) {
		startBtn.textContent = 'STOP';
		startBtn.className = 'btn btn-danger btn-lg';
	} else {
		startBtn.textContent = 'START';
		startBtn.className = 'btn btn-primary btn-lg';
	}

	updateTimerDisplay();
	saveLocalState();
}

/**
 * Formats seconds into MM:SS display.
 */
function updateTimerDisplay() {
	if (!currentBout) return;
	const secs = currentBout.timeRemainingSeconds ?? 180;
	const min = Math.floor(secs / 60);
	const sec = secs % 60;
	document.getElementById('timer').textContent = `${min}:${sec.toString().padStart(2, '0')}`;
}

/**
 * Changes score for a side (+1 / -1).
 */
function changeScore(side, delta) {
	if (!currentBout) return;
	if (side === 'left') {
		currentBout.leftScore = Math.max(0, (currentBout.leftScore || 0) + delta);
	} else {
		currentBout.rightScore = Math.max(0, (currentBout.rightScore || 0) + delta);
	}
	render();
	syncToServer();
}

/**
 * Adds a yellow or red card penalty to a fencer.
 */
function addCard(side, cardType) {
	if (!currentBout) return;
	if (side === 'left') {
		if (cardType === 'yellow') currentBout.leftYellowCards = ((currentBout.leftYellowCards || 0) + 1) % 3;
		if (cardType === 'red') {
			currentBout.leftRedCards = (currentBout.leftRedCards || 0) + 1;
			// Red card awards point to opponent
			currentBout.rightScore = (currentBout.rightScore || 0) + 1;
		}
	} else {
		if (cardType === 'yellow') currentBout.rightYellowCards = ((currentBout.rightYellowCards || 0) + 1) % 3;
		if (cardType === 'red') {
			currentBout.rightRedCards = (currentBout.rightRedCards || 0) + 1;
			// Red card awards point to opponent
			currentBout.leftScore = (currentBout.leftScore || 0) + 1;
		}
	}
	render();
	syncToServer();
}

/**
 * Toggles timer start/stop.
 */
function toggleTimer() {
	if (!currentBout) return;
	if (currentBout.timeRemainingSeconds <= 0) return;

	currentBout.timeRunning = !currentBout.timeRunning;

	if (currentBout.timeRunning) {
		currentBout.status = 'InProgress';
		timerInterval = setInterval(() => {
			if (currentBout.timeRemainingSeconds > 0) {
				currentBout.timeRemainingSeconds--;
				updateTimerDisplay();
				saveLocalState();
			} else {
				currentBout.timeRunning = false;
				clearInterval(timerInterval);
				render();
				syncToServer();
			}
		}, 1000);
	} else {
		clearInterval(timerInterval);
	}

	render();
	syncToServer();
}

/**
 * Resets time remaining back to original duration.
 */
function resetTimer() {
	if (!currentBout) return;
	currentBout.timeRemainingSeconds = defaultDurationSeconds;
	if (currentBout.timeRunning) {
		currentBout.timeRunning = false;
		clearInterval(timerInterval);
	}
	render();
	syncToServer();
}

/**
 * Ends and finishes the bout.
 */
async function endBout() {
	if (!currentBout) return;
	if (currentBout.timeRunning) {
		currentBout.timeRunning = false;
		clearInterval(timerInterval);
	}
	currentBout.status = 'Finished';
	currentBout.finishedAt = new Date().toISOString();
	render();
	await syncToServer();
	alert('Bout marked as Finished & Saved!');
}

/**
 * Handles creation of a new bout from modal form.
 */
async function handleCreateBout(event) {
	event.preventDefault();

	const leftName = document.getElementById('inputLeftName').value.trim();
	const rightName = document.getElementById('inputRightName').value.trim();
	const weapon = document.getElementById('inputWeapon').value;
	const durationMins = parseInt(document.getElementById('inputDuration').value, 10) || 3;

	defaultDurationSeconds = durationMins * 60;

	const newBoutData = {
		leftFencerName: leftName,
		rightFencerName: rightName,
		weapon: weapon,
		timeRemainingSeconds: defaultDurationSeconds,
		status: 'NotStarted',
		leftScore: 0,
		rightScore: 0,
		leftYellowCards: 0,
		rightYellowCards: 0,
		leftRedCards: 0,
		rightRedCards: 0
	};

	try {
		const response = await fetch('/api/bouts', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(newBoutData)
		});

		if (response.ok) {
			currentBout = await response.json();
			closeModal('newBoutModal');
			render();
		} else {
			alert('Failed to create bout on server. Working offline.');
			currentBout = { id: crypto.randomUUID(), ...newBoutData };
			closeModal('newBoutModal');
			render();
		}
	} catch (err) {
		console.warn('Network error, creating bout offline:', err);
		currentBout = { id: crypto.randomUUID(), ...newBoutData };
		closeModal('newBoutModal');
		render();
	}
}

/**
 * Syncs current bout state to the C++ API backend.
 */
async function syncToServer() {
	if (!currentBout || !currentBout.id) return;
	try {
		await fetch(`/api/bouts/${currentBout.id}`, {
			method: 'PUT',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(currentBout)
		});
	} catch (e) {
		console.warn('Could not sync bout to server:', e);
	}
}

/**
 * Saves bout state to device LocalStorage.
 */
function saveLocalState() {
	if (currentBout) {
		localStorage.setItem('activeBout', JSON.stringify(currentBout));
	}
}

/**
 * Fetches saved bouts from backend.
 */
async function fetchBoutsList() {
	const container = document.getElementById('boutsList');
	container.innerHTML = '<p>Loading bouts...</p>';

	try {
		const res = await fetch('/api/bouts');
		if (!res.ok) throw new Error('Failed to fetch bouts');
		const bouts = await res.json();

		if (bouts.length === 0) {
			container.innerHTML = '<p>No bouts saved yet.</p>';
			return;
		}

		container.innerHTML = '';
		bouts.forEach(b => {
			const item = document.createElement('div');
			item.className = 'bout-item';
			item.innerHTML = `
				<div>
					<strong>${b.leftFencerName} (${b.leftScore}) vs ${b.rightFencerName} (${b.rightScore})</strong>
					<div style="font-size: 0.8rem; color: #94a3b8;">${b.weapon} • ${b.status}</div>
				</div>
				<button type="button" class="btn btn-secondary btn-sm" onclick="loadBout('${b.id}')">Load</button>
			`;
			container.appendChild(item);
		});
	} catch (e) {
		container.innerHTML = `<p style="color: #ef4444;">Error loading bouts: ${e.message}</p>`;
	}
}

async function loadBout(id) {
	try {
		const res = await fetch(`/api/bouts/${id}`);
		if (res.ok) {
			currentBout = await res.json();
			closeModal('boutsListModal');
			render();
		}
	} catch (e) {
		alert('Failed to load bout');
	}
}

/* Modal Helpers */
function showNewBoutModal() {
	document.getElementById('newBoutModal').classList.add('active');
}

function showBoutListModal() {
	document.getElementById('boutsListModal').classList.add('active');
	fetchBoutsList();
}

function closeModal(id) {
	document.getElementById(id).classList.remove('active');
}
