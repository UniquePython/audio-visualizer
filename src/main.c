#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "kiss_fft.h"

// --- CONSTANTS ------------>

#define WIDTH 900
#define HEIGHT 600

#define NSAMPLES 2048

#define NBARS 64
#define BAR_WIDTH 10

#define NGROUPS (((NSAMPLES) / 2) / NBARS)
#define MAG_SCALE 80.0f

#define SMOOTHING 0.8f

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
	int start, end;
	static float smoothed[NBARS];

	for (int bar = 0; bar < NBARS; bar++)
	{
		start = round(powf(NSAMPLES / 2, (float)bar / NBARS));
		end = round(powf(NSAMPLES / 2, (float)(bar + 1) / NBARS));
		if (end <= start)
			end = start + 1;

		float max = 0;
		for (int i = start; i < end; i++)
		{
			if (magnitudes[i] > max)
				max = magnitudes[i];
		}

		float scaled = logf(1.0f + max) * MAG_SCALE;
		smoothed[bar] = smoothed[bar] * SMOOTHING + scaled * (1.0f - SMOOTHING);
		float hue = 120 * (1 - smoothed[bar] / HEIGHT);
		DrawRectangle(bar * (WIDTH / NBARS), HEIGHT - smoothed[bar], BAR_WIDTH, smoothed[bar], ColorFromHSV(hue, 1.0f, 1.0f));
	}
}
