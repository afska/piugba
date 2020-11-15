#ifndef SYNC_H
#define SYNC_H

#include <libgba-sprite-engine/gba/tonc_core.h>
#include <libgba-sprite-engine/gba/tonc_memdef.h>
#include <libgba-sprite-engine/gba/tonc_memmap.h>

#include "Protocol.h"
#include "gameplay/debug/DebugTools.h"
#include "gameplay/save/SaveFile.h"
#include "utils/LinkConnection.h"

#define SYNC_BUFFER_SIZE 3
#define SYNC_TIMEOUT 60

enum SyncState {
  SYNC_STATE_SEND_ROM_ID,
  SYNC_STATE_SEND_PROGRESS,
  SYNC_STATE_PLAYING
};
enum SyncMode { SYNC_MODE_OFFLINE, SYNC_MODE_VS, SYNC_MODE_COOP };
enum SyncError {
  SYNC_ERROR_NONE,
  SYNC_ERROR_WTF,
  SYNC_ERROR_TOO_MANY_PLAYERS,
  SYNC_ERROR_ROM_MISMATCH,
  SYNC_ERROR_WRONG_MODE
};

inline bool isMultiplayer() {
  return IS_MULTIPLAYER(SAVEFILE_getGameMode());
}

class Syncer {
 public:
  u8 $libraryType = 0;
  u8 $completedSongs = 0;
  Syncer() {}

  inline bool isPlaying() { return state >= SyncState::SYNC_STATE_PLAYING; }
  inline bool isMaster() { return playerId == 0; }
  inline int getLocalPlayerId() { return playerId; }
  inline int getRemotePlayerId() { return !playerId; }
  inline SyncError getLastError() { return error; }

  inline SyncState getState() { return state; }
  inline void setState(SyncState newState) {
#ifdef SENV_DEBUG
    DEBUTRACE("--> state " + DSTR(newState));
#endif

    state = newState;
    timeoutCount = 0;
  }
  inline SyncMode getMode() { return mode; }

  void initialize(SyncMode mode);
  void update();
  void send(u8 event, u16 payload);
  void registerTimeout();

 private:
  SyncState state = SyncState::SYNC_STATE_SEND_ROM_ID;
  SyncMode mode = SyncMode::SYNC_MODE_OFFLINE;
  SyncError error = SyncError::SYNC_ERROR_NONE;
  int playerId = -1;
  u8 outgoingEvent = 0;
  u16 outgoingPayload = 0;
  u32 timeoutCount = 0;

  inline bool isActive() { return playerId > -1; }

  inline u16 getPartialRomId() {
    u32 romId = SAVEFILE_read32(SRAM->romId);
    return (romId & 0b00000000000111111111100000000000) >> 11;
  }

  void sync(LinkState* linkState);
  void sendOutgoingData();
  void checkTimeout();
  void fail(SyncError error);
  void reset();
  void resetData();
  void resetError();
};

extern Syncer* syncer;

#endif  // SYNC_H
