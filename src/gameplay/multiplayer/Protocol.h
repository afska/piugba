#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
  F: Reserved (always 0)
  B~E: Event (16 values)
  1~A: Payload (1024 values)
  0: Reserved (always 0)
*/
#define SYNCER_EVENT_MASK 0b1111
#define SYNCER_PAYLOAD_MASK 0b1111111111
#define SYNCER_MSG_BUILD(EVENT, PAYLOAD) \
  (((EVENT)&SYNCER_EVENT_MASK) << 11) | (((PAYLOAD)&SYNCER_PAYLOAD_MASK) << 1)
#define SYNCER_MSG_EVENT(MSG) (((MSG) >> 11) & SYNCER_EVENT_MASK)
#define SYNCER_MSG_PAYLOAD(MSG) (((MSG) >> 1) & SYNCER_PAYLOAD_MASK)

// Message struct (`data` fields are parts of payload)
struct Message {
  u8 event;
  u16 data1;
  u16 data2;
  u16 data3;
};

// Message IDs:
#define SYNC_EVENT_ROM_ID 1
#define SYNC_EVENT_PROGRESS 2
#define SYNC_EVENT_SELECTION 3
#define SYNC_EVENT_SONG 4

/*
  9: Mode (0 = VS, 1 = Co-op)
  7~8: Library type (0 = Normal, 1 = Hard, 2 = Crazy)
  0~6: Completed songs (0 ~ librarySize)
*/
#define SYNCER_MSG_PROGRESS_BUILD(MODE, LIBRARY_TYPE, COMPLETED_SONGS) \
  (((MODE) << 9) | ((LIBRARY_TYPE) << 7) | (COMPLETED_SONGS))
#define SYNCER_MSG_PROGRESS_MODE(MSG) (((MSG) >> 9) & 1)
#define SYNCER_MSG_PROGRESS_LIBRARY_TYPE(MSG) (((MSG) >> 7) & 0b11)
#define SYNCER_MSG_PROGRESS_COMPLETED_SONGS(MSG) ((MSG)&0b1111111)

/*
  4: DOWNRIGHT
  3: UPRIGHT
  2: CENTER
  1: UPLEFT
  0: DOWNLEFT
*/
#define SYNCER_MSG_SELECTION_BUILD(DL, UL, C, UR, DR)                    \
  ((!!(DR)) << 4) | ((!!(UR)) << 3) | ((!!(C)) << 2) | ((!!(UL)) << 1) | \
      (!!(DL))
#define SYNCER_MSG_SELECTION_DIRECTION(MSG, DIRECTION) \
  (((MSG) >> DIRECTION) & 1)

#endif  // PROTOCOL_H
