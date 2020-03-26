#include "SongScene.h"
#include <libgba-sprite-engine/gba/tonc_core.h>  // TODO: REMOVE tonc_core (qran_range)
#include <libgba-sprite-engine/palette/palette_manager.h>
#include <libgba-sprite-engine/sprites/sprite_builder.h>
#include "data/content/BeethovenVirus.h"
#include "data/content/compiled/shared_palette.h"
#include "utils/SpriteUtils.h"

const u32 BPM = 162;
const int INITIAL_OFFSET = -175;
const u32 ARROW_POOL_SIZE = 20;

std::vector<Background*> SongScene::backgrounds() {
  return {/*bg.get()*/};
}

std::vector<Sprite*> SongScene::sprites() {
  std::vector<Sprite*> sprites;

  score->render(&sprites);

  arrowQueue->forEach([&sprites](Arrow* it) { sprites.push_back(it->get()); });

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    sprites.push_back(arrowHolders[i]->get());

  return sprites;
}

void SongScene::load() {
  // TODO: REMOVE
  Song* song = Song_parse(fs, (char*)"beethoven-virus.pius");
  log_text(std::to_string(song->sampleStart).c_str());

  foregroundPalette =
      std::unique_ptr<ForegroundPaletteManager>(new ForegroundPaletteManager(
          shared_palettePal, sizeof(shared_palettePal)));
  backgroundPalette =
      std::unique_ptr<BackgroundPaletteManager>(new BackgroundPaletteManager(
          BeethovenVirusPal, sizeof(BeethovenVirusPal)));
  SpriteBuilder<Sprite> builder;

  // setUpBackground();
  setUpArrows();

  score = std::unique_ptr<Score>{new Score()};
}

void SongScene::tick(u16 keys) {
  // 60000-----BPMbeats
  // millis-----x = millis*BPM/60000
  int millis = msecs + INITIAL_OFFSET;
  int beat = Div(millis * BPM, 60000);  // BPM bpm
  if (beat != lastBeat) {
    // SPRITE_goToFrame(arrowHolders[0]->get(),
    //                  ARROW_FRAMES * 0 + (beat % 2 == 0 ? ARROW_HOLDER_PRESSED
    //                                                    : ARROW_HOLDER_IDLE));
    // SPRITE_goToFrame(arrowHolders[1]->get(),
    //                  ARROW_FRAMES * 1 + (beat % 2 == 0 ? ARROW_HOLDER_PRESSED
    //                                                    : ARROW_HOLDER_IDLE));
    // SPRITE_goToFrame(arrowHolders[2]->get(),
    //                  ARROW_FRAMES * 2 + (beat % 2 == 0 ? ARROW_HOLDER_PRESSED
    //                                                    : ARROW_HOLDER_IDLE));
    // SPRITE_goToFrame(arrowHolders[3]->get(),
    //                  ARROW_FRAMES * 1 + (beat % 2 == 0 ? ARROW_HOLDER_PRESSED
    //                                                    : ARROW_HOLDER_IDLE));
    // SPRITE_goToFrame(arrowHolders[4]->get(),
    //                  ARROW_FRAMES * 0 + (beat % 2 == 0 ? ARROW_HOLDER_PRESSED
    //                                                    : ARROW_HOLDER_IDLE));
    // arrowQueue->push([](Arrow* it) {
    //   it->initialize(static_cast<ArrowType>(qran_range(0, 4)));
    // });
  }
  lastBeat = beat;

  score->tick();
  updateArrowHolders();
  updateArrows(millis);
  processKeys(keys);
}

void SongScene::setUpBackground() {
  engine.get()->disableText();

  bg = std::unique_ptr<Background>(
      new Background(0, BeethovenVirusTiles, sizeof(BeethovenVirusTiles),
                     BeethovenVirusMap, sizeof(BeethovenVirusMap)));
  bg.get()->useMapScreenBlock(24);
}

void SongScene::setUpArrows() {
  arrowQueue = std::unique_ptr<ObjectQueue<Arrow>>{new ObjectQueue<Arrow>(
      ARROW_POOL_SIZE, [](u32 id) -> Arrow* { return new Arrow(id); })};

  for (u32 i = 0; i < ARROWS_TOTAL; i++)
    arrowHolders.push_back(std::unique_ptr<ArrowHolder>{
        new ArrowHolder(static_cast<ArrowType>(i))});
}

void SongScene::updateArrowHolders() {
  for (auto& it : arrowHolders)
    it->tick();
}

void SongScene::updateArrows(u32 millis) {
  arrowQueue->forEachActive([this, &millis](Arrow* it) {
    bool isPressed = arrowHolders[it->type]->get()->getCurrentFrame() !=
                     ARROW_HOLDER_IDLE;  // TODO: Extract logic

    FeedbackType feedbackType = it->tick(millis, isPressed);
    if (feedbackType < FEEDBACK_TOTAL_SCORES)
      score->update(feedbackType);
    if (feedbackType == FeedbackType::INACTIVE)
      arrowQueue->pop();
  });
}

void SongScene::processKeys(u16 keys) {
  if ((keys & KEY_DOWN) && arrowHolders[0]->get()->getCurrentFrame() ==
                               ARROW_FRAMES * 0 + ARROW_HOLDER_IDLE) {
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::DOWNLEFT); });
  }

  if ((keys & KEY_L) && arrowHolders[1]->get()->getCurrentFrame() ==
                            ARROW_FRAMES * 1 + ARROW_HOLDER_IDLE) {
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::UPLEFT); });
  }

  if ((((keys & KEY_B) | (keys & KEY_RIGHT))) &&
      arrowHolders[2]->get()->getCurrentFrame() ==
          ARROW_FRAMES * 2 + ARROW_HOLDER_IDLE) {
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::CENTER); });
  }

  if ((keys & KEY_R) && arrowHolders[3]->get()->getCurrentFrame() ==
                            ARROW_FRAMES * 1 + ARROW_HOLDER_IDLE) {
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::UPRIGHT); });
  }

  if ((keys & KEY_A) && arrowHolders[4]->get()->getCurrentFrame() ==
                            ARROW_FRAMES * 0 + ARROW_HOLDER_IDLE) {
    arrowQueue->push([](Arrow* it) { it->initialize(ArrowType::DOWNRIGHT); });
  }

  SPRITE_goToFrame(arrowHolders[0]->get(),
                   ARROW_FRAMES * 0 + (keys & KEY_DOWN ? ARROW_HOLDER_PRESSED
                                                       : ARROW_HOLDER_IDLE));
  SPRITE_goToFrame(arrowHolders[1]->get(),
                   ARROW_FRAMES * 1 + (keys & KEY_L ? ARROW_HOLDER_PRESSED
                                                    : ARROW_HOLDER_IDLE));
  SPRITE_goToFrame(arrowHolders[2]->get(),
                   ARROW_FRAMES * 2 + ((keys & KEY_B) | (keys & KEY_RIGHT)
                                           ? ARROW_HOLDER_PRESSED
                                           : ARROW_HOLDER_IDLE));
  SPRITE_goToFrame(arrowHolders[3]->get(),
                   ARROW_FRAMES * 1 + (keys & KEY_R ? ARROW_HOLDER_PRESSED
                                                    : ARROW_HOLDER_IDLE));
  SPRITE_goToFrame(arrowHolders[4]->get(),
                   ARROW_FRAMES * 0 + (keys & KEY_A ? ARROW_HOLDER_PRESSED
                                                    : ARROW_HOLDER_IDLE));
}
