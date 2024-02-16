#include <libgba-sprite-engine/gba_engine.h>
#include <tonc.h>

#include "../libs/interrupt.h"
#include "gameplay/Sequence.h"
#include "gameplay/debug/DebugTools.h"
#include "gameplay/multiplayer/Syncer.h"
#include "player/PlaybackState.h"
#include "utils/SceneUtils.h"
#include "utils/flashio/FlashcartSDCard.h"

extern "C" {
#include "player/player.h"
}

// Emulators and flashcarts use this string to autodetect the save type
const char* SAVEFILE_TYPE_HINT = "SRAM_Vnnn\0\0";

void validateBuild();
void setUpInterrupts();
void synchronizeSongStart();
static std::shared_ptr<GBAEngine> engine{new GBAEngine()};
LinkUniversal* linkUniversal =
    new LinkUniversal(LinkUniversal::Protocol::AUTODETECT,
                      "piuGBA",
                      (LinkUniversal::CableOptions){
                          .baudRate = LinkCable::BAUD_RATE_1,
                          .timeout = SYNC_IRQ_TIMEOUT,
                          .remoteTimeout = SYNC_REMOTE_TIMEOUT,
                          .interval = SYNC_SEND_INTERVAL,
                          .sendTimerId = LINK_CABLE_DEFAULT_SEND_TIMER_ID},
                      (LinkUniversal::WirelessOptions){
                          .retransmission = true,
                          .maxPlayers = 2,
                          .timeout = LINK_WIRELESS_DEFAULT_TIMEOUT,
                          .remoteTimeout = LINK_WIRELESS_DEFAULT_REMOTE_TIMEOUT,
                          .interval = SYNC_SEND_INTERVAL,
                          .sendTimerId = LINK_WIRELESS_DEFAULT_SEND_TIMER_ID,
                          .asyncACKTimerId = 2});
Syncer* syncer = new Syncer();
FlashcartSDCard* flashcartSDCard = new FlashcartSDCard();
static const GBFS_FILE* fs = find_first_gbfs_file(0);

int main() {
  linkUniversal->deactivate();

  REG_WAITCNT = 0x4317;  // (3,1 waitstates, prefetch ON)

  flashcartSDCard->activate();
  // u32 cursor = 0;
  // u8 buff[512];
  // if (!flashcartSDCard->read(0, buff, 1)) {
  //   BSOD("error (1)");
  // } else {
  //   if (buff[0x52] != 'F') {
  //     u32 offset = buff[0x1c6] | (buff[0x1c7] << 8) | (buff[0x1c8] << 16) |
  //                  (buff[0x1c9] << 24);
  //     if (!flashcartSDCard->read(offset, buff, 1))
  //       BSOD("resp = 1 (2)");
  //   }

  //   // CUR = 2098048

  //   // ALL GOOD!
  //   cursor =
  //       (1024 * 1024 * 1024) / 512;  // start looking past the first partit.
  //   while (!(buff[0] == '!' && buff[2] == 'p' && buff[1] == '!' &&
  //            buff[3] == 'i' && buff[5] == 'G' && buff[4] == 'u' &&
  //            buff[6] == 'B' && buff[8] == '-' && buff[7] == 'A')) {
  //     BSOL(std::to_string(cursor));
  //     cursor++;
  //     if (!flashcartSDCard->read(cursor, buff, 1))
  //       BSOD("resp = 1");
  //   }

  //   BSOL(std::to_string(cursor));
  // }

  validateBuild();
  setUpInterrupts();
  player_init();
  SEQUENCE_initialize(engine, fs);

  engine->setScene(SEQUENCE_getInitialScene());
  player_forever(
      []() {
        // (onUpdate)
        LINK_UNIVERSAL_ISR_VBLANK();
        syncer->update();
        engine->update();

        if (syncer->$isPlayingSong && !syncer->$hasStartedAudio)
          synchronizeSongStart();

        return syncer->$isPlayingSong && !syncer->isMaster()
                   ? (int)syncer->$currentAudioChunk
                   : 0;  // (unsynchronized)
      },
      []() {
        // (onRender)
        engine->render();
        EFFECT_render();
      },
      [](u32 current) {
        // (onAudioChunk)
        if (syncer->$isPlayingSong) {
          if (syncer->isMaster()) {
            syncer->$currentAudioChunk = current;
            syncer->directSend(SYNC_AUDIO_CHUNK_HEADER |
                               ((u16)current + AUDIO_SYNC_LIMIT));
          }
#ifdef SENV_DEBUG
          LOGN(current, -1);
          LOGN(syncer->$currentAudioChunk, 0);
#endif
        }
      });

  LOGSTR(SAVEFILE_TYPE_HINT, 0);

  return 0;
}

void ISR_reset() {
  if (syncer->$isPlayingSong || syncer->$resetFlag) {
    syncer->$resetFlag = true;
    return;
  }
  if (syncer->isPlaying())
    return;

  SCENE_softReset();
}

void validateBuild() {
  if (fs == NULL)
    BSOD("GBFS file not found.");
  if (!ENV_ARCADE && gbfs_get_obj(fs, "_snm_0_list.txt", NULL) == NULL)
    BSOD("This is not an ARCADE build.");
}

void setUpInterrupts() {
  interrupt_init();

  // VBlank
  interrupt_set_handler(INTR_VBLANK, player_onVBlank);
  interrupt_enable(INTR_VBLANK);

  // LinkUniversal
  interrupt_set_handler(INTR_SERIAL, LINK_UNIVERSAL_ISR_SERIAL);
  interrupt_enable(INTR_SERIAL);
  interrupt_set_handler(INTR_TIMER3, LINK_UNIVERSAL_ISR_TIMER);
  interrupt_enable(INTR_TIMER3);

  // LinkWireless async-ACK
  interrupt_set_handler(INTR_TIMER2, LINK_UNIVERSAL_ISR_ACK_TIMER);
  interrupt_enable(INTR_TIMER2);

  // A+B+START+SELECT
  REG_KEYCNT = 0b1100000000001111;
  interrupt_set_handler(INTR_KEYPAD, ISR_reset);
  interrupt_enable(INTR_KEYPAD);
}

void synchronizeSongStart() {
  // discard all previous messages and wait for sync
  u8 remoteId = syncer->getRemotePlayerId();

  while (linkUniversal->read(remoteId) != LINK_CABLE_NO_DATA)
    ;

  u16 start = SYNC_START_SONG | syncer->$currentSongChecksum;
  bool isOnSync = false;
  while (syncer->$isPlayingSong && !isOnSync) {
    syncer->directSend(start);
    VBlankIntrWait();
    linkUniversal->sync();
    isOnSync = linkUniversal->read(remoteId) == start;
    if (!isOnSync)
      syncer->registerTimeout();
  }
  if (!isOnSync)
    return;

  while (linkUniversal->read(remoteId) != LINK_CABLE_NO_DATA)
    ;

  if (!syncer->isMaster())
    syncer->$currentAudioChunk = AUDIO_SYNC_LIMIT;

  syncer->$hasStartedAudio = true;
  syncer->clearTimeout();
}
