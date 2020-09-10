#ifndef SELECTION_SCENE_H
#define SELECTION_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "gameplay/Library.h"
#include "gameplay/save/SaveFile.h"
#include "objects/base/InputHandler.h"
#include "objects/ui/ArrowSelector.h"
#include "objects/ui/Button.h"
#include "objects/ui/ChannelBadge.h"
#include "objects/ui/Difficulty.h"
#include "objects/ui/GradeBadge.h"
#include "objects/ui/Lock.h"
#include "objects/ui/Multiplier.h"
#include "objects/ui/NumericProgress.h"
#include "ui/Highlighter.h"
#include "utils/PixelBlink.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

class SelectionScene : public Scene {
 public:
  SelectionScene(std::shared_ptr<GBAEngine> engine, const GBFS_FILE* fs);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  u32 init = 0;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  const GBFS_FILE* fs;

  std::unique_ptr<Library> library;
  std::vector<std::unique_ptr<SongFile>> songs;
  std::vector<std::unique_ptr<ArrowSelector>> arrowSelectors;
  std::vector<std::unique_ptr<ChannelBadge>> channelBadges;
  std::vector<std::unique_ptr<GradeBadge>> gradeBadges;
  std::vector<std::unique_ptr<Lock>> locks;
  std::unique_ptr<Difficulty> difficulty;
  std::unique_ptr<Multiplier> multiplier;
  std::unique_ptr<NumericProgress> progress;
  std::unique_ptr<InputHandler> settingsMenuInput;
  std::unique_ptr<Button> numericLevelBadge;
  std::vector<u8> numericLevels;
  u32 page = 0;
  u32 selected = 0;
  u32 count = 0;
  bool confirmed = false;
  u32 blendAlpha = HIGHLIGHTER_OPACITY;

  inline GameMode getGameMode() {
    return static_cast<GameMode>(SAVEFILE_read8(SRAM->state.gameMode));
  }
  inline SongFile* getSelectedSong() { return songs[selected].get(); }
  inline u32 getSelectedSongIndex() { return getPageStart() + selected; }
  inline u32 getPageStart() { return page * PAGE_SIZE; }
  inline u32 getLastUnlockedSongIndex() {
    return getGameMode() == GameMode::ARCADE
               ? min(getCompletedSongs() - 1, count - 1)
               : min(getCompletedSongs(), count - 1);
  }
  inline u32 getCompletedSongs() {
    return getGameMode() == GameMode::ARCADE
               ? SAVEFILE_getCompletedSongs()
               : getCompletedSongsOf(difficulty->getValue());
  }
  inline u32 getCompletedSongsOf(DifficultyLevel difficultyLevel) {
    switch (getGameMode()) {
      case GameMode::CAMPAIGN:
      case GameMode::ARCADE: {
        return SAVEFILE_read8(SRAM->progress[difficultyLevel].completedSongs);
      }
      case GameMode::IMPOSSIBLE: {
        return SAVEFILE_read8(
            SRAM->progress[PROGRESS_IMPOSSIBLE + difficultyLevel]
                .completedSongs);
      }
    }

    return 0;
  }
  inline u8 getSelectedNumericLevel() {
    return numericLevels[getSelectedNumericLevelIndex()];
  }
  inline u8 getSelectedNumericLevelIndex() {
    return SAVEFILE_read8(SRAM->memory.numericLevel);
  }
  inline void setClosestNumericLevel(u8 level) {
    u32 min = 0;
    u32 minDiff = abs((int)numericLevels[0] - (int)level);
    for (u32 i = 0; i < numericLevels.size(); i++) {
      u32 diff = (u32)abs((int)numericLevels[i] - (int)level);
      if (diff < minDiff) {
        min = i;
        minDiff = diff;
      }
    }

    SAVEFILE_write8(SRAM->memory.numericLevel, min);
  }

  inline DifficultyLevel getLibraryType() {
    if (getGameMode() != GameMode::ARCADE)
      return difficulty->getValue();

    DifficultyLevel maxLevel;
    u32 max = 0;

    for (u32 i = 0; i < MAX_DIFFICULTY + 1; i++) {
      auto difficultyLevel = static_cast<DifficultyLevel>(i);
      auto completedSongs = getCompletedSongsOf(difficultyLevel);

      if (completedSongs >= max) {
        maxLevel = difficultyLevel;
        max = completedSongs;
      }
    }

    return maxLevel;
  }

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpArrows();
  void setUpChannelBadges();
  void setUpGradeBadges();
  void setUpLocks();
  void setUpPager();

  void scrollTo(u32 songIndex);
  void scrollTo(u32 page, u32 selected);
  void goToSong();

  void processKeys(u16 keys);
  void processDifficultyChangeEvents();
  void processSelectionChangeEvents();
  void processConfirmEvents();
  void processMenuEvents(u16 keys);
  bool onDifficultyLevelChange(ArrowDirection selector,
                               DifficultyLevel newValue);
  bool onNumericLevelChange(ArrowDirection selector, u8 newValue);
  bool onSelectionChange(ArrowDirection selector,
                         bool isOnListEdge,
                         bool isOnPageEdge,
                         int direction);

  void updateSelection() { updateSelection(false); }
  void updateSelection(bool isChangingLevel);
  void confirm();
  void unconfirm();
  void setPage(u32 page, int direction);
  void loadChannels();
  void loadProgress();
  void setNames(std::string title, std::string artist);
  void printNumericLevel(DifficultyLevel difficulty) {
    printNumericLevel(difficulty, 0);
  }
  void printNumericLevel(DifficultyLevel difficulty, s8 offset);

  ~SelectionScene();
};

#endif  // SELECTION_SCENE_H
