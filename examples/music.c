#include <stdio.h>
#include <stdlib.h>
#include <audio/audio.h>

int main(void) {
  a_ctx_info ctx_info = a_ctx_info_default();
  // Create the overall audio context
  a_ctx* ctx = a_ctx_create(ctx_info);

  uint16_t music_layer_id = a_layer_create(ctx, "music", 0, 1);
  a_layer_set_gain(ctx, music_layer_id, 0.05f);

  // The position of the source in 3D Space (not important
  // for now)
  a_vec3 position;
  position[0] = 0.0f;
  position[1] = 0.0f;
  position[2] = 0.0f;

  // Create a request to manage how we want to play a sound
  a_req req = a_req_create(position, 1.0f, 1.0f, 0, 0, 0, 0, 0);

  unsigned char* data_ptr = 0;

  // Create the song
  uint16_t song_id = a_song_create_fs(ctx, "resources/song.ogg", "song", 32, 3,
                                      4096 * 4, &data_ptr);

  float song_length = a_song_get_length(ctx, song_id);
  printf("Song length: %.2f\n", song_length);

  a_song_play(ctx, music_layer_id, song_id, &req);

  // Let the sound effect loop 4 times
  while (1) {
    // Update the audio context
    a_ctx_update(ctx);

    if (req.state == AL_STOPPED) {
      break;
    }
  }

  a_song_stop(ctx, song_id);
  a_song_destroy(ctx, song_id);
  free(data_ptr);

  a_ctx_destroy(ctx);
  return 0;
}
