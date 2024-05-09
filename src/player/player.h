#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#define RATE_LEVELS 3
#define AUDIO_SYNC_LIMIT 2

void player_init();
void player_unload();
bool player_play(const char* name, bool forceGSM);
bool player_playSfx(const char* name);
void player_enableLoop();
void player_seek(unsigned int msecs);
void player_setRate(int rate);
void player_stop();
bool player_isPlaying();
void player_onVBlank();
void player_forever(int (*onUpdate)(),
                    void (*onRender)(),
                    void (*onAudioChunks)(unsigned int current),
                    void (*onError)());

#endif  // PLAYER_H
