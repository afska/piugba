#ifndef GAME_MODE_H
#define GAME_MODE_H

#include <libgba-sprite-engine/gba/tonc_core.h>

enum GameMode : u8 {
  CAMPAIGN,
  ARCADE,
  IMPOSSIBLE,
  MULTI_VS,
  MULTI_COOP,
  DEATH_MIX
};

#define IS_STORY(GAME_MODE) \
  (GAME_MODE == GameMode::CAMPAIGN || GAME_MODE == GameMode::IMPOSSIBLE)

#define IS_MULTIPLAYER(GAME_MODE) \
  (GAME_MODE == GameMode::MULTI_VS || GAME_MODE == GameMode::MULTI_COOP)

#define IS_ARCADE(GAME_MODE) \
  (GAME_MODE == GameMode::ARCADE || IS_MULTIPLAYER(GAME_MODE))

#define IS_CHALLENGE(GAME_MODE) \
  (GAME_MODE == GameMode::IMPOSSIBLE || GAME_MODE == GameMode::DEATH_MIX)

#endif  // GAME_MODE_H
