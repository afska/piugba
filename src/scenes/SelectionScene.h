#ifndef SELECTION_SCENE_H
#define SELECTION_SCENE_H

#include <libgba-sprite-engine/background/background.h>
#include <libgba-sprite-engine/gba_engine.h>
#include <libgba-sprite-engine/scene.h>
#include <libgba-sprite-engine/sprites/sprite.h>

#include <vector>

#include "gameplay/Library.h"
#include "objects/ArrowSelector.h"
#include "objects/Difficulty.h"
#include "objects/NumericProgress.h"
#include "ui/Highlighter.h"
#include "utils/PixelBlink.h"

class SelectionScene : public Scene {
 public:
  SelectionScene(std::shared_ptr<GBAEngine> engine);

  std::vector<Background*> backgrounds() override;
  std::vector<Sprite*> sprites() override;

  void load() override;
  void tick(u16 keys) override;

 private:
  u32 init = 0;
  std::vector<std::unique_ptr<SongFile>> songs;
  u32 page = 0;
  u32 selected = 0;
  u32 count = 0;
  bool confirmed = false;
  u32 blendAlpha = HIGHLIGHTER_OPACITY;
  std::unique_ptr<Background> bg;
  std::unique_ptr<PixelBlink> pixelBlink;
  std::vector<std::unique_ptr<ArrowSelector>> arrowSelectors;
  std::unique_ptr<Difficulty> difficulty;
  std::unique_ptr<NumericProgress> progress;

  void setUpSpritesPalette();
  void setUpBackground();
  void setUpArrows();
  void setUpPager();

  void goToSong();
  SongFile* getSelectedSong();
  u32 getSelectedSongIndex();
  u32 getPageStart();

  void processKeys(u16 keys);
  void processDifficultyChangeEvents();
  void processSelectionChangeEvents();
  bool onDifficultyChange(u32 selector, DifficultyLevel newValue);
  bool onSelectionChange(u32 selector,
                         bool isOnListEdge,
                         bool isOnPageEdge,
                         int direction);

  void updateSelection();
  void confirm();
  void unconfirm();
  void setPage(u32 page, int direction);
  void setNames(std::string title, std::string artist);
};

#endif  // SELECTION_SCENE_H
