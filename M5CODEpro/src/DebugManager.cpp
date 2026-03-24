#include "DebugManager.h"

void DebugManager::begin(int baud) {
    Serial.begin(baud);
}

void DebugManager::log(String message) {
    // On commente pour éviter de casser le parsing JSON de l'IHM sur le port série
    // Ou on l'utilise seulement si on ne debug pas via l'IHM
    // Serial.println("[DEBUG] " + message);
}