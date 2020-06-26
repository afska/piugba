#ifndef PLAYER_H
#define PLAYER_H

void player_init();
void player_play(const char* name);
void player_stop();
void player_stopAll();
void player_forever(void (*update)());

#endif  // PLAYER_H
