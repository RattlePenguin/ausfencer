using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;

using FencingTracker.Api.Models;
using FencingTracker.Api.Data;

namespace FencingTracker.Api.Controllers;

[ApiController]
[Route("api/[controller]")]
public class BoutsController : ControllerBase {
	// Inject DB context
	private readonly AppDbContext _db;

	public BoutsController(AppDbContext db) {
		_db = db;
	}

	/**
	 *  Creates a Bout instance from the request body.
	 *  Bout is added to db and local BoutStore.
	 */
	[HttpPost]
	public async Task<ActionResult<Bout>> CreateBout([FromBody] Bout bout) {
		// Persistent storage
		_db.Bouts.Add(bout);
		await _db.SaveChangesAsync();
		
		return CreatedAtAction(nameof(GetBout), new { id = bout.Id }, bout);
	}

	/**
	 *  Returns a list of all bouts.
	 */
	[HttpGet]
	public async Task<ActionResult<IEnumerable<Bout>>> GetAllBouts() {
		return await _db.Bouts.ToListAsync();
	}

	/**
	 *  Returns a bout with a given id.
	 */
	[HttpGet("{id}")]
	public async Task<ActionResult<Bout>> GetBout(Guid id) {
		var bout = await _db.Bouts.FindAsync(id);
		if (bout == null) {
			return NotFound();
		}
		return Ok(bout);
	}

	/**
	 *  Update a bout with a given id.
	 */
	[HttpPut("{id}")]
	public async Task<IActionResult> UpdateBout(Guid id, [FromBody] Bout updated) {
		var bout = await _db.Bouts.FindAsync(id);

		if (bout == null) {
			return NotFound();
		}
		
		bout.LeftScore = updated.LeftScore;
		bout.RightScore = updated.RightScore;
		bout.TimeRemainingSeconds = updated.TimeRemainingSeconds;
		bout.Status = updated.Status;

		await _db.SaveChangesAsync();

		return NoContent();
	}
}
