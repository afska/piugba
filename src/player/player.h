#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#define RATE_LEVELS 3

void player_init();
void player_play(const char* name);
void player_loop(const char* name);
void player_seek(unsigned int msecs);
void player_setRate(int rate);
void player_stop();
bool player_isPlaying();
void player_onVBlank();
void player_forever(bool (*update)());

#endif  // PLAYER_H
