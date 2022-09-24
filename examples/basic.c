#include <stdio.h>
#include <stdint.h>
#include <audio/audio.h>

int main(int argc, char** argv) {
  // Create the overall audio context
  a_ctx* ctx = a_ctx_create(0, 2, 16, 16, 0, 2, 2, 4096 * 4);

  // Create a layer for the SFX to go to
  uint16_t layer_id = a_layer_create(ctx, "sfx", 16, 0);

  // Buffer in the sound effect from file
  uint16_t buffer_id = a_buf_create_fs(ctx, "resources/test.wav", "test", 0);

  // The position of the sound effect in 3D Space (not important for now)
  a_vec3 position;
  position[0] = 0.0f;
  position[1] = 0.0f;
  position[2] = 0.0f;

  // Create a request to manage how we want to play a sound
  a_req req      = a_req_create(position, 1.0f, 1.0f, 0, 0, 0, 0, 0);
  req.loop_count = 4;

  // Play the sound effect
  uint16_t sfx_id = a_sfx_play(ctx, layer_id, buffer_id, &req);

  // Let the sound effect loop 4 times
  while (1) {
    // Update the audio context
    a_ctx_update(ctx);

    if (req.state == AL_STOPPED) {
      break;
    }
  }

  a_buf_destroy(ctx, buffer_id);

  a_ctx_destroy(ctx);
  return 0;
}
