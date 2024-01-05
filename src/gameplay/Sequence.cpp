#include "Sequence.h"

#include <libgba-sprite-engine/gba/tonc_bios.h>

#include <functional>
#include <string>

#include "Key.h"
#include "SequenceMessages.h"
#include "gameplay/Library.h"
#include "multiplayer/Syncer.h"
#include "scenes/CalibrateScene.h"
#include "scenes/ControlsScene.h"
#include "scenes/DeathMixScene.h"
#include "scenes/MultiplayerLobbyScene.h"
#include "scenes/SelectionScene.h"
#include "scenes/SongScene.h"
#include "scenes/StartScene.h"
#include "scenes/TalkScene.h"

static std::shared_ptr<GBAEngine> _engine;
static const GBFS_FILE* _fs;

static void goTo(Scene* scene) {
  _engine->transitionIntoScene(scene, new PixelTransitionEffect());
}

void SEQUENCE_initialize(std::shared_ptr<GBAEngine> engine,
                         const GBFS_FILE* fs) {
  _engine = engine;
  _fs = fs;
}

Scene* SEQUENCE_getInitialScene() {
  u32 fixes = SAVEFILE_initialize(_fs);

  if (!SAVEFILE_isWorking(_fs)) {
    auto scene = new TalkScene(_engine, _fs, SRAM_TEST_FAILED, [](u16 keys) {});
    scene->withButton = false;
    return scene;
  }

  if (fixes > 0) {
    auto scene =
        new TalkScene(_engine, _fs,
                      "Save file fixed!\r\n -> code: " + std::to_string(fixes) +
                          "\r\n\r\n=> Press A+B+START+SELECT",
                      [](u16 keys) {});
    scene->withButton = false;
    return scene;
  }

  bool isPlaying = SAVEFILE_read8(SRAM->state.isPlaying);
  auto gameMode = SAVEFILE_getGameMode();
  SAVEFILE_write8(SRAM->state.isPlaying, false);

  if (isPlaying && gameMode == GameMode::DEATH_MIX)
    return new DeathMixScene(_engine, _fs);

  if (isPlaying && !IS_MULTIPLAYER(gameMode))
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
  bool areArcadeModesUnlocked = SAVEFILE_isModeUnlocked(GameMode::ARCADE);
  bool isImpossibleModeUnlocked = SAVEFILE_isModeUnlocked(GameMode::IMPOSSIBLE);

  if (!IS_STORY(gameMode) && !areArcadeModesUnlocked) {
    goTo(new TalkScene(
        _engine, _fs, ARCADE_MODE_LOCKED,
        [](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed)
            goTo(new StartScene(_engine, _fs));
        },
        true));
    return;
  }

  if (gameMode == GameMode::IMPOSSIBLE && !isImpossibleModeUnlocked) {
    goTo(new TalkScene(
        _engine, _fs, IMPOSSIBLE_MODE_LOCKED,
        [](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed)
            goTo(new StartScene(_engine, _fs));
        },
        true));
    return;
  }

  auto lastGameMode = SAVEFILE_getGameMode();
  bool isTransitioningBetweenCampaignAndChallenges =
      (lastGameMode == GameMode::CAMPAIGN &&
       gameMode >= GameMode::IMPOSSIBLE) ||
      (lastGameMode >= GameMode::IMPOSSIBLE && gameMode == GameMode::CAMPAIGN);
  if (lastGameMode != gameMode &&
      !isTransitioningBetweenCampaignAndChallenges) {
    auto songIndex = IS_STORY(gameMode) ? SAVEFILE_getLibrarySize() - 1 : 0;
    SAVEFILE_write8(SRAM->memory.numericLevel, 0);
    SAVEFILE_write8(SRAM->memory.pageIndex, Div(songIndex, PAGE_SIZE));
    SAVEFILE_write8(SRAM->memory.songIndex, DivMod(songIndex, PAGE_SIZE));
    SAVEFILE_write8(SRAM->adminSettings.arcadeCharts, ArcadeChartsOpts::SINGLE);
  }

  SAVEFILE_write8(SRAM->state.gameMode, gameMode);
  if (IS_MULTIPLAYER(gameMode)) {
    u16 keys = ~REG_KEYS & KEY_ANY;
    linkUniversal->setProtocol((keys & KEY_START)
                                   ? LinkUniversal::Protocol::WIRELESS_SERVER
                                   : LinkUniversal::Protocol::AUTODETECT);

    SEQUENCE_goToMultiplayerGameMode(gameMode);
  } else if (gameMode == GameMode::DEATH_MIX) {
    goTo(new DeathMixScene(_engine, _fs));
  } else {
    auto message = gameMode == GameMode::CAMPAIGN ? MODE_CAMPAIGN
                   : gameMode == GameMode::ARCADE ? MODE_ARCADE
                                                  : MODE_IMPOSSIBLE;
    goTo(new TalkScene(
        _engine, _fs, message,
        [](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed)
            goTo(new SelectionScene(_engine, _fs));
        },
        true));
  }
}

void SEQUENCE_goToMultiplayerGameMode(GameMode gameMode) {
  goTo(new MultiplayerLobbyScene(_engine, _fs,
                                 gameMode == GameMode::MULTI_COOP
                                     ? SyncMode::SYNC_MODE_COOP
                                     : SyncMode::SYNC_MODE_VS));
}

void SEQUENCE_goToMessageOrSong(Song* song, Chart* chart, Chart* remoteChart) {
  STATE_setup(song, chart);

  auto gameMode = SAVEFILE_getGameMode();

  if (!isMultiplayer())
    SAVEFILE_write8(SRAM->state.isPlaying, true);

  if (gameMode == GameMode::CAMPAIGN && song->applyTo[chart->difficulty] &&
      song->hasMessage) {
    goTo(new TalkScene(
        _engine, _fs, std::string(song->message), [song, chart](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed)
            goTo(new SongScene(_engine, _fs, song, chart));
        }));
    return;
  }

  if (gameMode == GameMode::CAMPAIGN && song->index == 0) {
    goTo(new TalkScene(
        _engine, _fs, KEYS_HINT,
        [song, chart](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed)
            goTo(new SongScene(_engine, _fs, song, chart));
        },
        true));
    return;
  }

  if (gameMode == GameMode::ARCADE &&
      GameState.mods.trainingMode == TrainingModeOpts::tON) {
    goTo(new TalkScene(
        _engine, _fs, KEYS_TRAINING_HINT,
        [song, chart](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed)
            goTo(new SongScene(_engine, _fs, song, chart));
        },
        true));
    return;
  }

  if (gameMode == GameMode::ARCADE && isSinglePlayerDouble() &&
      chart->type == ChartType::DOUBLE_COOP_CHART) {
    goTo(new TalkScene(
        _engine, _fs, COOP_HINT,
        [song, chart](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed)
            goTo(new SongScene(_engine, _fs, song, chart));
        },
        true));
    return;
  }

  goTo(new SongScene(_engine, _fs, song, chart, remoteChart));
}

void SEQUENCE_goToWinOrSelection(bool isLastSong) {
  auto gameMode = SAVEFILE_getGameMode();

  if (IS_STORY(gameMode) && isLastSong)
    goTo(new TalkScene(
        _engine, _fs, gameMode == GameMode::CAMPAIGN ? WIN : WIN_IMPOSSIBLE,
        [isLastSong](u16 keys) {
          bool isPressed =
              SAVEFILE_isUsingGBAStyle() ? (keys & KEY_A) : KEY_CENTER(keys);
          if (isPressed) {
            goTo(isLastSong ? SEQUENCE_getMainScene()
                            : new SelectionScene(_engine, _fs));
          }
        }));
  else
    goTo(new SelectionScene(_engine, _fs));
}

bool SEQUENCE_isMultiplayerSessionDead() {
  return isMultiplayer() && !syncer->isPlaying();
}
