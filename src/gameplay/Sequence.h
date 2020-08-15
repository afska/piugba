#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <libgba-sprite-engine/gba_engine.h>

#include <functional>

#include "Key.h"
#include "SequenceMessages.h"
#include "gameplay/save/SaveFile.h"
#include "scenes/CalibrateScene.h"
#include "scenes/ControlsScene.h"
#include "scenes/SelectionScene.h"
#include "scenes/TalkScene.h"

static std::shared_ptr<GBAEngine> _engine;
static const GBFS_FILE* _fs;

static void goTo(Scene* scene, int delay) {
  _engine->transitionIntoScene(scene, new FadeOutScene(delay));
}

static void goTo(Scene* scene) {
  goTo(scene, 2);
}

void SEQUENCE_initialize(std::shared_ptr<GBAEngine> engine,
                         const GBFS_FILE* fs) {
  _engine = engine;
  _fs = fs;
}

Scene* SEQUENCE_getMainScene() {
  return new SelectionScene(_engine, _fs);
}

Scene* SEQUENCE_getCalibrateOrMainScene() {
  bool isAudioLagCalibrated = SAVEFILE_read8(SRAM->memory.isAudioLagCalibrated);

  if (!isAudioLagCalibrated) {
    return new TalkScene(_engine, _fs, CALIBRATE_AUDIO_LAG, [](u16 keys) {
      if (keys & KEY_START) {
        SAVEFILE_write8(SRAM->memory.isAudioLagCalibrated, 1);
        goTo(new CalibrateScene(_engine, _fs,
                                []() { goTo(SEQUENCE_getMainScene()); }));
      }

      if (keys & KEY_SELECT) {
        SAVEFILE_write8(SRAM->memory.isAudioLagCalibrated, 1);
        goTo(SEQUENCE_getMainScene());
      }
    });
  }

  return SEQUENCE_getMainScene();
}

Scene* SEQUENCE_getInitialScene() {
  if (!SAVEFILE_initialize(_fs))
    return new TalkScene(_engine, _fs, SRAM_TEST_FAILED, [](u16 keys) {});

  bool isPlaying = SAVEFILE_read8(SRAM->state.isPlaying);
  bool showControls = SAVEFILE_read8(SRAM->settings.showControls) && !isPlaying;
  SAVEFILE_write8(SRAM->state.isPlaying, 0);

  if (showControls)
    return new ControlsScene(
        _engine, _fs, []() { goTo(SEQUENCE_getCalibrateOrMainScene()); });

  return SEQUENCE_getCalibrateOrMainScene();
}

#endif  // SEQUENCE_H
