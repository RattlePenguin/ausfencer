let bout = null;
let timerInterval = null;

/**
 * Creates the bout instance that will be manipulated in the referee screen.
 * Currently uses a mock json request.
 */
async function createBout() {
	const response = await fetch('/api/bouts', {
		method: 'POST',
		headers: { 'Content-Type': 'application/json' },
		body: JSON.stringify({
			leftFencerName: 'Allan',
			rightFencerName: 'Bob',
			leftScore: 2,
			weapon: 'Sabre'
		})
	});

	bout = await response.json();
	render();
}

/**
 * Renders the screen with local bout data. Not updated to server yet.
 */
function render() {
	document.getElementById('leftName').textContent = bout.leftFencerName;
	document.getElementById('rightName').textContent = bout.rightFencerName;
	document.getElementById('leftScore').textContent = bout.leftScore;
	document.getElementById('rightScore').textContent = bout.rightScore;
	updateTimerDisplay();
}

/**
 * Formats the timer from integer in seconds to the min:sec display.
 */
function updateTimerDisplay() {
	const min = Math.floor(bout.timeRemainingSeconds / 60);
	const sec = bout.timeRemainingSeconds % 60;

	document.getElementById('timer').textContent =
		`${min}:${sec.toString().padStart(2, '0')}`
}

/**
 * Changes the score display.
 * Buttons typically give delta +-1.
 * Renders.
 */
function changeScore(side, delta) {
	if (side === 'left') {
		bout.leftScore = Math.max(0, bout.leftScore + delta);
	} else {
		bout.rightScore = Math.max(0, bout.rightScore + delta);
	}
	
	render();
}

/**
 * Start/Stop button activates timer depending on bout data.
 * Saves timer onto server (PUT)... Might not be necessary.
 */
function toggleTimer() {
	if (bout.timeRemainingSeconds <= 0) return;

	bout.timerRunning = !bout.timerRunning;

	if (bout.timerRunning) {
		timerInterval = setInterval(() => {
			if (bout.timeRemainingSeconds > 0) {
				bout.timeRemainingSeconds--;
				updateTimerDisplay();
			} else {
				bout.timerRunning = false;
				clearInterval(timerInterval);
			}
		}, 1000);
	} else {
		clearInterval(timerInterval);
	}

	save();
}

/**
 * Ends the bout.
 */
function endBout() {
	bout.status = 'Finished';
	save();
	alert('Bout ended');
}

/**
 * Sends updated bout data to server.
 * Also renders so don't double stack with render().
 */
async function save() {
	await fetch(`/api/bouts/${bout.id}`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: JSON.stringify(bout)
	});
	render();
}

createBout();
