#include "fxes.h"

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
#include "core/gsm.h"
#include "core/private.h" /* for sizeof(struct gsm_state) */
#include "utils/gbfs/gbfs.h"

PLAYER_DEFINE(REG_DMA2CNT,
              REG_DMA2SAD,
              REG_DMA2DAD,
              FIFO_ADDR_B,
              CHANNEL_B_MUTE,
              CHANNEL_B_UNMUTE);

void fxes_init() {
  PLAYER_INIT(REG_TM1CNT_L, REG_TM1CNT_H);
}

void fxes_play(const char* name) {
  PLAYER_PLAY(name);
}

void fxes_stop() {
  PLAYER_STOP();
}

void fxes_preUpdate() {
  PLAYER_PRE_UPDATE({ fxes_stop(); });
}

void fxes_postUpdate() {
  PLAYER_POST_UPDATE();
}
