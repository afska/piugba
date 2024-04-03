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
#define AUDIO_CHUNK_SIZE_GSM 33
#define AUDIO_CHUNK_SIZE_PCM 304
#define FRACUMUL_PRECISION 0xFFFFFFFF
#define AS_MSECS_GSM 1146880000
#define AS_MSECS_PCM 118273043  // 0xffffffff * (1000/36314)
#define AS_CURSOR_GSM 3201039125
#define REG_DMA2CNT_L *(vu16*)(REG_BASE + 0x0d0)
#define REG_DMA2CNT_H *(vu16*)(REG_BASE + 0x0d2)

#define CODE_EWRAM __attribute__((section(".ewram")))
#define INLINE static inline __attribute__((always_inline))

Playback PlaybackState;

// This player uses:
// - In GSM mode:
//   A sample rate of 18157hz, linearly interpolated to 36314hz.
//   Each GSM chunk is 33 bytes and represents 304 samples.
//   Two chunks are copied per frame, filling the 608 entries of the buffer.
//   (This is one of the few combinations of sample rate / buffer size that
//   time out perfectly in the 280896 cycles of a GBA frame)
//   See: (JS code)
//     for (var i=80; i<1000; i++)
//       if ((280896%i)==0 && (i%16) == 0)
//         console.log(
//           'timer =', 65536-(280896/i), '; buffer =',
//           i, '; sample rate =', i*(1<<24)/280896, 'hz'
//         );
// - In PCM s8 mode:
//   A sample rate of 36314.
//   Each PCM chunk is 304 bytes and represents 304 samples.
//   Two chunks are copied per frame.

static const int rateDelays[] = {1, 2, 4, 0, 4, 2, 1};

static bool isPCM = false;
static int rate = 0;
static u32 rateCounter = 0;
static u32 currentAudioChunk = 0;
static bool didRun = false;

#define AUDIO_CHUNK_SIZE (isPCM ? AUDIO_CHUNK_SIZE_PCM : AUDIO_CHUNK_SIZE_GSM)
#define AS_MSECS (isPCM ? AS_MSECS_PCM : AS_MSECS_GSM)
#define AS_CURSOR AS_CURSOR_GSM

/* GSM Player ----------------------------------------- */
#define PLAYER_PRE_UPDATE(ON_STEP, ON_STOP)             \
  didRun = true;                                        \
  dst_pos = double_buffers[cur_buffer];                 \
                                                        \
  if (isPCM) {                                          \
    for (i = 0; i < 608; i++) {                         \
      ON_STEP;                                          \
      ON_STEP;                                          \
      if (src_pos < src_end) {                          \
        *dst_pos++ = *src_pos++;                        \
      } else {                                          \
        if (src_pos != NULL) {                          \
          ON_STOP;                                      \
        }                                               \
        break;                                          \
      }                                                 \
    }                                                   \
  } else {                                              \
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
    }                                                   \
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
  DSOUNDCTRL = DSOUNDCTRL & CHANNEL_B_MUTE;
}

INLINE void unmute() {
  DSOUNDCTRL = DSOUNDCTRL | CHANNEL_B_UNMUTE;
}

INLINE void turnOnSound() {
  SETSNDRES(1);
  SNDSTAT = SNDSTAT_ENABLE;
  DSOUNDCTRL = 0b1111000000001100;
  mute();
}

INLINE void init() {
  /* TMxCNT_L is count; TMxCNT_H is control */
  REG_TM1CNT_H = 0;
  REG_TM1CNT_L = 0x10000 - (924 / 2);        // 0x10000 - (16777216 / 36314)
  REG_TM1CNT_H = TIMER_16MHZ | TIMER_START;  //            cpuFreq  / sampleRate

  mute();
}

INLINE void stop() {
  mute();
  src_pos = NULL;
  src_end = NULL;
  decode_pos = 160;
  cur_buffer = 0;
  last_sample = 0;
  for (u32 i = 0; i < 2; i++) {
    u32* bufferPtr = (u32*)double_buffers[i];
    for (u32 j = 0; j < 608 / 4; j++)
      bufferPtr[j] = 0;
  }
}

INLINE void play(const char* name) {
  stop();
  gsmInit(&decoder);
  src = gbfs_get_obj(fs, "414.aud.bin", &src_len);  // TODO: name
  src_pos = src;
  src_end = src + src_len;
  isPCM = true;  // TODO: REMOVE
}

INLINE void disableAudioDMA() {
  // ----------------------------------------------------
  // This convoluted process was taken from the official manual.
  // It's supposed to disable DMA2 in a "safe" way, avoiding DMA lockups.
  //
  // 32-bit write
  // enabled = 1; start timing = immediately; transfer type = 32 bits;
  // repeat = off; destination = fixed; other bits = no change
  REG_DMA2CNT = (REG_DMA2CNT & 0b00000000000000001100110111111111) |
                (0x0004 << 16) | DMA_ENABLE | DMA32 | DMA_DST_FIXED;
  //
  // wait 4 cycles
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
  //
  // 16-bit write
  // enabled = 0; start timing = immediately; transfer type = 32 bits;
  // repeat = off; destination = fixed; other bits = no change
  REG_DMA2CNT_H = (REG_DMA2CNT_H & 0b0100110111111111) |
                  0b0000010100000000;  // DMA32 | DMA_DST_FIXED
  //
  // wait 4 more cycles
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
  asm volatile("eor r0, r0; eor r0, r0" ::: "r0");
}

INLINE void dsoundStartAudioCopy(const void* src) {
  // disable DMA2
  disableAudioDMA();

  // setup DMA2 for audio
  REG_DMA2SAD = (intptr_t)src;
  REG_DMA2DAD = (intptr_t)FIFO_ADDR_B;
  REG_DMA2CNT = DMA_DST_FIXED | DMA_SRC_INC | DMA_REPEAT | DMA32 | DMA_SPECIAL |
                DMA_ENABLE | 1;
}
/* ---------------------------------------------------- */

CODE_EWRAM void player_init() {
  fs = find_first_gbfs_file(0);
  turnOnSound();
  init();
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
  dsoundStartAudioCopy(double_buffers[cur_buffer]);

  if (!didRun)
    return;

  if (src_pos != NULL)
    unmute();

  cur_buffer = !cur_buffer;
  didRun = false;
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
    // > main game loop
    int expectedAudioChunk = onUpdate();

    // > multiplayer audio sync
    bool isSynchronized = expectedAudioChunk > 0;
    int availableAudioChunks = expectedAudioChunk - currentAudioChunk;
    if (isSynchronized && availableAudioChunks > AUDIO_SYNC_LIMIT) {
      // underrun (slave is behind master)
      unsigned int diff = availableAudioChunks - AUDIO_SYNC_LIMIT;

      src_pos += AUDIO_CHUNK_SIZE * diff;
      currentAudioChunk += diff;
      availableAudioChunks = AUDIO_SYNC_LIMIT;
    }

    // > audio processing (back buffer)
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

    // > notify multiplayer audio sync cursor
    onAudioChunks(currentAudioChunk);

    // > adjust position based on audio rate
    updateRate();

    // > calculate played milliseconds
    unsigned int diff = src_pos - src;
    PlaybackState.msecs = fracumul(diff, AS_MSECS);

    // > wait for vertical blank
    VBlankIntrWait();

    // > draw
    onRender();
  }
}
