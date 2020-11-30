#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
  General message format:
  F: Reserved (always 0)
  B~E: Event (16 values)
  1~A: Payload (1024 values)
  0: Reserved (always 0)
*/
#define SYNC_EVENT_MASK 0b1111
#define SYNC_PAYLOAD_MASK 0b1111111111
#define SYNC_MSG_BUILD(EVENT, PAYLOAD) \
  (((EVENT)&SYNC_EVENT_MASK) << 11) | (((PAYLOAD)&SYNC_PAYLOAD_MASK) << 1)
#define SYNC_MSG_EVENT(MSG) (((MSG) >> 11) & SYNC_EVENT_MASK)
#define SYNC_MSG_PAYLOAD(MSG) (((MSG) >> 1) & SYNC_PAYLOAD_MASK)

// Event IDs:
#define SYNC_EVENT_ROM_ID 1
#define SYNC_EVENT_PROGRESS 2
#define SYNC_EVENT_SONG_CHANGED 3
#define SYNC_EVENT_LEVEL_CHANGED 4
#define SYNC_EVENT_SONG_CORNER 5
#define SYNC_EVENT_START_SONG 6
#define SYNC_EVENT_KEYS 7
#define SYNC_EVENT_FEEDBACK 8
#define SYNC_EVENT_MULTIPLIER_CHANGE 9
#define SYNC_EVENT_STAGE_END 10
#define SYNC_EVENT_CONFIRM_SONG_END 11

/*
  [Progress sync]
  9: Mode (0 = VS, 1 = Co-op)
  7~8: Library type (0 = Normal, 1 = Hard, 2 = Crazy)
  0~6: Completed songs (0 ~ librarySize)
*/
#define SYNC_MSG_PROGRESS_BUILD(MODE, LIBRARY_TYPE, COMPLETED_SONGS) \
  (((MODE) << 9) | ((LIBRARY_TYPE) << 7) | (COMPLETED_SONGS))
#define SYNC_MSG_PROGRESS_MODE(MSG) (((MSG) >> 9) & 1)
#define SYNC_MSG_PROGRESS_LIBRARY_TYPE(MSG) (((MSG) >> 7) & 0b11)
#define SYNC_MSG_PROGRESS_COMPLETED_SONGS(MSG) ((MSG)&0b1111111)

/*
  [Keys sync]
  5~9: Consumed audio chunks
  4: DOWNRIGHT
  3: UPRIGHT
  2: CENTER
  1: UPLEFT
  0: DOWNLEFT
*/
#define SYNC_MSG_KEYS_BUILD(DL, UL, C, UR, DR, AUDIO_CHUNKS)             \
  ((!!(DR)) << 4) | ((!!(UR)) << 3) | ((!!(C)) << 2) | ((!!(UL)) << 1) | \
      (!!(DL)) | ((AUDIO_CHUNKS) << 5)
#define SYNC_MSG_KEYS_DIRECTION(MSG, DIRECTION) (((MSG) >> DIRECTION) & 1)
#define SYNC_MSG_KEYS_AUDIO_CHUNKS(MSG) (((MSG) >> 5) & 0b11111)

/*
  [Feedback sync]
  3: Is long note
  0~2: Feedback type (0 = Perfect, 1 = Great, etc...)
*/
#define SYNC_MSG_FEEDBACK_BUILD(TYPE, IS_LONG) (((IS_LONG) << 3) | (TYPE))
#define SYNC_MSG_FEEDBACK_TYPE(MSG) ((MSG)&0b111)
#define SYNC_MSG_FEEDBACK_IS_LONG(MSG) (((MSG) >> 3) & 1)

#endif  // PROTOCOL_H
