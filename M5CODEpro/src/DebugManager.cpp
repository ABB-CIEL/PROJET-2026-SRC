#include "DebugManager.h"

// Flag global pour activer/désactiver les logs (évite de polluer le JSON USB par défaut)
bool g_debugEnabled = false;

void DebugManager::begin(int baud) {
    Serial.begin(baud);
}

void DebugManager::log(String message) {
    if (g_debugEnabled) {
        // Préfixe [DEBUG] pour que l'IHM puisse filtrer ou ignorer ces lignes
        Serial.println("[DEBUG] " + message);
    }
}