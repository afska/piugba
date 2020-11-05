#include "Syncer.h"

void Syncer::initialize(SyncMode mode) {
  this->mode = mode;
  reset();
  resetError();
}

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
    playerId = linkState.currentPlayerId;
    reset();
  }

  if (isReady())
    resetError();

  sync(linkState);
}

void Syncer::sync(LinkState linkState) {
  if (mode == SyncMode::SYNC_MODE_OFFLINE)
    return;

  u16 incomingData = linkState.data[!playerId];
  u8 incomingEvent = SYNCER_MSG_EVENT(incomingData);
  u16 incomingPayload = SYNCER_MSG_PAYLOAD(incomingData);

  switch (state) {
    case SyncState::SYNC_STATE_WAIT_ROM_ID: {
      outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_ROM_ID, getPartialRomId());

      if (incomingEvent == SYNC_EVENT_ROM_ID) {
        if (incomingPayload == getPartialRomId())
          state = SyncState::SYNC_STATE_WAIT_MODE;
        else
          fail(SyncError::SYNC_ERROR_ROM_MISMATCH);
      }
    }
    case SyncState::SYNC_STATE_WAIT_MODE: {
      outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_MODE, mode);

      if (incomingEvent == SYNC_EVENT_MODE) {
        if (incomingPayload == mode)
          state = SyncState::SYNC_STATE_PLAYING;
        else
          fail(SyncError::SYNC_ERROR_WRONG_MODE);
      }
    }
    case SyncState::SYNC_STATE_PLAYING: {
      // TODO: IMPLEMENT
      // TODO: DISCONNECT ON UNEXPECTED DATA
    }
  }
}

void Syncer::fail(SyncError error) {
  isActive = false;
  playerId = -1;
  reset();
  this->error = error;
}

void Syncer::reset() {
  state = SyncState::SYNC_STATE_WAIT_ROM_ID;
  outgoingData = 0;
}

void Syncer::resetError() {
  error = SyncError::SYNC_ERROR_NONE;
}
