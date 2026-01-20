namespace FencingTracker.Api.Models;

class Bout {
	// ID related data
	private Guid Id;
	private string LeftFencerName;
	private string RightFencerName;

	// Fencing related data
	private int LeftScore;
	private int RightScore;
	
	private int LeftYellowCards;
	private int RightYellowCards;
	private int LeftRedCards;
	private int RightRedCards;

}
