#include "Syncer.h"

#include "Protocol.h"

void Syncer::initialize(SyncMode mode) {
  this->mode = mode;
  reset();
  resetError();
}

void Syncer::update() {
  if (mode == SyncMode::SYNC_MODE_OFFLINE)
    return;

  auto linkState = linkConnection->tick(outgoingData);

  if (!linkState.isConnected()) {
    fail(SyncError::SYNC_ERROR_NONE);
    return;
  }

  if (linkState.playerCount > 2) {
    fail(SyncError::SYNC_ERROR_TOO_MANY_PLAYERS);
    return;
  }

  if (!isActive()) {
    playerId = linkState.currentPlayerId;
    reset();
  }

  if (isReady())
    resetError();

  sync(linkState);
}

void Syncer::sync(LinkState linkState) {
  u16 incomingData = linkState.data[!playerId];
  u8 incomingEvent = SYNCER_MSG_EVENT(incomingData);
  u16 incomingPayload = SYNCER_MSG_PAYLOAD(incomingData);

  switch (state) {
    case SyncState::SYNC_STATE_SEND_ROM_ID: {
      outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_ROM_ID, getPartialRomId());

      if (incomingEvent == SYNC_EVENT_ROM_ID) {
        if (incomingPayload == getPartialRomId())
          state = SyncState::SYNC_STATE_SEND_PROGRESS;
        else
          fail(SyncError::SYNC_ERROR_ROM_MISMATCH);
      }
    }
    case SyncState::SYNC_STATE_SEND_PROGRESS: {
      outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_MODE, mode);
      // TODO: Use SYNCER_MSG_PROGRESS_BUILD
      // TODO: Validate library type and library size

      if (incomingEvent == SYNC_EVENT_MODE) {
        if (incomingPayload == mode)
          state = SyncState::SYNC_STATE_SELECTING_SONG;
        else
          fail(SyncError::SYNC_ERROR_WRONG_MODE);
      }
    }
    case SyncState::SYNC_STATE_SELECTING_SONG: {
      // TODO: IMPLEMENT
      // TODO: DISCONNECT ON UNEXPECTED DATA
    }
  }
}

void Syncer::fail(SyncError error) {
  playerId = -1;
  reset();
  this->error = error;
}

void Syncer::reset() {
  state = SyncState::SYNC_STATE_SEND_ROM_ID;
  outgoingData = 0;
}

void Syncer::resetError() {
  error = SyncError::SYNC_ERROR_NONE;
}
