#include "Syncer.h"

#include "gameplay/Key.h"

#define ASSERT(CONDITION, FAILURE_REASON) \
  if (!(CONDITION)) {                     \
    fail(FAILURE_REASON);                 \
    return;                               \
  }

#define ASSERT_EVENT(EXPECTED_EVENT, LOG)  \
  if (incomingEvent != (EXPECTED_EVENT)) { \
    timeoutCount++;                        \
    break;                                 \
  } else {                                 \
    DEBUTRACE((LOG));                      \
    timeoutCount = 0;                      \
  }

void Syncer::initialize(SyncMode mode) {
  if (mode != SyncMode::SYNC_MODE_OFFLINE)
    linkConnection->activate();
  else
    linkConnection->deactivate();

  this->mode = mode;
  reset();
  resetError();
}

void Syncer::update() {
  if (mode == SyncMode::SYNC_MODE_OFFLINE)
    return;

  LinkState* linkState = linkConnection->linkState.get();

#ifdef SENV_DEBUG
  if (!linkState->isConnected())
    DEBUTRACE("disconnected...");
#endif

  ASSERT(linkState->isConnected(), SyncError::SYNC_ERROR_NONE);
  ASSERT(linkState->playerCount == 2, SyncError::SYNC_ERROR_TOO_MANY_PLAYERS);

  if (!isActive()) {
    reset();
    playerId = linkState->currentPlayerId;

#ifdef SENV_DEBUG
    DEBUTRACE("* init: player " + DSTR(playerId));
#endif
  }

  if (isReady())
    resetError();

  do {
    sync(linkState);
  } while (isActive() && linkState->isConnected() &&
           linkState->hasMessage(!playerId));
}

void Syncer::sync(LinkState* linkState) {
  u16 incomingData = linkState->readMessage(!playerId);
  u8 incomingEvent = SYNCER_MSG_EVENT(incomingData);
  u16 incomingPayload = SYNCER_MSG_PAYLOAD(incomingData);

#ifdef SENV_DEBUG
  DEBUTRACE("(" + DSTR(state) + ")<- " + DSTR(incomingData) + " (" +
            DSTR(incomingEvent) + "-" + DSTR(incomingPayload) + ")");
#endif

  outgoingEvent = LINK_NO_DATA;
  outgoingPayload = LINK_NO_DATA;

  switch (state) {
    case SyncState::SYNC_STATE_SEND_ROM_ID: {
      outgoingEvent = SYNC_EVENT_ROM_ID;
      outgoingPayload = getPartialRomId();

      ASSERT_EVENT(SYNC_EVENT_ROM_ID, "* rom id received");

      ASSERT(incomingPayload == getPartialRomId(),
             SyncError::SYNC_ERROR_ROM_MISMATCH);
      setState(SyncState::SYNC_STATE_SEND_PROGRESS);

      break;
    }
    case SyncState::SYNC_STATE_SEND_PROGRESS: {
      u8 modeBit = mode == SyncMode::SYNC_MODE_COOP;
      outgoingEvent = SYNC_EVENT_PROGRESS;
      outgoingPayload = isMaster() ? SYNCER_MSG_PROGRESS_BUILD(
                                         modeBit, SAVEFILE_getMaxLibraryType(),
                                         SAVEFILE_getMaxCompletedSongs())
                                   : modeBit;

      ASSERT_EVENT(SYNC_EVENT_PROGRESS, "* progress received");

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
               SyncError::SYNC_ERROR_WTF);
        ASSERT(receivedCompletedSongs >= 0 &&
                   receivedCompletedSongs <= SAVEFILE_getLibrarySize(),
               SyncError::SYNC_ERROR_WTF);

        setState(SyncState::SYNC_STATE_SELECTING_SONG);
      }

      break;
    }
    case SyncState::SYNC_STATE_SELECTING_SONG: {
      outgoingEvent = SYNC_EVENT_SELECTION;

      if (isMaster()) {
        u16 keys = getPressedKeys();
        outgoingPayload = SYNCER_MSG_SELECTION_BUILD(
            KEY_DOWNLEFT(keys), KEY_UPLEFT(keys), KEY_CENTER(keys),
            KEY_UPRIGHT(keys), KEY_DOWNRIGHT(keys));

        ASSERT_EVENT(SYNC_EVENT_SELECTION, "");
      } else {
        outgoingPayload = 0;

        ASSERT_EVENT(SYNC_EVENT_SELECTION, "");
        lastMessage.event = SYNC_EVENT_SELECTION;
        lastMessage.data1 = incomingPayload;
      }

      break;
    }
  }

  sendOutgoingData();
  checkTimeout();
}

void Syncer::sendOutgoingData() {
  u16 outgoingData = SYNCER_MSG_BUILD(outgoingEvent, outgoingPayload);
  linkConnection->send(outgoingData);

#ifdef SENV_DEBUG
  if (outgoingData != LINK_NO_DATA)
    DEBUTRACE("(" + DSTR(state) + ")...-> " + DSTR(outgoingData) + " (" +
              DSTR(outgoingEvent) + "-" + DSTR(outgoingPayload) + ")");
#endif
}

void Syncer::checkTimeout() {
  if (timeoutCount >= SYNC_TIMEOUT) {
#ifdef SENV_DEBUG
    DEBUTRACE("! state timeout: " + DSTR(timeoutCount));
#endif

    reset();
  }
}

void Syncer::fail(SyncError error) {
#ifdef SENV_DEBUG
  DEBUTRACE("* fail: " + DSTR(error));
#endif

  if (isActive())
    sendOutgoingData();
  reset();
  this->error = error;
}

void Syncer::reset() {
#ifdef SENV_DEBUG
  DEBUTRACE("* reset");
#endif

  playerId = -1;
  state = SyncState::SYNC_STATE_SEND_ROM_ID;
  timeoutCount = 0;
  resetData();
}

void Syncer::resetData() {
  outgoingEvent = LINK_NO_DATA;
  outgoingPayload = LINK_NO_DATA;
  lastMessage.event = 0;
}

void Syncer::resetError() {
  error = SyncError::SYNC_ERROR_NONE;
}
