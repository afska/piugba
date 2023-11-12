#include "LinkWireless.h"

#include "objects/ArrowInfo.h"

// [!]

void LinkWireless::_onVBlank() {
  if (!isEnabled)
    return;

  if (!isSessionActive()) {
    copyState();
    return;
  }

  if (isConnected() && sessionState.frameRecvCount == 0)
    sessionState.recvTimeout++;

  sessionState.frameRecvCount = 0;
  sessionState.acceptCalled = false;
  sessionState.pingSent = false;

  copyState();
}

CODE_IWRAM void LinkWireless::copyIncomingState() {
  if (!isReadingMessages) {
    while (!sessionState.tmpMessagesToReceive.isEmpty()) {
      auto message = sessionState.tmpMessagesToReceive.pop();

      if (state == SERVING || state == CONNECTED)
        sessionState.incomingMessages.push(message);
    }
  }
}

CODE_IWRAM void LinkWireless::_onACKTimer() {
  if (!isEnabled || !asyncCommand.isActive ||
      asyncCommand.ackStep == AsyncCommand::ACKStep::READY)
    return;

  if (asyncCommand.ackStep == AsyncCommand::ACKStep::WAITING_FOR_HIGH) {
    if (!linkSPI->_isSIHigh())
      return;

    linkSPI->_setSOHigh();
    asyncCommand.ackStep = AsyncCommand::ACKStep::WAITING_FOR_LOW;
  } else if (asyncCommand.ackStep == AsyncCommand::ACKStep::WAITING_FOR_LOW) {
    if (linkSPI->_isSIHigh())
      return;

    linkSPI->_setSOLow();
    asyncCommand.ackStep = AsyncCommand::ACKStep::READY;
    _stopACKTimer();

    if (asyncCommand.state == AsyncCommand::State::PENDING) {
      updateAsyncCommand(asyncCommand.pendingData);

      if (asyncCommand.state == AsyncCommand::State::COMPLETED)
        processAsyncCommand();
    }
  }
}

CODE_IWRAM void LinkWireless::_onSerial() {
  if (!isEnabled)
    return;

  linkSPI->_onSerial(true);

  bool hasNewData = linkSPI->getAsyncState() == LinkSPI::AsyncState::READY;
  u32 newData = linkSPI->getAsyncData();

  if (!isSessionActive())
    return;

  if (asyncCommand.isActive) {
    if (asyncCommand.ackStep != AsyncCommand::ACKStep::READY)
      return;

    if (hasNewData) {
      linkSPI->_setSOLow();
      asyncCommand.ackStep = AsyncCommand::ACKStep::WAITING_FOR_HIGH;
      asyncCommand.pendingData = newData;
      _startACKTimer();
    } else
      return;
  }
}

void LinkWireless::_onTimer() {
  if (!isEnabled)
    return;

  if (!isSessionActive())
    return;

  if (sessionState.recvTimeout >= config.timeout) {
    reset();
    lastError = TIMEOUT;
    return;
  }

  if (!asyncCommand.isActive)
    acceptConnectionsOrSendData();
}
