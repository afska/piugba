#ifndef SYNCER_H
#define SYNCER_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "gameplay/save/SaveFile.h"
#include "utils/LinkConnection.h"

enum SyncState {
  SYNC_STATE_SEND_ROM_ID,
  SYNC_STATE_SEND_PROGRESS,
  SYNC_STATE_SELECTING_SONG
};
enum SyncMode { SYNC_MODE_OFFLINE, SYNC_MODE_VS, SYNC_MODE_COOP };
enum SyncError {
  SYNC_ERROR_NONE,
  SYNC_ERROR_TOO_MANY_PLAYERS,
  SYNC_ERROR_ROM_MISMATCH,
  SYNC_ERROR_WRONG_MODE
};

class Syncer {
 public:
  Syncer() {}

  inline bool isReady() {
    return state >= SyncState::SYNC_STATE_SELECTING_SONG;
  }
  inline bool isMaster() { return playerId == 0; }
  inline SyncError getLastError() { return error; }

  void initialize(SyncMode mode);
  void update();

 private:
  SyncState state = SyncState::SYNC_STATE_SEND_ROM_ID;
  SyncMode mode = SyncMode::SYNC_MODE_OFFLINE;
  SyncError error = SyncError::SYNC_ERROR_NONE;
  int playerId = -1;
  u16 outgoingData = 0;

  inline bool isActive() { return playerId > -1; }

  inline u16 getPartialRomId() {
    u32 romId = SAVEFILE_read32(SRAM->romId);
    return (romId & 0b00000000000111111111100000000000) >> 11;
  }

  void sync(LinkState linkState);
  void fail(SyncError error);
  void reset();
  void resetError();
};

extern Syncer* syncer;

#endif  // SYNCER_H
