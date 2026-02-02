using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace FencingTracker.Api.Migrations
{
    /// <inheritdoc />
    public partial class InitialCreate : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "Bouts",
                columns: table => new
                {
                    Id = table.Column<Guid>(type: "TEXT", nullable: false),
                    LeftFencerName = table.Column<string>(type: "TEXT", nullable: false),
                    RightFencerName = table.Column<string>(type: "TEXT", nullable: false),
                    LeftScore = table.Column<int>(type: "INTEGER", nullable: false),
                    RightScore = table.Column<int>(type: "INTEGER", nullable: false),
                    TimeRemainingSeconds = table.Column<int>(type: "INTEGER", nullable: false),
                    TimeRunning = table.Column<bool>(type: "INTEGER", nullable: false),
                    LeftYellowCards = table.Column<int>(type: "INTEGER", nullable: false),
                    RightYellowCards = table.Column<int>(type: "INTEGER", nullable: false),
                    LeftRedCards = table.Column<int>(type: "INTEGER", nullable: false),
                    RightRedCards = table.Column<int>(type: "INTEGER", nullable: false),
                    Weapon = table.Column<int>(type: "INTEGER", nullable: false),
                    Status = table.Column<int>(type: "INTEGER", nullable: false),
                    CreatedAt = table.Column<DateTime>(type: "TEXT", nullable: false),
                    FinishedAt = table.Column<DateTime>(type: "TEXT", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_Bouts", x => x.Id);
                });
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "Bouts");
        }
    }
}
