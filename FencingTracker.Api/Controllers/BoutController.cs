using Microsoft.AspNetCore.Mvc;

using FencingTracker.Api.Models;
using FencingTracker.Api.Services;

namespace FencingTracker.Api.Controllers;

[ApiController]
[Route("api/[controller]")]
public class BoutController : ControllerBase {
	[HttpPost]
	public ActionResult<Bout> CreateBout([FromBody] Bout bout) {
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
		if (bout.Status == BoutStatus.Finished) {
			return BadRequest("Cannot update a finished bout.");
		}
		
		var updated = BoutStore.Update(id, bout);
		if (!updated) {
			return NotFound();
		}
		return NoContent();
	}
}
