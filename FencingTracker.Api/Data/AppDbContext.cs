using Microsoft.EntityFrameworkCore;
using FencingTracker.Api.Models;

namespace FencingTracker.Api.Data;

public class AppDbContext : DbContext {
	public AppDbContext(DbContextOptions<AppDbContext> options) : base(options) {}

	public DbSet<Bout> Bouts => Set<Bout>();
}
