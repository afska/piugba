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

#include "GSMPlayer.h"
#include "PlaybackState.h"
#include "core/gsm.h"
#include "core/private.h" /* for sizeof(struct gsm_state) */
#include "utils/gbfs/gbfs.h"

Playback PlaybackState;

static const int rateDelays[] = {1, 2, 4, 0, 4, 2, 1};

static int rate = 0;
static u32 rateCounter = 0;

PLAYER_DEFINE(REG_DMA1CNT,
              REG_DMA1SAD,
              REG_DMA1DAD,
              FIFO_ADDR_A,
              CHANNEL_A_MUTE,
              CHANNEL_A_UNMUTE);

inline void player_init() {
  PLAYER_TURN_ON_SOUND();
  PLAYER_INIT(REG_TM0CNT_L, REG_TM0CNT_H);
}

inline void player_play(const char* name) {
  PLAYER_PLAY(name);
  PlaybackState.msecs = 0;
  PlaybackState.hasFinished = false;
  PlaybackState.isLooping = false;
  rate = 0;
  rateCounter = 0;
}

inline void player_loop(const char* name) {
  player_play(name);
  PlaybackState.isLooping = true;
}

inline void player_seek(unsigned int msecs) {
  // (cursor must be a multiple of AUDIO_CHUNK)
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
}

inline void player_setRate(int newRate) {
  rate = newRate;
  rateCounter = 0;
}

inline void player_stop() {
  PLAYER_STOP();
  PlaybackState.msecs = 0;
  PlaybackState.hasFinished = false;
  PlaybackState.isLooping = false;
  rate = 0;
  rateCounter = 0;
}

inline bool player_isPlaying() {
  return src_pos != NULL;
}

inline void player_mute() {
  mute();
}

inline void player_unmute() {
  unmute();
}

inline void player_forever(void (*update)()) {
  while (1) {
    if (rate != 0) {
      rateCounter++;
      if (rateCounter == rateDelays[rate + RATE_LEVELS]) {
        src_pos += AUDIO_CHUNK_SIZE * (rate < 0 ? -1 : 1);
        rateCounter = 0;
      }
    }

    unsigned int msecs = src_pos - src;
    msecs = fracumul(msecs, AS_MSECS);
    PlaybackState.msecs = msecs;
    update();

    PLAYER_PRE_UPDATE({
      if (PlaybackState.isLooping)
        player_seek(0);
      else {
        player_stop();
        PlaybackState.hasFinished = true;
      }
    });

    // --------------
    VBlankIntrWait();  // VBLANK
    // --------------

    PLAYER_POST_UPDATE();
  }
}
