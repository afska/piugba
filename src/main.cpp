#include <libgba-sprite-engine/gba_engine.h>
#include <tonc.h>

#include "../libs/interrupt.h"
#include "gameplay/Sequence.h"
#include "gameplay/debug/DebugTools.h"
#include "gameplay/multiplayer/Syncer.h"
#include "player/PlaybackState.h"
#include "utils/SceneUtils.h"

extern "C" {
#include "player/player.h"
}

// Emulators and flashcarts use this string to autodetect the save type
const char* SAVEFILE_TYPE_HINT = "SRAM_Vnnn\0\0";

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
                          .sendTimerId = LINK_CABLE_DEFAULT_SEND_TIMER_ID});
Syncer* syncer = new Syncer();
static const GBFS_FILE* fs = find_first_gbfs_file(0);

int main() {
  if (fs == NULL)
    BSOD("GBFS file not found.");

  linkUniversal->deactivate();
  setUpInterrupts();
  player_init();
  SEQUENCE_initialize(engine, fs);

  engine->setScene(SEQUENCE_getInitialScene());
  player_forever(
      []() {
        syncer->update();
        engine->update();

        if (syncer->$isPlayingSong && !syncer->$hasStartedAudio)
          synchronizeSongStart();

        return syncer->$isPlayingSong && !syncer->isMaster()
                   ? (int)syncer->$currentAudioChunk
                   : 0;  // (unsynchronized)
      },
      [](u32 current) {
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

  SCENE_softReset();
}

void ISR_vblank() {
  player_onVBlank();
  LINK_UNIVERSAL_ISR_VBLANK();
}

void setUpInterrupts() {
  irq_init(NULL);

  interrupt_init();

  // VBlank
  interrupt_set_handler(INTR_VBLANK, ISR_vblank);
  interrupt_enable(INTR_VBLANK);

  // LinkUniversal
  interrupt_set_handler(INTR_SERIAL, LINK_UNIVERSAL_ISR_SERIAL);
  interrupt_enable(INTR_SERIAL);
  interrupt_set_handler(INTR_TIMER3, LINK_UNIVERSAL_ISR_TIMER);
  interrupt_enable(INTR_TIMER3);

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

  u16 start = SYNC_START_SONG | syncer->$currentSongId;
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
}
