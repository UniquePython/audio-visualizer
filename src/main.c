#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

#define FONT_SIZE 30
#define SKIP_SECONDS 5.0f
#define SEEK_TIMER 0.5f

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

	bool paused = false;
	float volume = 1.0f;
	float seek_timer = 0.0f;
	char seek_text[32] = {0};

	kiss_fft_cfg cfg = kiss_fft_alloc(NSAMPLES, 0, NULL, NULL);

	kiss_fft_cpx fin[NSAMPLES];
	kiss_fft_cpx fout[NSAMPLES];
	float mag[NSAMPLES / 2];

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(BLACK);

		if (IsKeyPressed(KEY_SPACE))
		{
			paused = !paused;
			if (paused)
				PauseMusicStream(music);
			else
				ResumeMusicStream(music);
		}

		if (IsKeyPressed(KEY_UP))
			volume += 0.05f;
		if (IsKeyPressed(KEY_DOWN))
			volume -= 0.05f;

		if (volume >= 1.0f)
			volume = 1.0f;
		if (volume <= 0.0f)
			volume = 0.0f;

		SetMusicVolume(music, volume);

		int textWidth = 0;

		if (IsKeyPressed(KEY_RIGHT))
		{
			seek_timer = SEEK_TIMER;
			SeekMusicStream(music, GetMusicTimePlayed(music) + SKIP_SECONDS);
			sprintf(seek_text, ">> %gs", SKIP_SECONDS);
		}
		if (IsKeyPressed(KEY_LEFT))
		{
			seek_timer = SEEK_TIMER;
			SeekMusicStream(music, GetMusicTimePlayed(music) - SKIP_SECONDS);
			sprintf(seek_text, "<< %gs", SKIP_SECONDS);
		}

		if (seek_timer > 0.0f)
		{
			seek_timer -= GetFrameTime();
			textWidth = MeasureText(seek_text, FONT_SIZE);
			DrawText(seek_text, WIDTH - textWidth - 10, 70, FONT_SIZE, WHITE);
		}

		char hud_center[32] = {0};
		sprintf(hud_center, "%02i:%02i", (int)GetMusicTimePlayed(music) / 60, (int)GetMusicTimePlayed(music) % 60);
		textWidth = MeasureText(hud_center, FONT_SIZE);
		DrawText(hud_center, (WIDTH - textWidth) / 2, 10, FONT_SIZE, WHITE);

		char hud_right[32] = {0};

		sprintf(hud_right, "%s", paused ? "PAUSED" : "PLAYING");
		textWidth = MeasureText(hud_right, FONT_SIZE);
		DrawText(hud_right, WIDTH - textWidth - 10, 10, FONT_SIZE, WHITE);

		sprintf(hud_right, "%i%%", (int)(volume * 100));
		textWidth = MeasureText(hud_right, FONT_SIZE);
		DrawText(hud_right, WIDTH - textWidth - 10, 40, FONT_SIZE, WHITE);

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
		float hue = 75 * (1 - smoothed[bar] / HEIGHT);
		if (hue < 0)
			hue = 0;
		DrawRectangle(bar * (WIDTH / NBARS), HEIGHT - smoothed[bar], BAR_WIDTH, smoothed[bar], ColorFromHSV(hue, 1.0f, 1.0f));
	}
}
