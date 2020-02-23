#include "player.h"
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

const unsigned char* src_pos = NULL;
const unsigned char* src_end = NULL;
unsigned int decode_pos = 160, cur_buffer = 0;
signed char* dst_pos;
int last_sample = 0;
int i;

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

void enable_vblank_interrupt() {
  irqInit();
  irqEnable(IRQ_VBLANK);
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

void player_play(char* name) {
  gsm_init(&decoder);
  src = gbfs_get_obj(fs, name, &src_len);
  src_pos = src;
  src_end = src + src_len;
}

void player_forever(void (*update)()) {
  enable_vblank_interrupt();

  while (1) {
    update(src, src_pos, src_end);

    dst_pos = double_buffers[cur_buffer];

    if (src_pos < src_end) {
      for (i = 304 / 4; i > 0; i--) {
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
    } else {
      src_pos = NULL;
      src_end = NULL;
    }

    VBlankIntrWait();
    dsound_switch_buffers(double_buffers[cur_buffer]);

    cur_buffer = !cur_buffer;
  }
}
