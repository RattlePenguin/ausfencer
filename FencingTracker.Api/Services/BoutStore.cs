using FencingTracker.Api.Models;

namespace FencingTracker.Api.Services;

public static class BoutStore {
	// Store bouts in a dictionary
	private static readonly Dictionary<Guid, Bout> _bouts = new();
	
	// Add an instance of a Bout to BoutStore.
	public static Bout Create(Bout bout) {
		_bouts[bout.Id] = bout;
		return bout;
	}

	// Retrieve an instance of a Bout from BoutStore. Otherwise return NULL.
	public static Bout? Get(Guid id) {
		_bouts.TryGetValue(id, out var bout);
		return bout;
	}

	// Update the entry at id to updatedBout. If not exists, do nothing.
	public static bool Update(Guid id, Bout updatedBout) {
		if (!_bouts.ContainsKey(id)) {
			return false;
		}
		updatedBout.Id = id;
		_bouts[id] = updatedBout;
		return true;
	}
}
