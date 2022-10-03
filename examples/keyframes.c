#include <audio/audio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> // for random seed
#include <math.h>

#include <stdint.h>
#if defined(__linux)
#define HAVE_POSIX_TIMER
#include <time.h>
#ifdef CLOCK_MONOTONIC
#define CLOCKID CLOCK_MONOTONIC
#else
#define CLOCKID CLOCK_REALTIME
#endif
#elif defined(__APPLE__)
#define HAVE_MACH_TIMER
#include <mach/mach_time.h>
#include <uinstd.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/* Get time in nanoseconds from the Operating System's High performance timer */
static uint64_t get_ns() {
  static uint64_t is_init = 0;
#if defined(__APPLE__)
  static mach_timebase_info_data_t info;
  if (0 == is_init) {
    mach_timebase_info(&info);
    is_init = 1;
  }
  uint64_t now;
  now = mach_absolute_time();
  now *= info.numer;
  now /= info.denom;
  return now;
#elif defined(__linux)
  static struct timespec linux_rate;
  if (0 == is_init) {
    clock_getres(CLOCKID, &linux_rate);
    is_init = 1;
  }
  uint64_t        now;
  struct timespec spec;
  clock_gettime(CLOCKID, &spec);
  now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
  return now;
#elif defined(_WIN32)
  static LARGE_INTEGER win_frequency;
  if (0 == is_init) {
    QueryPerformanceFrequency(&win_frequency);
    is_init = 1;
  }
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  return (uint64_t)((1e9 * now.QuadPart) / win_frequency.QuadPart);
#endif
}

double get_time() { return get_ns() / 1e6; }

double a_sleep(double duration) {
#if defined(_WIN32) || defined(_WIN64)
  Sleep(duration * 1e-3);
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec  = duration / 1e3;
  ts.tv_nsec = fmod(duration, 1e-6);
  nanosleep(&ts, NULL);
#else
  uint32_t sleep_conv = duration / 1e-3;
  usleep(sleep_conv);
#endif
  return duration;
}

int main(void) {
  int keyframe_count = 16;

  srand(time(0));

  a_ctx_info ctx_info = a_ctx_info_default();
  // Create the overall audio context
  a_ctx* ctx = a_ctx_create(ctx_info);

  uint16_t music_layer_id = a_layer_create(ctx, "music", 0, 1);
  a_layer_set_gain(ctx, music_layer_id, 0.05f);

  // The position of the source in 3D Space (not important
  // for now)
  a_vec3 position;
  a_vec3_set(position, 0.0f, 0.0f, 0.0f);

  // Create a request to manage how we want to play a sound
  a_req req = a_req_create(position, 1.0f, 1.0f, 0, 0, 0, 0, 0);

  // Generate random keyframe data
  float            time_index = 0.0f;
  float*           times      = calloc(keyframe_count, sizeof(float));
  float*           values     = calloc(keyframe_count, sizeof(float));
  a_keyframe_ease* eases      = calloc(keyframe_count, sizeof(a_keyframe_ease));

  // evenly distribute keyframes across the length of the song
  float keyframe_time = 30000.0f / keyframe_count;

  for (int i = 0; i < keyframe_count; ++i) {
    times[i] = time_index;

    // Random float range: [0.2f, 0.8f]
    values[i] = ((((float)rand() / RAND_MAX) * 0.8f) + 0.2f);
    eases[i]  = (rand() % 4);
    time_index += keyframe_time;
  }

  printf("Timeline:\n");
  for (int i = 0; i < keyframe_count; ++i) {
    printf("%.2f: %.2f\n", times[i], values[i]);
  }

  float timeline_value = 0.0f;

  // Keyframe timeline
  a_timeline timeline = a_timeline_create(times, values, eases, keyframe_count);
  a_timeline_view view = a_timeline_create_view(&timeline);
  a_timeline_set_output(&view, &timeline_value);

  time_index = 0.0f;
  for (int i = 0; i < keyframe_count * 4; ++i) {
    time_index += (((float)rand() / RAND_MAX) * ((keyframe_time / 4) * 0.75f)) +
                  ((keyframe_time / 4) * 0.25f);
    printf("Timeline value @ %.2f: %.2f\n", time_index,
           a_timeline_calc_value_at(&timeline, time_index));
  }

  // Create the song
  unsigned char* data_ptr = 0;
  uint16_t song_id = a_song_create_fs(ctx, "resources/song.ogg", "song", 32, 3,
                                      4096 * 4, &data_ptr);

  a_song_play(ctx, music_layer_id, song_id, &req);

  double target_update_rate = 16.0f; // 16 ms (60 fps)

  double time = get_time();
  // Let the sound effect loop 4 times
  while (1) {
    // Update the audio context
    a_ctx_update(ctx);
    double now   = get_time();
    double delta = now - time;
    time         = now;

    a_timeline_update(&view, (float)delta);

    int   current_index = view.current_index;
    float prev_value    = view.timeline->keyframes[view.current_index].value;
    float next_value    = prev_value;
    if (current_index < view.timeline->keyframe_count)
      next_value = view.timeline->keyframes[view.current_index].value;

    // Update the song's gain to match the current value from the timeline
    req.gain = view.value;

    if (req.state == AL_STOPPED) {
      break;
    }

    a_sleep(target_update_rate - delta);
  }

  a_song_stop(ctx, song_id);
  a_song_destroy(ctx, song_id);
  free(data_ptr);

  a_ctx_destroy(ctx);
  return 0;
}
