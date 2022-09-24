#include <audio/audio.h>
#include <stdio.h>

int main(int argc, char** argv) {
  printf("It's basically magic!\n");

  /*a_ctx* a_ctx_create(const char* device, uint8_t layers, uint16_t max_sfx,
                    uint16_t max_buffers, uint16_t max_songs, uint16_t max_fx,
                    uint16_t max_filters, uint32_t pcm_size) { */
  a_ctx* ctx = a_ctx_create(0, 1, 16, 16, 0, 2, 2, 4096 * 4096);

  // TODO everything else here

  a_ctx_destroy(ctx);
  return 0;
}
