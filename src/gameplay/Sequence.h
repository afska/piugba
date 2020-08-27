#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <libgba-sprite-engine/effects/fade_out_scene.h>
#include <libgba-sprite-engine/gba_engine.h>

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
void SEQUENCE_goToGameMode(GameMode gameMode);
void SEQUENCE_goToMessageOrSong(Song* song, Chart* chart);

#endif  // SEQUENCE_H
