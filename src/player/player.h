#ifndef PLAYER_H
#define PLAYER_H

void player_init();
void player_play(const char* name);
void player_loop(const char* name);
void player_seek(unsigned int msecs);
void player_stop();
void player_stopAll();
void player_forever(void (*update)());

void fxes_playSolo(const char* name);

#endif  // PLAYER_H
