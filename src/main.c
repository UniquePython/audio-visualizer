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
	if (!IsWindowReady())
		exit(EXIT_FAILURE);
	SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

	InitAudioDevice();
	if (!IsAudioDeviceReady())
	{
		CloseWindow();
		exit(EXIT_FAILURE);
	}

	Music music = LoadMusicStream("./believer_music.mp3");
	if (!IsMusicValid(music))
	{
		CloseAudioDevice();
		CloseWindow();
		exit(EXIT_FAILURE);
	}
	PlayMusicStream(music);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		UpdateMusicStream(music);
		EndDrawing();
	}

	CloseAudioDevice();
	CloseWindow();

	return EXIT_SUCCESS;
}

// --- IMPLEMENTATIONS ------------>
