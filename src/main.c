#include <raylib.h>
#include <stdlib.h>
#include <string.h>

// --- CONSTANTS ------------>

#define WIDTH 900
#define HEIGHT 600

#define NSAMPLES 1024

// --- GLOBALS ------------>

float BUFFER[NSAMPLES];
int COUNT = 0;

// --- PROTOTYPES ------------>

void Callback(void *, unsigned int);

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
	SetAudioStreamCallback(music.stream, Callback);
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

void Callback(void *buffer, unsigned int frames)
{
	unsigned int copy = frames;
	if (COUNT + frames > NSAMPLES)
		copy = NSAMPLES - COUNT;
	memcpy(&BUFFER[COUNT], (float *)buffer, copy * sizeof(float));
	COUNT += copy;
	if (COUNT >= NSAMPLES)
		COUNT = 0;
	return;
}
