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
#include "fxes.h"
#include "utils/gbfs/gbfs.h"

Playback PlaybackState;

PLAYER_DEFINE(REG_DMA1CNT,
              REG_DMA1SAD,
              REG_DMA1DAD,
              FIFO_ADDR_A,
              CHANNEL_A_MUTE,
              CHANNEL_A_UNMUTE);

void player_init() {
  PLAYER_TURN_ON_SOUND();
  PLAYER_INIT(REG_TM0CNT_L, REG_TM0CNT_H);
  fxes_init();
}

void player_play(const char* name) {
  PLAYER_PLAY(name);
  PlaybackState.msecs = 0;
  PlaybackState.hasFinished = false;
}

void player_stop() {
  PLAYER_STOP();
  PlaybackState.msecs = 0;
  PlaybackState.hasFinished = false;
}

void player_forever(void (*update)()) {
  while (1) {
    unsigned int msecs = src_pos - src;
    msecs = fracumul(msecs, 1146880 * 1000);
    PlaybackState.msecs = msecs;
    update();

    PLAYER_PRE_UPDATE({
      PlaybackState.msecs = 0;
      PlaybackState.hasFinished = true;
    });
    fxes_preUpdate();

    // --------------
    VBlankIntrWait();  // VBLANK
    // --------------

    PLAYER_POST_UPDATE();
    fxes_postUpdate();
  }
}
