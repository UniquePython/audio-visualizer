#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "kiss_fft.h"

// --- CONSTANTS ------------>

#define WIDTH 900
#define HEIGHT 600

#define NSAMPLES 1024

#define NBARS 32
#define BAR_WIDTH 20

#define NGROUPS (((NSAMPLES) / 2) / NBARS)
#define MAG_SCALE 2.0f

// --- GLOBALS ------------>

float BUFFER[NSAMPLES];
int COUNT = 0;

// --- PROTOTYPES ------------>

void Callback(void *, unsigned int);
void DrawBars(const float *);

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
	AttachAudioStreamProcessor(music.stream, Callback);
	PlayMusicStream(music);

	kiss_fft_cfg cfg = kiss_fft_alloc(NSAMPLES, 0, NULL, NULL);

	kiss_fft_cpx fin[NSAMPLES];
	kiss_fft_cpx fout[NSAMPLES];
	float mag[NSAMPLES / 2];

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(BLACK);
		UpdateMusicStream(music);

		if (COUNT == 0)
		{
			for (int i = 0; i < NSAMPLES; i++)
			{
				fin[i].r = BUFFER[i];
				fin[i].i = 0.0f;
			}
			kiss_fft(cfg, fin, fout);
		}

		for (int j = 0; j < NSAMPLES / 2; j++)
			mag[j] = hypotf(fout[j].i, fout[j].r);

		DrawBars(mag);

		EndDrawing();
	}

	kiss_fft_free(cfg);

	CloseAudioDevice();
	CloseWindow();

	return EXIT_SUCCESS;
}

// --- IMPLEMENTATIONS ------------>

void Callback(void *buffer, unsigned int frames)
{
	float *samples = (float *)buffer;
	for (unsigned int i = 0; i < frames && COUNT < NSAMPLES; i++)
		BUFFER[COUNT++] = samples[i * 2]; // left channel only
	if (COUNT >= NSAMPLES)
		COUNT = 0;
	return;
}

void DrawBars(const float *magnitudes)
{
	float groups[NBARS];
	int nmax = 0;
	float max = -1;
	for (int i = 0; i < NSAMPLES / 2; i++)
	{
		if (magnitudes[i] > max)
			max = magnitudes[i];

		if (i % NGROUPS == NGROUPS - 1)
		{
			groups[nmax++] = max;
			max = -1;
		}
	}

	int height;
	for (int j = 0; j < NBARS; j++)
	{
		height = groups[j] * MAG_SCALE;
		DrawRectangle(j * (WIDTH / NBARS), HEIGHT - height, BAR_WIDTH, height, RAYWHITE);
	}
}
