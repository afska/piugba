#ifndef PLAYER_H
#define PLAYER_H

void player_init();
void player_play(char* name);
void player_forever(void (*update)(char* src, char* src_pos, char* src_end));

#endif  // PLAYER_H
