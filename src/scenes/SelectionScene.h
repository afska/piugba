#ifndef SELECTION_SCENE_H
#define SELECTION_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "gameplay/Library.h"
#include "objects/ArrowSelector.h"
#include "objects/ChannelBadge.h"
#include "objects/Difficulty.h"
#include "objects/GradeBadge.h"
#include "objects/NumericProgress.h"
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
  std::unique_ptr<Difficulty> difficulty;
  std::unique_ptr<NumericProgress> progress;
  u32 page = 0;
  u32 selected = 0;
  u32 count = 0;
  bool confirmed = false;
  u32 blendAlpha = HIGHLIGHTER_OPACITY;

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpArrows();
  void setUpChannelBadges();
  void setUpGradeBadges();
  void setUpPager();

  void goToSong();
  SongFile* getSelectedSong();
  u32 getSelectedSongIndex();
  u32 getPageStart();

  void processKeys(u16 keys);
  void processDifficultyChangeEvents();
  void processSelectionChangeEvents();
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
};

#endif  // SELECTION_SCENE_H
