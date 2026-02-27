#include <raylib.h>
#include <stdlib.h>

// --- CONSTANTS ------------>

#define WIDTH 900
#define HEIGHT 600

// --- PROTOTYPES ------------>

// --- ENTRY POINT ------------>

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Music Visualizer");
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		EndDrawing();
	}

	CloseWindow();

	return EXIT_SUCCESS;
}

// --- IMPLEMENTATIONS ------------>
