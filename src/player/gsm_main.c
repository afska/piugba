#include <gba_compression.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_sound.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  // for memset

#include "gbfs.h"
#include "gsm.h"
#include "private.h" /* for sizeof(struct gsm_state) */

#include "gsm_main.h"

#if 0
#define PROFILE_WAIT_Y(y) \
  do {                    \
  } while (REG_VCOUNT != (y))
#define PROFILE_COLOR(r, g, b) (BG_COLORS[0] = RGB5((r), (g), (b)))
#else
#define PROFILE_WAIT_Y(y) ((void)0)
#define PROFILE_COLOR(r, g, b) ((void)0)
#endif

extern const char _x16Tiles[2048];  // font

// hud.c stuff

/**
 * does (uint64_t)x * frac >> 32
 */
uint32_t fracumul(uint32_t x, uint32_t frac) __attribute__((long_call));

void dma_memset16(void* dst, unsigned int c16, size_t n) {
  volatile unsigned short src = c16;
  DMA_Copy(3, &src, dst, DMA_SRC_FIXED | DMA16 | (n >> 1));
}

void bitunpack1(void* restrict dst, const void* restrict src, size_t len) {
  // Load tiles
  BUP bgtilespec = {.SrcNum = len,
                    .SrcBitNum = 1,
                    .DestBitNum = 4,
                    .DestOffset = 0,
                    .DestOffset0_On = 0};
  BitUnPack(src, dst, &bgtilespec);
}

/**
 * Writes a string to the HUD.
 */
static void hud_wline(unsigned int y, const char* s) {
  unsigned short* dst = MAP[31][y * 2] + 1;
  unsigned int wid_left;

  // Write first 28 characters of text line
  for (wid_left = 28; wid_left > 0 && *s; wid_left--) {
    unsigned char c0 = *s++;

    dst[0] = c0 << 1;
    dst[32] = (c0 << 1) | 1;
    ++dst;
  }
  // Clear to end of line
  for (; wid_left > 0; wid_left--, dst++) {
    dst[0] = ' ' << 1;
    dst[32] = (' ' << 1) | 1;
  }
}

static void hud_cls(void) {
  dma_memset16(MAP[31], ' ' << 1, 32 * 20 * 2);
}

void hud_init(void) {
  BG_COLORS[0] = RGB5(27, 31, 27);
  BG_COLORS[1] = RGB5(0, 16, 0);
  bitunpack1(PATRAM4(0, 0), _x16Tiles, sizeof(_x16Tiles));
  // REG_DISPCNT = 0;
  REG_BG2CNT = SCREEN_BASE(31) | CHAR_BASE(0);

  hud_cls();
  // hud_wline(1, "GSM Player for GBA");
  // hud_wline(2, "Copr. 2004, 2019");
  // hud_wline(3, "Damian Yerrick");
  // hud_wline(4, "and Toast contributors");
  // hud_wline(5, "(See TOAST-COPYRIGHT.txt)");

  VBlankIntrWait();
  REG_DISPCNT = 0 | BG2_ON;
}

/* base 10, 10, 6, 10 conversion */
static unsigned int hud_bcd[] = {600, 60, 10, 1};

#undef BCD_LOOP
#define BCD_LOOP(b)      \
  if (src >= fac << b) { \
    src -= fac << b;     \
    c += 1 << b;         \
  }

static void decimal_time(char* dst, unsigned int src) {
  unsigned int i;

  for (i = 0; i < 4; i++) {
    unsigned int fac = hud_bcd[i];
    char c = '0';

    BCD_LOOP(3);
    BCD_LOOP(2);
    BCD_LOOP(1);
    BCD_LOOP(0);
    *dst++ = c;
  }
}

struct HUD_CLOCK {
  unsigned int cycles;
  unsigned char trackno[2];
  unsigned char clock[4];
} hud_clock;

static const char clockmax[4] = {9, 9, 5, 9};

/**
 * @param locked 1 for locked, 0 for not locked
 * @param t offset in bytes from start of sample
 * (at 18157 kHz, 33/160 bytes per sample)
 */
void hud_frame(int locked, unsigned int t) {
  char line[16];
  char time_bcd[4];

  /* a fractional value for Seconds Per Byte
     1/33 frame/byte * 160 sample/frame * 924 cpu/sample / 2^24 sec/cpu
     * 2^32 fracunits = 1146880 sec/byte fracunits
   */

  t = fracumul(t, 1146880);
  if (t > 5999)
    t = 5999;
  decimal_time(time_bcd, t);

  line[0] = (locked & KEY_SELECT) ? 12 : ' ';
  line[1] = (locked & KEY_START) ? 16 : ' ';
  line[2] = ' ';
  line[3] = hud_clock.trackno[0] + '0';
  line[4] = hud_clock.trackno[1] + '0';
  line[5] = ' ';
  line[6] = ' ';
  line[7] = time_bcd[0];
  line[8] = time_bcd[1];
  line[9] = ':';
  line[10] = time_bcd[2];
  line[11] = time_bcd[3];
  line[12] = '\0';
  hud_wline(9, line);
}

void hud_new_song(const char* name, unsigned int trackno) {
  int upper;

  hud_wline(5, "Playing");
  hud_wline(6, name);
  hud_clock.cycles = 0;

  for (upper = 0; upper < 4; upper++)
    hud_clock.clock[0] = 0;
  upper = trackno / 10;
  hud_clock.trackno[1] = trackno - upper * 10;

  trackno = upper;
  upper = trackno / 10;
  hud_clock.trackno[0] = trackno - upper * 10;
}

// gsmplay.c ////////////////////////////////////////////////////////

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

void init_sound(void) {
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

struct gsm_state decoder;
const GBFS_FILE* fs;
const unsigned char* src;
uint32_t src_len;

signed short out_samples[160];
signed char double_buffers[2][608] __attribute__((aligned(4)));

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

void reset_gba(void) __attribute__((long_call));
void hud_init(void);
void hud_new_song(const char* name, unsigned int trackno);
void hud_frame(int locked, unsigned int t);

void streaming_run(/*void (*update)()*/) {
  const unsigned char* src_pos = NULL;
  const unsigned char* src_end = NULL;
  unsigned int decode_pos = 160, cur_buffer = 0;
  unsigned short last_joy = 0x3ff;
  unsigned int cur_song = (unsigned int)(-1);
  int last_sample = 0;
  unsigned int locked = 0;

  unsigned int nframes = 0;

  while (1) {
    // update();

    REG_BG0HOFS = ++nframes;
    unsigned short j = (REG_KEYINPUT & 0x3ff) ^ 0x3ff;
    unsigned short cmd = j & (~last_joy | KEY_R | KEY_L);
    signed char* dst_pos = double_buffers[cur_buffer];

    last_joy = j;

    /*
          if((j & (KEY_A | KEY_B | KEY_SELECT | KEY_START))
             == (KEY_A | KEY_B | KEY_SELECT | KEY_START))
            reset_gba();
    */

    if (cmd & KEY_SELECT) {
      locked ^= KEY_SELECT;
    }

    if (locked & KEY_SELECT) {
      cmd = 0;
    }

    if (cmd & KEY_START) {
      locked ^= KEY_START;
    }

    if (cmd & KEY_L) {
      src_pos -= 33 * 50;
      if (src_pos < src) {
        cmd |= KEY_LEFT;
      }
    }

    // R button: Skip forward
    if (cmd & KEY_R) {
      src_pos += 33 * 50;
    }

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
      hud_new_song(name, cur_song + 1);
      // If reached by seek, go near end of the track.
      // Otherwise, go to the start.
      if (cmd & KEY_L) {
        src_pos = src + src_len - 33 * 60;
      } else {
        src_pos = src;
      }
      src_end = src + src_len;
    }

    PROFILE_WAIT_Y(0);

    // BG_COLORS[0] = RGB(22, 0, 0);

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

    PROFILE_COLOR(27, 27, 27);
    VBlankIntrWait();
    dsound_switch_buffers(double_buffers[cur_buffer]);
    PROFILE_COLOR(27, 31, 27);

    hud_frame(locked, src_pos - src);
    cur_buffer = !cur_buffer;
  }
}

void maino(/*void (*update)()*/) {
  // Enable vblank IRQ for VBlankIntrWait()
  irqInit();
  irqEnable(IRQ_VBLANK);

  hud_init();
  fs = find_first_gbfs_file(find_first_gbfs_file);
  if(!fs) {
    hud_wline(7, "Please append gsmsongs.gbfs");
    BG_COLORS[0] = RGB5(31, 23, 23);
    BG_COLORS[1] = RGB5(16, 0, 0);
    while (1) {
      VBlankIntrWait();
    }
  }

  init_sound();
  streaming_run(/*update*/);
}
