#include <libgba-sprite-engine/gba_engine.h>
#include <tonc.h>

#include "gameplay/save/SaveFile.h"
#include "scenes/ControlsScene.h"
#include "scenes/SelectionScene.h"

extern "C" {
#include "player/player.h"
}

void setUpInterrupts();
static std::shared_ptr<GBAEngine> engine{new GBAEngine()};

static const GBFS_FILE* fs = find_first_gbfs_file(0);

int main() {
  setUpInterrupts();
  player_init();
  SAVEFILE_initialize(fs);

  bool showControls = SAVEFILE_read8(SRAM->settings.showControls);
  engine->setScene(showControls ? (Scene*)new ControlsScene(engine, fs)
                                : (Scene*)new SelectionScene(engine, fs));
  player_forever([]() { engine->update(); });

  return 0;
}

void ISR_reset() {
  RegisterRamReset(RESET_REG | RESET_VRAM);
  SoftReset();
  REG_IF = IRQ_KEYPAD;
}

void setUpInterrupts() {
  // VBlank
  irq_init(NULL);
  irq_add(II_VBLANK, NULL);

  // A+B+START+SELECT
  REG_KEYCNT = 0b1100000000001111;
  irq_add(II_KEYPAD, ISR_reset);
}
