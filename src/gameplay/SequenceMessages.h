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

#endif  // SEQUENCE_MESSAGES_H
