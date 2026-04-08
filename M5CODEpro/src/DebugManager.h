#ifndef DEBUG_MANAGER_H
#define DEBUG_MANAGER_H

#include <Arduino.h>

class DebugManager {

public:

    void begin(int baud);

    void log(String message);

};

#endif