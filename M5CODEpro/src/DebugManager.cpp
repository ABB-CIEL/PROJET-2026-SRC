#include "DebugManager.h"

void DebugManager::begin(int baud) {
    Serial.begin(baud);
}

void DebugManager::log(String message) {
    Serial.println("[DEBUG] " + message);
}