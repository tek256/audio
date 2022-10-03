#include <audio/audio.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  // Info needed for how to create the audio context
  a_ctx_info ctx_info  = a_ctx_info_default();
  ctx_info.max_fx      = 16;
  ctx_info.max_filters = 16;

  // Create the overall audio context
  a_ctx* ctx = a_ctx_create(ctx_info);

  uint16_t music_layer_id = a_layer_create(ctx, "music", 0, 1);

  // Make the music a bit more of a background element
  a_layer_set_gain(ctx, music_layer_id, 0.4f);

  // The position of the source in 3D Space (not important
  // for now)
  a_vec3 position;
  a_vec3_set(position, 0.0f, 0.0f, 0.0f);

  // data pointer for the music file gotten from file system for the song
  unsigned char* data_ptr = 0;

  // Create the song
  uint16_t song_id = a_song_create_fs(ctx, "resources/song.ogg", "song", 32, 3,
                                      4096 * 4, &data_ptr);

  // Create the default reverb effect! (You can change the values as you like!)
  a_fx_eq fx_eq = a_fx_eq_default();
  // Change the EQ values (only low pitch sound)
  fx_eq.high_gain = 0.126f;
  fx_eq.mid2_gain = 0.126f;
  fx_eq.mid1_gain = 0.126f;

  // Create the placeholder struct
  uint16_t eq = a_fx_create(ctx, FX_EQ, &fx_eq);

  // Create a requests to manage how we want to play our sounds
  a_req song_req = a_req_create(position, 1.0f, 1.0f, 0, &eq, 1, 0, 0);

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
