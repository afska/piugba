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
    "         LOCKED\r\n"
    "Win at least one song in\r\n"
    "  the CAMPAIGN mode to\r\n"
    "      unlock this.";

const std::string IMPOSSIBLE_MODE_LOCKED =
    "         LOCKED\r\n"
    "  Complete the CAMPAIGN\r\n"
    "  mode in any difficulty\r\n"
    "  level to unlock this.";

const std::string MODE_CAMPAIGN =
    "Play the CAMPAIGN MODE to\r\n"
    "make progress in the game\r\n"
    "and unlock new songs.";

const std::string MODE_ARCADE =
    "In ARCADE MODE, you are\r\n"
    "free to play the songs in\r\n"
    "any difficulty level.\r\n"
    "Use SELECT to add mods!";

const std::string MODE_IMPOSSIBLE_1 =
    "In IMPOSSIBLE MODE...\r\n"
    "the colors are INVERTED,\r\n"
    "the arrows are MIRRORED,\r\n"
    "and it's all PIXELATED.";

const std::string MODE_IMPOSSIBLE_2 =
    "Also, you have to be more\r\n"
    "PRECISE to hit the notes.\r\n"
    "That's called EXTRA\r\n"
    "JUDGEMENT. Best of luck!";

const std::string KEYS_HINT =
    "To change the speed, use:\r\n"
    "      START/SELECT\r\n"
    "To cancel the song, use: \r\n"
    "    A+B+START+SELECT";

const std::string WIN =
    "  *YOU'VE WON THE GAME*\r\n"
    "\r\n"
    "Now, I challenge you to \r\n"
    "play the IMPOSSIBLE MODE";

const std::string WIN_IMPOSSIBLE =
    "   WHAT?! You did it??\r\n"
    "\r\n"
    "  What about the other\r\n"
    "   difficulty levels?    ";

#endif  // SEQUENCE_MESSAGES_H
