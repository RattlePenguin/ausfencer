using Microsoft.EntityFrameworkCore;
using FencingTracker.Api.Models;

namespace FencingTracker.Api.Data;

/**
 *  Defines the context class and entity classes that make up the database.
 *  Think of AppDbContext as a strongly-typed session with the database.
 */
public class AppDbContext : DbContext {
	/**
	 *  Constructor that passes options up to DbContext.
	 */
	public AppDbContext(DbContextOptions<AppDbContext> options) : base(options) {}

	/**
	 *  DbSet represents a table in the database.
	 *  Based on the Bout entity.
	 *  Lambda gives the set of Bout entities tracked in db.
	 */
	public DbSet<Bout> Bouts => Set<Bout>();
}
