#include "Sequence.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include <functional>

#include "Key.h"
#include "SequenceMessages.h"
#include "gameplay/Library.h"
#include "scenes/CalibrateScene.h"
#include "scenes/ControlsScene.h"
#include "scenes/SelectionScene.h"
#include "scenes/SongScene.h"
#include "scenes/StartScene.h"
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

Scene* SEQUENCE_getInitialScene() {
  SAVEFILE_initialize(_fs);

  if (!SAVEFILE_isWorking(_fs)) {
    auto scene = new TalkScene(_engine, _fs, SRAM_TEST_FAILED, [](u16 keys) {});
    scene->withButton = false;
    return scene;
  }

  bool isPlaying = SAVEFILE_read8(SRAM->state.isPlaying);
  SAVEFILE_write8(SRAM->state.isPlaying, 0);

  if (isPlaying)
    return new SelectionScene(_engine, _fs);

  return new ControlsScene(_engine, _fs,
                           []() { goTo(SEQUENCE_getCalibrateOrMainScene()); });
}

Scene* SEQUENCE_getCalibrateOrMainScene() {
  bool isAudioLagCalibrated = SAVEFILE_read8(SRAM->memory.isAudioLagCalibrated);

  if (!isAudioLagCalibrated) {
    auto scene = new TalkScene(_engine, _fs, CALIBRATE_AUDIO_LAG, [](u16 keys) {
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
    scene->withButton = false;
    return scene;
  }

  return SEQUENCE_getMainScene();
}

Scene* SEQUENCE_getMainScene() {
  return new StartScene(_engine, _fs);
}

void SEQUENCE_goToGameMode(GameMode gameMode) {
  bool isArcadeModeUnlocked = SAVEFILE_isModeUnlocked(GameMode::ARCADE);
  bool isImpossibleModeUnlocked = SAVEFILE_isModeUnlocked(GameMode::IMPOSSIBLE);

  if (gameMode == GameMode::ARCADE && !isArcadeModeUnlocked) {
    goTo(new TalkScene(
        _engine, _fs, ARCADE_MODE_LOCKED,
        [](u16 keys) {
          if (KEY_CENTER(keys))
            goTo(new StartScene(_engine, _fs));
        },
        true));
    return;
  }

  if (gameMode == GameMode::IMPOSSIBLE && !isImpossibleModeUnlocked) {
    goTo(new TalkScene(
        _engine, _fs, IMPOSSIBLE_MODE_LOCKED,
        [](u16 keys) {
          if (KEY_CENTER(keys))
            goTo(new StartScene(_engine, _fs));
        },
        true));
    return;
  }

  auto lastGameMode =
      static_cast<GameMode>(SAVEFILE_read8(SRAM->state.gameMode));
  if (lastGameMode != gameMode) {
    auto songIndex =
        gameMode == GameMode::ARCADE ? 0 : SAVEFILE_getLibrarySize() - 1;
    SAVEFILE_write8(SRAM->memory.pageIndex, Div(songIndex, PAGE_SIZE));
    SAVEFILE_write8(SRAM->memory.songIndex, DivMod(songIndex, PAGE_SIZE));
  }

  SAVEFILE_write8(SRAM->state.gameMode, gameMode);
  auto message =
      gameMode == GameMode::CAMPAIGN
          ? MODE_CAMPAIGN
          : gameMode == GameMode::ARCADE ? MODE_ARCADE : MODE_IMPOSSIBLE;
  goTo(new TalkScene(
      _engine, _fs, message,
      [](u16 keys) {
        if (KEY_CENTER(keys))
          goTo(new SelectionScene(_engine, _fs));
      },
      true));
}

void SEQUENCE_goToMessageOrSong(Song* song, Chart* chart) {
  auto gameMode = static_cast<GameMode>(SAVEFILE_read8(SRAM->state.gameMode));

  if (gameMode == GameMode::CAMPAIGN && song->applyTo[chart->difficulty] &&
      song->hasMessage) {
    goTo(new TalkScene(_engine, _fs, std::string(song->message),
                       [song, chart](u16 keys) {
                         if (KEY_CENTER(keys))
                           goTo(new SongScene(_engine, _fs, song, chart), 2);
                       }),
         4);
    return;
  }

  if (gameMode == GameMode::CAMPAIGN && song->id == 1) {
    goTo(new TalkScene(
             _engine, _fs, KEYS_HINT,
             [song, chart](u16 keys) {
               if (KEY_CENTER(keys))
                 goTo(new SongScene(_engine, _fs, song, chart), 2);
             },
             true),
         4);
    return;
  }

  goTo(new SongScene(_engine, _fs, song, chart), 4);
}

void SEQUENCE_goToWinOrSelection(bool isLastSong) {
  auto gameMode = static_cast<GameMode>(SAVEFILE_read8(SRAM->state.gameMode));

  if (gameMode != GameMode::ARCADE && isLastSong)
    goTo(new TalkScene(_engine, _fs,
                       gameMode == GameMode::CAMPAIGN ? WIN : WIN_IMPOSSIBLE,
                       [](u16 keys) {
                         if (KEY_CENTER(keys))
                           goTo(new SelectionScene(_engine, _fs));
                       }),
         4);
  else
    goTo(new SelectionScene(_engine, _fs));
}
