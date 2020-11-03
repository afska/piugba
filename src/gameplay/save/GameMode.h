#ifndef GAME_MODE_H
#define GAME_MODE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum GameMode : u8 { CAMPAIGN, ARCADE, IMPOSSIBLE, MULTI_VS, MULTI_COOP };

#define IS_MULTIPLAYER(GAME_MODE) \
  (GAME_MODE == GameMode::MULTI_VS || GAME_MODE == GameMode::MULTI_COOP)

#endif  // GAME_MODE_H
