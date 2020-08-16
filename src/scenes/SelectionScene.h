#ifndef SELECTION_SCENE_H
#define SELECTION_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "gameplay/Library.h"
#include "objects/ui/ArrowSelector.h"
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

const u32 PAGE_SIZE = 4;

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
  u32 page = 0;
  u32 selected = 0;
  u32 count = 0;
  bool confirmed = false;
  u32 blendAlpha = HIGHLIGHTER_OPACITY;

  inline SongFile* getSelectedSong() { return songs[selected].get(); }
  inline u32 getSelectedSongIndex() { return page * PAGE_SIZE + selected; }
  inline u32 getPageStart() { return page * PAGE_SIZE; }
  inline u32 getLastUnlockedSongIndex() {
    return min(getCompletedSongs(), count - 1);
  }
  inline u32 getCompletedSongs() {
    return SAVEFILE_read32(
        SRAM->progress[difficulty->getValue()].completedSongs);
  }

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpArrows();
  void setUpChannelBadges();
  void setUpGradeBadges();
  void setUpLocks();
  void setUpPager();

  void scrollTo(u32 page, u32 selected);
  void goToSong();

  void processKeys(u16 keys);
  void processDifficultyChangeEvents();
  void processSelectionChangeEvents();
  void processMultiplierChangeEvents();
  void processConfirmEvents();
  void processMenuEvents(u16 keys);
  bool onDifficultyChange(ArrowDirection selector, DifficultyLevel newValue);
  bool onSelectionChange(ArrowDirection selector,
                         bool isOnListEdge,
                         bool isOnPageEdge,
                         int direction);

  void updateSelection();
  void confirm();
  void unconfirm();
  void setPage(u32 page, int direction);
  void loadChannels();
  void loadProgress();
  void setNames(std::string title, std::string artist);

  ~SelectionScene();
};

#endif  // SELECTION_SCENE_H
