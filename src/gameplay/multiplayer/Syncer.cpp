#include "Syncer.h"

#include "gameplay/Key.h"

#define ASSERT(CONDITION, FAILURE_REASON) \
  if (!(CONDITION)) {                     \
    fail(FAILURE_REASON);                 \
    return;                               \
  }

#define ASSERT_EVENT(EXPECTED_EVENT)       \
  if (incomingEvent != (EXPECTED_EVENT)) { \
    timeoutCount++;                        \
    break;                                 \
  } else                                   \
    timeoutCount = 0;

void Syncer::initialize(SyncMode mode) {
  this->mode = mode;
  reset();
  resetError();
}

void Syncer::update() {
  if (mode == SyncMode::SYNC_MODE_OFFLINE)
    return;

  auto linkState = linkConnection->tick(outgoingData);

  ASSERT(linkState.isConnected(), SyncError::SYNC_ERROR_NONE);
  ASSERT(linkState.playerCount == 2, SyncError::SYNC_ERROR_TOO_MANY_PLAYERS);

  if (!isActive()) {
    reset();
    playerId = linkState.currentPlayerId;
  }

  if (isReady())
    resetError();

  sync(linkState);
  linkConnection->tick(outgoingData);
}

void Syncer::sync(LinkState linkState) {
  u16 incomingData = linkState.data[!playerId];
  u8 incomingEvent = SYNCER_MSG_EVENT(incomingData);
  u16 incomingPayload = SYNCER_MSG_PAYLOAD(incomingData);

  outgoingData = 0;

  switch (state) {
    case SyncState::SYNC_STATE_SEND_ROM_ID: {
      outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_ROM_ID, getPartialRomId());

      ASSERT_EVENT(SYNC_EVENT_ROM_ID);

      ASSERT(incomingPayload == getPartialRomId(),
             SyncError::SYNC_ERROR_ROM_MISMATCH);
      setState(SyncState::SYNC_STATE_SEND_PROGRESS);

      break;
    }
    case SyncState::SYNC_STATE_SEND_PROGRESS: {
      u8 modeBit = mode == SyncMode::SYNC_MODE_COOP;
      outgoingData =
          isMaster()
              ? SYNCER_MSG_BUILD(SYNC_EVENT_PROGRESS,
                                 SYNCER_MSG_PROGRESS_BUILD(
                                     modeBit, SAVEFILE_getMaxLibraryType(),
                                     SAVEFILE_getMaxCompletedSongs()))
              : SYNCER_MSG_BUILD(SYNC_EVENT_PROGRESS, modeBit);

      ASSERT_EVENT(SYNC_EVENT_PROGRESS);

      if (isMaster()) {
        ASSERT(incomingPayload == modeBit, SyncError::SYNC_ERROR_WRONG_MODE);
        setState(SyncState::SYNC_STATE_SELECTING_SONG);
      } else {
        u8 receivedModeBit = SYNCER_MSG_PROGRESS_MODE(incomingPayload);
        u8 receivedLibraryType =
            SYNCER_MSG_PROGRESS_LIBRARY_TYPE(incomingPayload);
        u8 receivedCompletedSongs =
            SYNCER_MSG_PROGRESS_COMPLETED_SONGS(incomingPayload);

        ASSERT(receivedModeBit == modeBit, SyncError::SYNC_ERROR_WRONG_MODE);
        ASSERT(receivedLibraryType >= DifficultyLevel::NORMAL &&
                   receivedLibraryType <= DifficultyLevel::CRAZY,
               SyncError::SYNC_ERROR_ROM_MISMATCH);
        ASSERT(receivedCompletedSongs >= 0 &&
                   receivedCompletedSongs <= SAVEFILE_getLibrarySize(),
               SyncError::SYNC_ERROR_NONE);

        setState(SyncState::SYNC_STATE_SELECTING_SONG);
      }

      break;
    }
    case SyncState::SYNC_STATE_SELECTING_SONG: {
      if (isMaster()) {
        u16 keys = getPressedKeys();
        u16 payload = SYNCER_MSG_SELECTION_BUILD(
            KEY_DOWNLEFT(keys), KEY_UPLEFT(keys), KEY_CENTER(keys),
            KEY_UPRIGHT(keys), KEY_DOWNRIGHT(keys));
        outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_SELECTION, payload);

        ASSERT_EVENT(SYNC_EVENT_SELECTION);
      } else {
        outgoingData = SYNCER_MSG_BUILD(SYNC_EVENT_SELECTION, 0);

        ASSERT_EVENT(SYNC_EVENT_SELECTION);
        lastMessage.event = SYNC_EVENT_SELECTION;
        lastMessage.data1 = incomingPayload;
      }
      break;
    }
  }

  checkTimeout();
}

void Syncer::fail(SyncError error) {
  reset();
  this->error = error;
}

void Syncer::checkTimeout() {
  if (timeoutCount >= SYNC_TIMEOUT_FRAMES)
    reset();
}

void Syncer::reset() {
  playerId = -1;
  setState(SyncState::SYNC_STATE_SEND_ROM_ID);
  outgoingData = 0;
  lastMessage.event = 0;
}

void Syncer::resetError() {
  error = SyncError::SYNC_ERROR_NONE;
}
