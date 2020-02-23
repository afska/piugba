#include "gsm_main.h"
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_sound.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  // for memset
#include "../utils/gbfs.h"
#include "core/gsm.h"
#include "core/private.h" /* for sizeof(struct gsm_state) */

#define CMD_START_SONG 0x0400
#define TIMER_16MHZ 0

struct gsm_state decoder;
const GBFS_FILE* fs;
const unsigned char* src;
uint32_t src_len;

signed short out_samples[160];
signed char double_buffers[2][608] __attribute__((aligned(4)));

static void dsound_switch_buffers(const void* src) {
  REG_DMA1CNT = 0;

  /* no-op to let DMA registers catch up */
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");

  REG_DMA1SAD = (intptr_t)src;
  REG_DMA1DAD = (intptr_t)0x040000a0; /* write to FIFO A address */
  REG_DMA1CNT = DMA_DST_FIXED | DMA_SRC_INC | DMA_REPEAT | DMA32 | DMA_SPECIAL |
                DMA_ENABLE | 1;
}

void gsm_init(gsm r) {
  memset((char*)r, 0, sizeof(*r));
  r->nrp = 40;
}

void streaming_run(void (*update)()) {
  const unsigned char* src_pos = NULL;
  const unsigned char* src_end = NULL;
  unsigned int decode_pos = 160, cur_buffer = 0;
  unsigned short last_joy = 0x3ff;
  unsigned int cur_song = (unsigned int)(-1);
  int last_sample = 0;

  unsigned int nframes = 0;

  while (1) {
    update();

    unsigned short j = (REG_KEYINPUT & 0x3ff) ^ 0x3ff;
    unsigned short cmd = j & (~last_joy);
    signed char* dst_pos = double_buffers[cur_buffer];
    last_joy = j;

    // At end of track, proceed to the next
    if (src_pos >= src_end) {
      cur_song++;
      if (cur_song >= gbfs_count_objs(fs)) {
        cur_song = 0;
      }
      cmd |= CMD_START_SONG;
    }

    if (cmd & CMD_START_SONG) {
      char name[25];
      gsm_init(&decoder);
      src = gbfs_get_nth_obj(fs, cur_song, name, &src_len);
      src_pos = src;
      src_end = src + src_len;
    }

    for (j = 304 / 4; j > 0; j--) {
      int cur_sample;
      if (decode_pos >= 160) {
        if (src_pos < src_end) {
          gsm_decode(&decoder, src_pos, out_samples);
        }
        src_pos += sizeof(gsm_frame);
        decode_pos = 0;
      }

      /* 2:1 linear interpolation */
      cur_sample = out_samples[decode_pos++];
      *dst_pos++ = (last_sample + cur_sample) >> 9;
      *dst_pos++ = cur_sample >> 8;
      last_sample = cur_sample;

      cur_sample = out_samples[decode_pos++];
      *dst_pos++ = (last_sample + cur_sample) >> 9;
      *dst_pos++ = cur_sample >> 8;
      last_sample = cur_sample;

      cur_sample = out_samples[decode_pos++];
      *dst_pos++ = (last_sample + cur_sample) >> 9;
      *dst_pos++ = cur_sample >> 8;
      last_sample = cur_sample;

      cur_sample = out_samples[decode_pos++];
      *dst_pos++ = (last_sample + cur_sample) >> 9;
      *dst_pos++ = cur_sample >> 8;
      last_sample = cur_sample;
    }

    VBlankIntrWait();
    dsound_switch_buffers(double_buffers[cur_buffer]);

    cur_buffer = !cur_buffer;
  }
}

void player_init() {
  fs = find_first_gbfs_file(0);

  // TM0CNT_L is count; TM0CNT_H is control
  REG_TM0CNT_H = 0;
  // turn on sound circuit
  SETSNDRES(1);
  SNDSTAT = SNDSTAT_ENABLE;
  DSOUNDCTRL = 0x0b0e;
  REG_TM0CNT_L = 0x10000 - (924 / 2);
  REG_TM0CNT_H = TIMER_16MHZ | TIMER_START;
}

void maino(void (*update)()) {
  // Enable vblank IRQ for VBlankIntrWait()
  irqInit();
  irqEnable(IRQ_VBLANK);

  streaming_run(update);
}
