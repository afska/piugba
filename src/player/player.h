#ifndef PLAYER_H
#define PLAYER_H

void player_init();
void player_play(char* name);
void player_forever(void (*update)());

#endif  // PLAYER_H
