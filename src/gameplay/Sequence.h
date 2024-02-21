#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <libgba-sprite-engine/gba_engine.h>

#include <string>

#include "gameplay/models/Song.h"
#include "gameplay/save/SaveFile.h"

extern "C" {
#include "utils/gbfs/gbfs.h"
}

void SEQUENCE_initialize(std::shared_ptr<GBAEngine> engine,
                         const GBFS_FILE* fs);

Scene* SEQUENCE_getInitialScene();
Scene* SEQUENCE_getCalibrateOrMainScene();
Scene* SEQUENCE_getMainScene();
Scene* SEQUENCE_activateVideo(bool showSuccessMessage);
Scene* SEQUENCE_deactivateVideo();
Scene* SEQUENCE_activateEWRAMOverclock();
Scene* SEQUENCE_deactivateEWRAMOverclock();
void SEQUENCE_goToGameMode(GameMode gameMode);
void SEQUENCE_goToMultiplayerGameMode(GameMode gameMode);
void SEQUENCE_goToMessageOrSong(Song* song,
                                Chart* chart,
                                Chart* remoteChart = NULL);
void SEQUENCE_goToWinOrSelection(bool isLastSong);
bool SEQUENCE_isMultiplayerSessionDead();
Scene* SEQUENCE_halt(std::string error);

#endif  // SEQUENCE_H
