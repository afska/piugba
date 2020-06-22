#ifndef GSM_PLAYER_H
#define GSM_PLAYER_H

#define TIMER_16MHZ 0
#define FIFO_ADDR_A 0x040000a0
#define FIFO_ADDR_B 0x040000A4
#define CHANNEL_A_MUTE 0b1111110011111111
#define CHANNEL_A_UNMUTE 0b0000001100000000
#define CHANNEL_B_MUTE 0b1100111111111111
#define CHANNEL_B_UNMUTE 0b0011000000000000

#define PLAYER_DEFINE(DMA_CNT, DMA_SAD, DMA_DAD, FIFO_ADDRESS, MUTE, UNMUTE)   \
  uint32_t fracumul(uint32_t x, uint32_t frac) __attribute__((long_call));     \
                                                                               \
  static const GBFS_FILE* fs;                                                  \
  static const unsigned char* src;                                             \
  static uint32_t src_len;                                                     \
  static const unsigned char* src_pos = NULL;                                  \
  static const unsigned char* src_end = NULL;                                  \
                                                                               \
  static struct gsm_state decoder;                                             \
  static signed short out_samples[160];                                        \
  static signed char double_buffers[2][608] __attribute__((aligned(4)));       \
  static unsigned int decode_pos = 160, cur_buffer = 0;                        \
  static signed char* dst_pos;                                                 \
  static int last_sample = 0;                                                  \
  static int i;                                                                \
                                                                               \
  static void gsm_init(gsm r) {                                                \
    memset((char*)r, 0, sizeof(*r));                                           \
    r->nrp = 40;                                                               \
  }                                                                            \
                                                                               \
  static void dsound_switch_buffers(const void* src) {                         \
    DMA_CNT = 0;                                                               \
                                                                               \
    /* no-op to let DMA registers catch up */                                  \
    asm volatile("eor r0, r0; eor r0, r0" ::: "r0");                           \
                                                                               \
    DMA_SAD = (intptr_t)src;                                                   \
    DMA_DAD = (intptr_t)FIFO_ADDRESS;                                          \
    DMA_CNT = DMA_DST_FIXED | DMA_SRC_INC | DMA_REPEAT | DMA32 | DMA_SPECIAL | \
              DMA_ENABLE | 1;                                                  \
  }                                                                            \
                                                                               \
  static void mute() { DSOUNDCTRL = DSOUNDCTRL & MUTE; }                       \
                                                                               \
  static void unmute() { DSOUNDCTRL = DSOUNDCTRL | UNMUTE; }

#define PLAYER_TURN_ON_SOUND() \
  SETSNDRES(1);                \
  SNDSTAT = SNDSTAT_ENABLE;    \
  DSOUNDCTRL = 0b1111101100001110;

#define PLAYER_INIT(TM_CNT_L, TM_CNT_H)        \
  fs = find_first_gbfs_file(0);                \
                                               \
  /* TMxCNT_L is count; TMxCNT_H is control */ \
  TM_CNT_H = 0;                                \
  TM_CNT_L = 0x10000 - (924 / 2);              \
  TM_CNT_H = TIMER_16MHZ | TIMER_START;        \
                                               \
  mute();

#define PLAYER_PLAY(NAME)                 \
  gsm_init(&decoder);                     \
  src = gbfs_get_obj(fs, NAME, &src_len); \
  src_pos = src;                          \
  src_end = src + src_len;

#define PLAYER_STOP() \
  src_pos = NULL;     \
  src_end = NULL;     \
  mute();

#define PLAYER_PRE_UPDATE(ON_STOP)                    \
  dst_pos = double_buffers[cur_buffer];               \
                                                      \
  if (src_pos < src_end) {                            \
    for (i = 304 / 4; i > 0; i--) {                   \
      int cur_sample;                                 \
      if (decode_pos >= 160) {                        \
        if (src_pos < src_end) {                      \
          gsm_decode(&decoder, src_pos, out_samples); \
        }                                             \
        src_pos += sizeof(gsm_frame);                 \
        decode_pos = 0;                               \
      }                                               \
                                                      \
      /* 2:1 linear interpolation */                  \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
                                                      \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
                                                      \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
                                                      \
      cur_sample = out_samples[decode_pos++];         \
      *dst_pos++ = (last_sample + cur_sample) >> 9;   \
      *dst_pos++ = cur_sample >> 8;                   \
      last_sample = cur_sample;                       \
    }                                                 \
  } else {                                            \
    PLAYER_STOP();                                    \
    ON_STOP;                                          \
  }

#define PLAYER_POST_UPDATE()                         \
  dsound_switch_buffers(double_buffers[cur_buffer]); \
                                                     \
  if (src_pos != NULL)                               \
    unmute();                                        \
                                                     \
  cur_buffer = !cur_buffer;

#endif  // GSM_PLAYER_H
