#include "Syncer.h"

#include "gameplay/save/SaveFile.h"

#define ROM_ID_MASK 0b00000000000111111111100000000000
#define ROM_ID_MASK_OFFSET 11

void Syncer::update() {
  auto linkState = linkConnection->tick(outgoingData);

  if (!linkState.isConnected()) {
    fail(SyncError::SYNC_ERROR_NONE);
    return;
  }

  if (linkState.playerCount > 2) {
    fail(SyncError::SYNC_ERROR_TOO_MANY_PLAYERS);
    return;
  }

  if (!isActive) {
    isActive = true;
    reset();
  }

  syncState(linkState);
}

void Syncer::syncState(LinkState linkState) {
  if (mode == SyncMode::SYNC_MODE_OFFLINE)
    return;

  u16 incomingData = linkState.data[!linkState.currentPlayerId];
  u8 incomingEvent = SYNCER_MSG_EVENT(incomingData);
  u16 incomingPayload = SYNCER_MSG_PAYLOAD(incomingData);

  if (state == SyncState::SYNC_STATE_WAIT_ROM_ID) {
    u32 romId = SAVEFILE_read32(SRAM->romId);
    u16 partialRomId = (romId & ROM_ID_MASK) >> ROM_ID_MASK_OFFSET;
    outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_ROM_ID, partialRomId);

    if (incomingEvent == SYNC_EVENT_ROM_ID) {
      if (incomingPayload == partialRomId)
        state = SyncState::SYNC_STATE_WAIT_MODE;
      else {
        fail(SyncError::SYNC_ERROR_ROM_MISMATCH);
        return;
      }
    }
  }

  if (state == SyncState::SYNC_STATE_WAIT_MODE) {
    outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_MODE, mode);

    if (incomingEvent == SYNC_EVENT_MODE) {
      if (incomingPayload == mode)
        state = SyncState::SYNC_STATE_PLAYING;
      else {
        fail(SyncError::SYNC_ERROR_WRONG_MODE);
        return;
      }
    }
  }
}

void Syncer::fail(SyncError error) {
  isActive = false;
  reset();
  this->error = error;
}

void Syncer::reset() {
  state = SyncState::SYNC_STATE_WAIT_ROM_ID;
  error = SyncError::SYNC_ERROR_NONE;
}
