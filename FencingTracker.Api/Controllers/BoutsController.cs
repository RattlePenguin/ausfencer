using Microsoft.AspNetCore.Mvc;

using FencingTracker.Api.Models;
using FencingTracker.Api.Services;
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

	[HttpPost]
	public async Task<ActionResult<Bout>> CreateBout([FromBody] Bout bout) {
		// Persistent storage
		_db.Bouts.Add(bout);
		await _db.SaveChangesAsync();
		
		// Local storage
		var created = BoutStore.Create(bout);
		return CreatedAtAction(nameof(GetBout), new { id = created.Id }, created);
	}

	[HttpGet("{id}")]
	public ActionResult<Bout> GetBout(Guid id) {
		var bout = BoutStore.Get(id);
		if (bout == null) {
			return NotFound();
		}
		return Ok(bout);
	}

	[HttpPut("{id}")]
	public IActionResult UpdateBout(Guid id, [FromBody] Bout bout) {
		var oldBout = BoutStore.Get(id);
		if (oldBout == null) {
			return BadRequest("Bout id does not exist.");
		} else if (oldBout.Status == BoutStatus.Finished) {
			return BadRequest("Cannot update a finished bout.");
		}
		
		var updated = BoutStore.Update(id, bout);
		if (!updated) {
			return NotFound();
		}
		return NoContent();
	}
}
