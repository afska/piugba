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

#include "PlaybackState.h"
#include "core/gsm.h"
#include "core/private.h" /* for sizeof(struct gsm_state) */
#include "utils/gbfs/gbfs.h"

#define TIMER_16MHZ 0
#define FIFO_ADDR_A 0x040000a0
#define FIFO_ADDR_B 0x040000A4
#define CHANNEL_A_MUTE 0b1111110011111111
#define CHANNEL_A_UNMUTE 0b0000001100000000
#define CHANNEL_B_MUTE 0b1100111111111111
#define CHANNEL_B_UNMUTE 0b0011000000000000
#define AUDIO_CHUNK_SIZE 33
#define FRACUMUL_PRECISION 0xFFFFFFFF
#define AS_MSECS (1146880 * 1000)
#define AS_CURSOR 3201039125
#define REG_DMA1CNT_L *(vu16*)(REG_BASE + 0x0c4)
#define REG_DMA1CNT_H *(vu16*)(REG_BASE + 0x0c6)

#define CODE_EWRAM __attribute__((section(".ewram")))
#define INLINE static inline __attribute__((always_inline))

Playback PlaybackState;

static const int rateDelays[] = {1, 2, 4, 0, 4, 2, 1};

static int rate = 0;
static u32 rateCounter = 0;
static u32 currentAudioChunk = 0;

/* GSM Player ----------------------------------------- */
#define PLAYER_PRE_UPDATE(ON_STEP, ON_STOP)           \
  dst_pos = double_buffers[cur_buffer];               \
                                                      \
  if (src_pos < src_end) {                            \
    for (i = 304 / 4; i > 0; i--) {                   \
      int cur_sample;                                 \
      if (decode_pos >= 160) {                        \
        if (src_pos < src_end)                        \
          gsm_decode(&decoder, src_pos, out_samples); \
        src_pos += sizeof(gsm_frame);                 \
        decode_pos = 0;                               \
        ON_STEP;                                      \
      }                                               \
                                                      \
      /* 2:1 linear interpolation */                  \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
                                                      \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
                                                      \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
                                                      \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
    }                                                 \
  } else if (src_pos != NULL) {                       \
    ON_STOP;                                          \
  }

uint32_t fracumul(uint32_t x, uint32_t frac) __attribute__((long_call));
static const GBFS_FILE* fs;
static const unsigned char* src;
static uint32_t src_len;
static const unsigned char* src_pos = NULL;
static const unsigned char* src_end = NULL;
static struct gsm_state decoder;
static signed short out_samples[160];
static signed char double_buffers[2][608] __attribute__((aligned(4)));
static unsigned int decode_pos = 160, cur_buffer = 0;
static signed char* dst_pos;
static int last_sample = 0;
static int i;

INLINE void gsmInit(gsm r) {
  memset((char*)r, 0, sizeof(*r));
  r->nrp = 40;
}

INLINE void mute() {
  DSOUNDCTRL = DSOUNDCTRL & CHANNEL_A_MUTE;
}

INLINE void unmute() {
  DSOUNDCTRL = DSOUNDCTRL | CHANNEL_A_UNMUTE;
}

INLINE void turnOnSound() {
  SETSNDRES(1);
  SNDSTAT = SNDSTAT_ENABLE;
  DSOUNDCTRL = 0b1111101100001110;
  mute();
}

INLINE void init() {
  /* TMxCNT_L is count; TMxCNT_H is control */
  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0x10000 - (924 / 2);
  REG_TM0CNT_H = TIMER_16MHZ | TIMER_START;

  mute();
}

INLINE void play(const char* name) {
  gsmInit(&decoder);
  src = gbfs_get_obj(fs, name, &src_len);
  src_pos = src;
  src_end = src + src_len;
  mute();
}

INLINE void stop() {
  src_pos = NULL;
  src_end = NULL;
  mute();
}

INLINE void disableAudioDMA() {
  // ----------------------------------------------------
  // This convoluted process was taken from the official manual.
  // It's supposed to disable DMA1 in a "safe" way, avoiding DMA lockups.
  //
  // 32-bit write
  // enabled = 1; start timing = immediately; transfer type = 32 bits;
  // repeat = off; destination = fixed; other bits = no change
  REG_DMA1CNT = (REG_DMA1CNT & 0b00000000000000001100110111111111) |
                (0x0004 << 16) | DMA_ENABLE | DMA32 | DMA_DST_FIXED;
  //
  // wait 4 cycles
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
  //
  // 16-bit write
  // enabled = 0; start timing = immediately; transfer type = 32 bits;
  // repeat = off; destination = fixed; other bits = no change
  REG_DMA1CNT_H = (REG_DMA1CNT_H & 0b0100110111111111) |
                  0b0000010100000000;  // DMA32 | DMA_DST_FIXED
  //
  // wait 4 more cycles
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
  // ----------------------------------------------------
}

INLINE void dsoundSwitchBuffers(const void* src) {
  // disable DMA1
  disableAudioDMA();

  // setup DMA1 for audio
  REG_DMA1SAD = (intptr_t)src;
  REG_DMA1DAD = (intptr_t)FIFO_ADDR_A;
  REG_DMA1CNT = DMA_DST_FIXED | DMA_SRC_INC | DMA_REPEAT | DMA32 | DMA_SPECIAL |
                DMA_ENABLE | 1;
}
/* ---------------------------------------------------- */

CODE_EWRAM void player_init() {
  fs = find_first_gbfs_file(0);
  turnOnSound();
  init();

  REG_DMA1SAD = (intptr_t)double_buffers[0];
  REG_DMA1DAD = (intptr_t)FIFO_ADDR_A;
  REG_DMA1CNT_L = 0x0004;
}

CODE_EWRAM void player_unload() {
  disableAudioDMA();
}

CODE_EWRAM void player_play(const char* name) {
  play(name);

  PlaybackState.msecs = 0;
  PlaybackState.hasFinished = false;
  PlaybackState.isLooping = false;
  rate = 0;
  rateCounter = 0;
  currentAudioChunk = 0;
}

CODE_EWRAM void player_loop(const char* name) {
  player_play(name);
  PlaybackState.isLooping = true;
}

CODE_EWRAM void player_seek(unsigned int msecs) {
  // (cursor must be a multiple of AUDIO_CHUNK_SIZE)
  // cursor = src_pos - src
  // msecs = cursor * msecsPerSample
  // msecsPerSample = AS_MSECS / FRACUMUL_PRECISION ~= 0.267
  // => msecs = cursor * 0.267
  // => cursor = msecs / 0.267 = msecs * 3.7453
  // => cursor = msecs * (3 + 0.7453)

  unsigned int cursor = msecs * 3 + fracumul(msecs, AS_CURSOR);
  cursor = (cursor / AUDIO_CHUNK_SIZE) * AUDIO_CHUNK_SIZE;
  src_pos = src + cursor;
  rateCounter = 0;
  currentAudioChunk = 0;
}

CODE_EWRAM void player_setRate(int newRate) {
  rate = newRate;
  rateCounter = 0;
}

CODE_EWRAM void player_stop() {
  stop();

  PlaybackState.msecs = 0;
  PlaybackState.hasFinished = false;
  PlaybackState.isLooping = false;
  rate = 0;
  rateCounter = 0;
  currentAudioChunk = 0;
}

CODE_EWRAM bool player_isPlaying() {
  return src_pos != NULL;
}

void player_onVBlank() {
  dsoundSwitchBuffers(double_buffers[cur_buffer]);

  if (src_pos != NULL)
    unmute();

  cur_buffer = !cur_buffer;
}

CODE_EWRAM void updateRate() {
  if (rate != 0) {
    rateCounter++;
    if (rateCounter == rateDelays[rate + RATE_LEVELS]) {
      src_pos += AUDIO_CHUNK_SIZE * (rate < 0 ? -1 : 1);
      rateCounter = 0;
    }
  }
}

void player_forever(int (*onUpdate)(),
                    void (*onRender)(),
                    void (*onAudioChunks)(unsigned int current)) {
  while (1) {
    int expectedAudioChunk = onUpdate();

    bool isSynchronized = expectedAudioChunk > 0;
    int availableAudioChunks = expectedAudioChunk - currentAudioChunk;
    if (isSynchronized && availableAudioChunks > AUDIO_SYNC_LIMIT) {
      // underrun (slave is behind master)
      unsigned int diff = availableAudioChunks - AUDIO_SYNC_LIMIT;

      src_pos += AUDIO_CHUNK_SIZE * diff;
      currentAudioChunk += diff;
      availableAudioChunks = AUDIO_SYNC_LIMIT;
    }

    PLAYER_PRE_UPDATE(
        {
          if (isSynchronized) {
            availableAudioChunks--;

            if (availableAudioChunks < -AUDIO_SYNC_LIMIT) {
              // overrun (master is behind slave)
              src_pos -= AUDIO_CHUNK_SIZE;
              availableAudioChunks = -AUDIO_SYNC_LIMIT;
            } else
              currentAudioChunk++;
          } else
            currentAudioChunk++;
        },
        {
          if (PlaybackState.isLooping)
            player_seek(0);
          else {
            player_stop();
            PlaybackState.hasFinished = true;
          }
        });

    updateRate();

    onAudioChunks(currentAudioChunk);

    unsigned int msecs = src_pos - src;
    msecs = fracumul(msecs, AS_MSECS);
    PlaybackState.msecs = msecs;

    VBlankIntrWait();

    onRender();
  }
}
