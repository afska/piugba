#ifndef SEQUENCE_MESSAGES_H
#define SEQUENCE_MESSAGES_H

#include <string>

const std::string CALIBRATE_AUDIO_LAG =
    "Emulators need some\r\n"
    "calibration. If you're\r\n"
    "emulating, press START.\r\n"
    "If not, press SELECT.";

const std::string SRAM_TEST_FAILED =
    "Save file test failed :(\r\n"
    "Please set your emulator\r\n"
    "or flashcart to SRAM mode\r\n"
    "and restart the game.";

const std::string ARCADE_MODE_LOCKED =
    "         LOCKED          \r\n"
    "Win at least one song in\r\n"
    "  the CAMPAIGN mode to\r\n"
    "      unlock this.\r\n";

const std::string IMPOSSIBLE_MODE_LOCKED =
    "         LOCKED          \r\n"
    "  Complete the CAMPAIGN\r\n"
    "  mode in any difficulty\r\n"
    "  level to unlock this.\r\n";

const std::string WIN =
    "  *YOU'VE WON THE GAME*  \r\n"
    "\r\n"
    " Now, I challenge you to \r\n"
    " play the IMPOSSIBLE MODE";

const std::string WIN_IMPOSSIBLE =
    "   WHAT?! You did it??   \r\n"
    "\r\n"
    "  What about the other   \r\n"
    "   difficulty levels?    ";

#endif  // SEQUENCE_MESSAGES_H
