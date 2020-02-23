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
#include "gsm.h"
#include "private.h" /* for sizeof(struct gsm_state) */
#include "utils/gbfs.h"

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

#define TIMER_16MHZ 0

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

/* gsm_init() **************
   This is to gsm_create() as placement new is to new.
*/
void gsm_init(gsm r) {
  memset((char*)r, 0, sizeof(*r));
  r->nrp = 40;
}

void wait4vbl(void) {
  asm volatile("mov r2, #0; swi 0x05" ::: "r0", "r1", "r2", "r3");
}

#if 0

static void dsound_silence(void)
{
  DMA[1].control = 0;
}

void pre_decode_run(void)
{
  const char *src_pos;
  const char *src_end;
  char *dst_pos = EWRAM;

  src = gbfs_get_obj(fs, "butterfly.pcm.gsm", &src_len);
  if(!src)
    {
      LCDMODE = 0;
      PALRAM[0] = RGB(31, 23, 0);
      while(1) {}
    }

  src_pos = src;
  src_end = src_pos + src_len;

  gsm_init(&decoder);

  while(src_pos < src_end)
    {
      unsigned int i;

      while(LCD_Y != 0 && LCD_Y != 76 && LCD_Y != 152) {}

      PALRAM[0] = RGB(0, 0, 31);
      if(gsm_decode(&decoder, src_pos, out_samples))
        {
          LCDMODE = 0;
          PALRAM[0] = RGB(31, 31, 0);
          while(1) {}
        }
      src_pos += sizeof(gsm_frame);
      PALRAM[0] = RGB(0, 0, 15);
      for(i = 0; i < 160; i++)
        dst_pos[i] = out_samples[i] >> 8;
      dst_pos += 160;
      PALRAM[0] = RGB(0, 0, 0);
    }
  PALRAM[0] = RGB(0, 31, 0);

  dsound_switch_buffers(EWRAM);

}
#endif

#define CMD_START_SONG 0x0400

void streaming_run(void (*update)()) {
  const unsigned char* src_pos = NULL;
  const unsigned char* src_end = NULL;
  unsigned int decode_pos = 160, cur_buffer = 0;
  unsigned short last_joy = 0x3ff;
  unsigned int cur_song = (unsigned int)(-1);
  int last_sample = 0;
  unsigned int locked = 0;

  unsigned int nframes = 0;

  while (1) {
    update();

    unsigned short j = (REG_KEYINPUT & 0x3ff) ^ 0x3ff;
    unsigned short cmd = j & (~last_joy | KEY_R | KEY_L);
    signed char* dst_pos = double_buffers[cur_buffer];

    last_joy = j;

    // At end of track, proceed to the next
    if (src_pos >= src_end) {
      cmd |= KEY_RIGHT;
    }

    if (cmd & KEY_RIGHT) {
      cur_song++;
      if (cur_song >= gbfs_count_objs(fs)) {
        cur_song = 0;
      }
      cmd |= CMD_START_SONG;
    }

    if (cmd & KEY_LEFT) {
      if (cur_song == 0) {
        cur_song = gbfs_count_objs(fs) - 1;
      } else {
        cur_song--;
      }
      cmd |= CMD_START_SONG;
    }

    if (cmd & CMD_START_SONG) {
      char name[25];
      gsm_init(&decoder);
      src = gbfs_get_nth_obj(fs, cur_song, name, &src_len);

      // If reached by seek, go near end of the track.
      // Otherwise, go to the start.
      if (cmd & KEY_L) {
        src_pos = src + src_len - 33 * 60;
      } else {
        src_pos = src;
      }
      src_end = src + src_len;
    }

    if (locked & KEY_START) { /* if paused */
      for (j = 304 / 2; j > 0; j--) {
        *dst_pos++ = last_sample >> 8;
        *dst_pos++ = last_sample >> 8;
        *dst_pos++ = last_sample >> 8;
        *dst_pos++ = last_sample >> 8;
      }
    } else {
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
    }

    VBlankIntrWait();
    dsound_switch_buffers(double_buffers[cur_buffer]);

    cur_buffer = !cur_buffer;
  }
}

void maino(void (*update)()) {
  // Enable vblank IRQ for VBlankIntrWait()
  irqInit();
  irqEnable(IRQ_VBLANK);

  streaming_run(update);
}
