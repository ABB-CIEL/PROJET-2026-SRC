/**
 * @file DebugManager.h
 * @brief Définition de la classe de gestion du débogage série
 * @author Étudiant BTS CIEL IR
 * @date 2026
 */

#ifndef DEBUG_MANAGER_H
#define DEBUG_MANAGER_H

#include <Arduino.h>

/** @class CDebugManager
 *  @brief Fournit des outils pour l'affichage de logs conditionnels
 */
class CDebugManager
{
public:
    /**
     * @brief Initialise la communication série pour le debug
     * @param baud Vitesse de communication
     */
    void begin(int baud);

    /**
     * @brief Affiche un message de debug si activé
     * @param message Texte à afficher
     */
    void log(String message);
};

#endif