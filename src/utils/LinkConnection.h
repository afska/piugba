#ifndef LINK_CONNECTION_H
#define LINK_CONNECTION_H

#include <tonc_core.h>
#include <tonc_memmap.h>

#define LINK_MAX_PLAYERS 4
#define LINK_NO_DATA 0xffff
#define LINK_BAUD_RATE_MAX 3
#define LINK_WAIT_VCOUNT 10
#define LINK_HEART_BIT 0xf
#define LINK_HEART_BIT_UNKNOWN 2
#define LINK_BIT_SLAVE 2
#define LINK_BIT_READY 3
#define LINK_BITS_PLAYER_ID 4
#define LINK_BIT_ERROR 6
#define LINK_BIT_START 7
#define LINK_BIT_MULTIPLAYER 13
#define LINK_BIT_IRQ 14

// A Link Cable connection for Multi-player mode.

// Usage:
// - 1) Include this header in your main.cpp file and add:
//       LinkConnection* linkConnection = new LinkConnection();
// - 2) Add the interrupt service routine:
//       irq_add(II_SERIAL, ISR_serial);
// - 3) Add to your update loop:
//       auto linkState = linkConnection->tick(data);
// - 4) Use `linkState` to process updates

// `data` restrictions:
// - 0xFFFF means 'disconnected', so don't use it
// - Bit 0xF will be ignored: it'll be used as a heartbeat

void ISR_serial();
u16 _withHeartBit(u16 data, bool heartBit);
bool _isBitHigh(u16 data, u8 bit);

struct LinkState {
  u8 playerCount;
  u8 currentPlayerId;
  u16 data[4];
  u8 _heartBits[4];
  u32 _tick;
  u32 _lastIRQTick;

  bool isConnected() { return playerCount > 1; }
  bool hasData(u8 playerId) { return data[playerId] != LINK_NO_DATA; }

  bool _isOutOfSync() {
    return isConnected() &&
           (_lastIRQTick != _tick || currentPlayerId >= playerCount);
  }
  void _sync() { _lastIRQTick = _tick; }
  void _reset() {
    playerCount = 0;
    currentPlayerId = 0;
    for (u32 i = 0; i < LINK_MAX_PLAYERS; i++) {
      data[i] = LINK_NO_DATA;
      _heartBits[i] = LINK_HEART_BIT_UNKNOWN;
    }
    _tick = 0;
    _lastIRQTick = 0;
  }
};

class LinkConnection {
 public:
  struct LinkState _linkState;

  LinkConnection() {}

  LinkState tick(u16 data) {
    if (!isReady() || _linkState._isOutOfSync()) {
      reset();
      return _linkState;
    }

    bool heartBit = getNewHeartBit();
    setNewHeartBit(heartBit);
    REG_SIOMLT_SEND = _withHeartBit(data, heartBit);
    _linkState._tick++;

    wait(LINK_WAIT_VCOUNT);

    if (!isBitHigh(LINK_BIT_SLAVE))
      setBitHigh(LINK_BIT_START);

    return _linkState;
  }

  bool isReady() {
    return isBitHigh(LINK_BIT_READY) && !isBitHigh(LINK_BIT_ERROR);
  }

  void wait(u32 verticalLines) {
    u32 lines = 0;
    u32 vCount = REG_VCOUNT;

    while (lines < verticalLines) {
      if (REG_VCOUNT != vCount) {
        lines++;
        vCount = REG_VCOUNT;
      }
    };
  }

 private:
  bool getNewHeartBit() {
    return _linkState.isConnected()
               ? !_linkState._heartBits[_linkState.currentPlayerId]
               : 1;
  }

  void setNewHeartBit(bool heartBit) {
    if (_linkState.isConnected())
      _linkState._heartBits[_linkState.currentPlayerId] = heartBit;
  }

  void reset() {
    _linkState._reset();

    // switching to another mode and going back resets the communication circuit
    REG_RCNT = 0xf;
    REG_RCNT = 0;
    REG_SIOCNT = LINK_BAUD_RATE_MAX;
    setBitHigh(LINK_BIT_MULTIPLAYER);
    setBitHigh(LINK_BIT_IRQ);
  }

  bool isBitHigh(u8 bit) { return _isBitHigh(REG_SIOCNT, bit); }
  void setBitHigh(u8 bit) { REG_SIOCNT |= 1 << bit; }
};

extern LinkConnection* linkConnection;

inline void ISR_serial() {
  u8 currentPlayerId =
      (REG_SIOCNT & (0b11 << LINK_BITS_PLAYER_ID)) >> LINK_BITS_PLAYER_ID;

  linkConnection->_linkState.playerCount = 0;
  linkConnection->_linkState.currentPlayerId = currentPlayerId;

  for (u32 i = 0; i < LINK_MAX_PLAYERS; i++) {
    auto data = REG_SIOMULTI[i];
    u8 oldHeartBit = linkConnection->_linkState._heartBits[i];
    u8 newHeartBit = _isBitHigh(data, LINK_HEART_BIT);

    bool isConnectionAlive =
        data != LINK_NO_DATA &&
        (i == currentPlayerId || oldHeartBit == LINK_HEART_BIT_UNKNOWN ||
         oldHeartBit != newHeartBit);

    linkConnection->_linkState.data[i] =
        isConnectionAlive ? _withHeartBit(data, 0) : LINK_NO_DATA;
    linkConnection->_linkState._heartBits[i] =
        isConnectionAlive ? newHeartBit : LINK_HEART_BIT_UNKNOWN;

    if (linkConnection->_linkState.hasData(i))
      linkConnection->_linkState.playerCount++;
  }

  linkConnection->_linkState._sync();
}

inline bool _isBitHigh(u16 data, u8 bit) {
  return (data >> bit) & 1;
}

inline u16 _withHeartBit(u16 data, bool heartBit) {
  return (data & ~(1 << LINK_HEART_BIT)) | (heartBit << LINK_HEART_BIT);
}

#endif  // LINK_CONNECTION_H
