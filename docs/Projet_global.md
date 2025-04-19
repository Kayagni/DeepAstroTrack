1. Objectif global du projet : créer une motorisation autonome, intelligente et modulaire pour une monture équatoriale Skywatcher EQ5, en remplacement du boîtier d’origine, avec les objectifs suivants :
   1. Suivi sidéral fiable, silencieux, et précis
   2. Système de GoTo pour pointer sur les cibles en fonction de leurs coordonnées RA et DEC
   3. Corriger les problèmes de suivi en cas de mauvaise mise en station
   4. Détecter les blocages moteurs ou les comportements inhabituels
   5. Déclencher automatiquement l'appareil photo pendant les sessions
   6. Créer un dithering léger entre chaque pose
   7. Alimentation autonome, rechargeable, et sécurisée
2. Setup matériel actuel :
   1. Télescope : ProNewton Omegon 153/750
   2. Monture : Skywatcher EQ5 avec double motorisation : https://www.astroshop.de/fr/moteurs-de-suivi-classiques-et-goto/skywatcher-eq4-5-serie-de-moteur-r-et-de/p,1500
   3. Appareil photo : Canon EOS 500D partiellement défiltré
3. Description technique des solutions envisagées
   1. Pilotage général par ESP32
   2. Alimentation par pack d'accus 18650 montés en 4S4P
      1. Alimentations différentiées par convertisseur buck
         1. 12V pour l'alimentation des moteurs NEMA
         2. 7.4V pour l'alimentation de la dummy battery de l'appareil photo
         3. 6V pour l'alimentation des moteurs actuels
         4. 3.3V (via module de régulation 3.3V relié au convertisseur buck 7.4V)
      2. Sécurité du battery pack
         1. BMS pour la gestion du pack
         2. Fusibles de protection en amont de chaque convertisseur buck, en cas de défaut de régulation du convertisseur, coupure automatique par mise en cours circuit du fusible, via MOSFET déclenché par diode Zener, en montage crowbar
   3. GoTo et suivi assuré par la détection de la position du télescope (via BNO055 et module GPS)
   4. Déclenchement de l'appareil photo par l'ESP32 via prise jack de télécommande
4. Étapes techniques principales
   1. Rétro-Engeneering du boitier pour comprendre les signaux envoyés aux moteurs (Fait, le fonctionnement du boitier et les informations techniques des moteurs sont détaillés dans le fichier "Synthèse rétro-engeneering boitier.txt")
   2. Essais de reproduction des signaux de commande de smoteurs via ESP32 et TMC2209 (Réalisé, le script de base est intégré aux fichiers du projet : "Pilotage_Moteurs_Origine_v0.ino")
   3. Remplacement future des moteurs par moteurs NEMA17 (ultérieurement et si nécessaire. Nécessite une adaptation mécanique)
   4. Implémentation de mesures vibratoires et de stratégies de détection pour :
      1. Détecter les pertes de pas
      2. Analyser les fréquences vibratoires par transformée de Fourier pour détermier l'origine des vibrations de la monture (motorisation, déclenchement de l'appareil photo, ou problème de setup)
      3. Implémenter un système de coupure automatique de la motorisation, et une alerte utilisateur (pour intervention si à proximité)
      4. Comparer les modes moteurs pour ajustement optimal [couple disponible / vibrations]
   5. Créer l'alimentation autonome
      1. Installer les accus 18650 en 4S4P avec le BMS
      2. Créer les lignes d'alimentation avec leur sécurité intrinsèque
      3. Etudier l'implémentation d'un monitoring du battery pack via l'ESP32 pour retour utilisateur et gestion des alertes
   6. Implémentation du GoTo
      1. Implémenter la détection de position par IMU
         1. Mise en oeuvre du BNO055 pour la détection de position (Fait, script de base intégré aux fichiers du projet : "Script_BNO055_v0.ino")
         2. Test en conditions réelles pour évaluation de la stabilité et de la précision
      2. Implémenter le processus de calibration du pointage et la conversion des données d'attitude en coordonnées azimut / altitude
      3. Implémenter la fonction GoTo
5. Logique projet et stratégies
   1. Approche prudente : on reste prudent sur toutes les étapes qui présenteraient un risque de déterioration du matériel existant
   2. Approche modulaire : chaque sous étape est validée indépendamment
   3. Approche progressive : 
      1. Ne jamais rendre le matériel d'astro indisponible
      2. Implémentation des fonctions de base, puis upgrading progressif
      3. L'upgrading logiciel est au maximum réalisé en flashant l'ESP avec un protocole OTA, pour réglages et modifications en conditions réelles
      4. Rétrocompatibilité : le matériel peut fonctionner sans le système en cours de création