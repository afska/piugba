#ifndef SYNCER_H
#define SYNCER_H

#include <libgba-sprite-engine/gba/tonc_core.h>

#include "utils/LinkConnection.h"

enum SyncState {
  SYNC_STATE_WAIT_ROM_ID,
  SYNC_STATE_WAIT_MODE,
  SYNC_STATE_PLAYING
};
enum SyncMode { SYNC_MODE_OFFLINE, SYNC_MODE_VS, SYNC_MODE_COOP };
enum SyncError {
  SYNC_ERROR_NONE,
  SYNC_ERROR_TOO_MANY_PLAYERS,
  SYNC_ERROR_ROM_MISMATCH,
  SYNC_ERROR_WRONG_MODE
};

#define SYNC_EVENT_ROM_ID 1
#define SYNC_EVENT_MODE 2

/*
F: Heartbit
E: Reserved (always 0)
A~D: Event (16 values)
0~9: Payload (1024 values)
*/
#define SYNCER_MSG_BUILD(EVENT, PAYLOAD) (((EVENT) << 10) | (PAYLOAD))
#define SYNCER_MSG_EVENT(MSG) (((MSG) >> 10) & 0b1111)
#define SYNCER_MSG_PAYLOAD(MSG) ((MSG)&0b1111111111)

class Syncer {
 public:
  Syncer() {}

  inline bool isReady() { return state >= SyncState::SYNC_STATE_PLAYING; }
  inline SyncError getLastError() { return error; }
  inline void setMode(SyncMode mode) { this->mode = mode; }

  void update();

 private:
  bool isActive = false;
  SyncState state = SyncState::SYNC_STATE_WAIT_ROM_ID;
  SyncMode mode = SyncMode::SYNC_MODE_OFFLINE;
  SyncError error = SyncError::SYNC_ERROR_NONE;
  u16 outgoingData = 0;

  void syncState(LinkState linkState);
  void fail(SyncError error);
  void reset();
};

extern Syncer* syncer;

#endif  // SYNCER_H
