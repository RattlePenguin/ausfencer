namespace FencingTracker.Api.Models;

public class Bout {
	// ID related data
	public Guid Id { get; set; } = Guid.NewGuid();

	public string LeftFencerName { get; set; } = string.Empty;
	public string RightFencerName { get; set; } = string.Empty;

	// Fencing related data
	public int LeftScore { get; set; }
	public int RightScore { get; set; }

	public int TimeRemainingSeconds { get; set; } = 180;
	public bool TimeRunning { get; set; }
	
	public int LeftYellowCards { get; set; }
	public int RightYellowCards { get; set; }
	public int LeftRedCards { get; set; }
	public int RightRedCards { get; set; }

	public Weapon Weapon { get; set; } = Weapon.Foil;
	public BoutStatus Status { get; set; } = BoutStatus.NotStarted;

	public DateTime CreatedAt { get; set; } = DateTime.UtcNow;
	public DateTime? FinishedAt { get; set; }
}
