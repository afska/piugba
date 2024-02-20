#ifndef SEQUENCE_MESSAGES_H
#define SEQUENCE_MESSAGES_H

#include <string>

#define CALIBRATE_AUDIO_LAG     \
  "Emulators need some\r\n"     \
  "calibration. If you're\r\n"  \
  "emulating, press START.\r\n" \
  "If not, press SELECT."

#define SRAM_TEST_FAILED          \
  "Save file test failed :(\r\n"  \
  "Please set your emulator\r\n"  \
  "or flashcart to SRAM mode\r\n" \
  "and restart the game."

#define ARCADE_MODE_LOCKED       \
  "         LOCKED\r\n"          \
  "Win at least one song in\r\n" \
  "  the CAMPAIGN mode to\r\n"   \
  "      unlock this."

#define IMPOSSIBLE_MODE_LOCKED  \
  "         LOCKED\r\n"         \
  " Complete the CAMPAIGN\r\n"  \
  " mode in any difficulty\r\n" \
  " level to unlock this."

#define MODE_CAMPAIGN             \
  "Play the CAMPAIGN MODE to\r\n" \
  "make progress in the game\r\n" \
  "and unlock new songs."

#define MODE_ARCADE               \
  "In ARCADE MODE, you are\r\n"   \
  "free to play the songs in\r\n" \
  "any difficulty level.\r\n"     \
  "Use SELECT to add mods!"

#define MODE_IMPOSSIBLE         \
  "In IMPOSSIBLE MODE:\r\n"     \
  "- Songs run FASTER\r\n"      \
  "- Patterns are MIRRORED\r\n" \
  "- Screen has visual MODS"

#define MODE_DEATH_MIX            \
  "In DEATHMIX MODE, you'll\r\n"  \
  "play a mixed nonstop set.\r\n" \
  "Life can't be recovered,\r\n"  \
  "so try not to MISS!"

#define KEYS_HINT                 \
  "To change the speed, use:\r\n" \
  "      START/SELECT\r\n"        \
  " To abort the song, use:\r\n"  \
  "    A+B+START+SELECT"

#define KEYS_TRAINING_HINT        \
  "How to use Training Mode:\r\n" \
  "- Speed: START/SELECT\r\n"     \
  "- Rate: B + START/SELECT\r\n"  \
  "- Fast-forward: A + START"

#define COOP_HINT             \
  "       CO-OP LEVEL\r\n"    \
  "1) Get a friend.\r\n"      \
  "2) Grab a Link Cable.\r\n" \
  "3) Use Multi COOP."

#define WIN                      \
  "  *YOU'VE WON THE GAME*\r\n"  \
  "\r\n"                         \
  "Now, I challenge you to \r\n" \
  "play the IMPOSSIBLE MODE"

#define WIN_IMPOSSIBLE         \
  "   WHAT?! You did it??\r\n" \
  "\r\n"                       \
  "   You are an absolute\r\n" \
  "  legend. Amazing work!"

#define SAVE_FILE_FIXED_1 \
  "Save file fixed!\r\n"  \
  " -> code: "
#define SAVE_FILE_FIXED_2 \
  "\r\n\r\n"              \
  "=> Press A+B+START+SELECT"

#define VIDEO_ACTIVATION_FAILED_CRASH \
  "Video activation failed!\r\n"      \
  " -> it crashed :/\r\n"             \
  "\r\n"                              \
  "=> Press A+B+START+SELECT"

#define VIDEO_ACTIVATION_FAILED_NO_FLASHCART \
  "Video activation failed!\r\n"             \
  " -> no EverDrive found\r\n"               \
  "\r\n"                                     \
  "=> Press A+B+START+SELECT"

#define VIDEO_ACTIVATION_FAILED_MOUNT_FAILED \
  "Video activation failed!\r\n"             \
  " -> FAT mount error\r\n"                  \
  "\r\n"                                     \
  "=> Press A+B+START+SELECT"

#define VIDEO_ACTIVATION_SUCCESS \
  "Videos are now enabled!\r\n"  \
  "\r\n"                         \
  "\r\n"                         \
  "=> Press A+B+START+SELECT"

#define VIDEO_READING_FAILED     \
  "Error reading SD card :(\r\n" \
  "\r\n"                         \
  "\r\n"                         \
  "Videos are now disabled."

#endif  // SEQUENCE_MESSAGES_H
