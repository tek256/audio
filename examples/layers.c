#include <audio/audio.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  // Info needed for how to create the audio context
  a_ctx_info ctx_info = a_ctx_info_default();

  // Create the overall audio context
  a_ctx* ctx = a_ctx_create(ctx_info);

  uint16_t music_layer_id = a_layer_create(ctx, "music", 0, 1);
  uint16_t sfx_layer_id   = a_layer_create(ctx, "sfx", 5, 0);

  // Make the music a bit more of a background element
  a_layer_set_gain(ctx, music_layer_id, 0.2f);
  a_layer_set_gain(ctx, sfx_layer_id, 0.9f);

  // The position of the source in 3D Space (not important
  // for now)
  a_vec3 position;
  a_vec3_set(position, 0.0f, 0.0f, 0.0f);

  uint16_t buf_id = a_buf_create_fs(ctx, "resources/test.wav", "sfx", 0);

  // data pointer for the music file gotten from file system for the song
  unsigned char* data_ptr = 0;

  // Create the song
  uint16_t song_id = a_song_create_fs(ctx, "resources/song.ogg", "song", 32, 3,
                                      4096 * 4, &data_ptr);

  // Create a requests to manage how we want to play our sounds
  a_req    song_req = a_req_create(position, 1.0f, 1.0f, 0, 0, 0, 0, 0);
  a_req    sfx_reqs[5];
  uint16_t sfx_ids[5];
  for (uint8_t i = 0; i < 5; ++i) {
    // Create individual requests for each sound effect
    a_vec3 sfx_pos;
    a_vec3_set(sfx_pos, -5.0f + (i * 2.0f), 0.0f, 0.0f);
    sfx_reqs[i]            = a_req_create(sfx_pos, 1.0f, 20.f, 0, 0, 0, 0, 0);
    sfx_reqs[i].loop_count = (i * 2) + 5;

    // Play each sound effect!
    sfx_ids[i] = a_sfx_play(ctx, sfx_layer_id, buf_id, &sfx_reqs[i]);
  }

  float song_length = a_song_get_length(ctx, song_id);

  a_song_play(ctx, music_layer_id, song_id, &song_req);

  while (1) {
    a_ctx_update(ctx);

    // If the main song is stopped/finished
    if (song_req.state == AL_STOPPED) {
      break;
    }
  }

  a_song_stop(ctx, song_id);
  a_song_destroy(ctx, song_id);
  free(data_ptr);

  a_ctx_destroy(ctx);
  return 0;
}
