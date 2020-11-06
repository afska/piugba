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

/*
  9: Mode (0 = VS, 1 = Co-op)
  6~8: Library type (0 = Normal, 1 = Hard, 2 = Crazy)
  0~6: Completed songs (0 ~ librarySize)
*/
#define SYNCER_MSG_PROGRESS_BUILD(MODE, LIBRARY_TYPE, COMPLETED_SONGS) \
  (((MODE) << 9) | ((LIBRARY_TYPE) << 7) | (COMPLETED_SONGS))
#define SYNCER_MSG_PROGRESS_MODE(MSG) (((MSG) >> 9) & 1)
#define SYNCER_MSG_PROGRESS_LIBRARY_TYPE(MSG) (((MSG) >> 6) & 0b111)
#define SYNCER_MSG_PROGRESS_COMPLETED_SONGS(MSG) ((MSG)&0b1111111)
