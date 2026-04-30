/**
 * @file DebugManager.cpp
 * @brief Implémentation de la gestion des logs de débogage
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#include "DebugManager.h"

/** @var g_debugEnabled
 *  @brief Drapeau global pour activer/désactiver les logs série
 */
bool g_debugEnabled = false;

// ============================================================
// REGION : Gestion des Logs
// ============================================================

/** @class CDebugManager
 *  @brief Fournit des outils de journalisation conditionnelle
 */

/**
 * @brief Initialise le port série de débogage
 * @param baud Vitesse de communication
 */
void CDebugManager::begin(int baud)
{
    Serial.begin(baud);
}

/**
 * @brief Envoie un message sur le port série si le débogage est actif
 * @param message Chaîne à logguer
 */
void CDebugManager::log(String message)
{
    if (g_debugEnabled)
    {
        Serial.println("[DEBUG] " + message);
    }
}