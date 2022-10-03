// Stress: Just an audio stress testing utility!
// NOTE: If you change the gain, it can get very loud
#include <audio/audio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// These are to separate out the sources in 3D Space so it's not all at once
#define ROW_COUNT 16
#define COL_COUNT 16

#define ROW_SPACING 4.0f
#define COL_SPACING 4.0f

// Pass in arguments to specify how many sound effects should be played
int main(int argc, char** argv) {
  int effect_count = 512; // just for now a smaller amount of effects
  // Now lets see about adding effects to it and whatnot, that should really
  // fuck with it (extra cpu usage)

  a_ctx_info ctx_info         = a_ctx_info_default();
  ctx_info.max_mono_sources   = effect_count;
  ctx_info.max_stereo_sources = effect_count;
  ctx_info.max_sfx            = effect_count;
  ctx_info.max_fx             = 2;

  a_ctx* ctx = a_ctx_create(ctx_info);

  // Create a layer for the SFX to go to
  uint16_t layer_id = a_layer_create(ctx, "sfx", effect_count, 0);
  a_layer_set_gain(ctx, layer_id, 0.05f);

  // Buffer in the sound effect from file
  uint16_t buffer_id = a_buf_create_fs(ctx, "resources/test.wav", "test", 0);

  uint16_t* sfx_ids  = calloc(effect_count, sizeof(uint16_t));
  a_req*    sfx_reqs = calloc(effect_count, sizeof(a_req));

  // Create the default reverb effect! (You can change the values as you like!)
  a_fx_reverb fx_reverb = a_fx_reverb_default();
  // Create the placeholder struct
  uint16_t reverb = a_fx_create(ctx, FX_REVERB, &fx_reverb);

  // Basically we'll just stack these sources in 3D Space to form a giant cube
  // of noise
  for (uint16_t i = 0; i < effect_count; ++i) {
    int x_cell = i % ROW_COUNT;
    int y_cell = i / ROW_COUNT;

    float x_pos = (x_cell - (ROW_COUNT / 2)) * ROW_SPACING;
    float y_pos = (y_cell - (COL_COUNT / 2)) * COL_SPACING;
    float z_pos = 0.0f;

    a_vec3 req_pos;
    a_vec3_set(req_pos, x_pos, y_pos, z_pos);
    sfx_reqs[i] = a_req_create(req_pos, 1.0f, 256.f, 0, &reverb, 1, 0, 0);
    sfx_reqs[i].loop_count = 10;
    sfx_ids[i]             = a_sfx_play(ctx, layer_id, buffer_id, &sfx_reqs[i]);
  }

  // Let the sound effect loop 4 times
  while (1) {
    // Update the audio context
    a_ctx_update(ctx);

    if (sfx_reqs[0].state == AL_STOPPED) {
      break;
    }
  }

  free(sfx_reqs);
  free(sfx_ids);
  a_buf_destroy(ctx, buffer_id);

  a_ctx_destroy(ctx);
  return 0;
}
