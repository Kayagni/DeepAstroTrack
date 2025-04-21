Détail des différentes fonctions de sécurité prévues sur le système
1. Sécurité monture
   1. Objectif : assurer la sécurité des mouvements de la monture en prévenant l'utilisateur, ou en coupant l'alimentation en cas de problème
   2. Eléments sous surveillance : moteurs
   3. Principes de surveillance
      1. Intensité moteur via INA219
      2. Vibrations ou mouvements anormaux via BNO055
2. Sécurité alimentation
   1. Sécurité pendant l'utilisation
      1. Objectif : assurer la sécurité du matériel en aval des convertisseurs en prévenant l'utilisateur, ou en coupant l'alimentation en cas de problème
      2. Éléments sous surveillance
         1. Sortie des convertisseurs buck
         2. Sortie du battery pack
   2. Sécurité pendant la charge
      1. Objectif : assurer la sécurité du battery pack pendant la charge, avec possibilité de stopper la charge en cas de risque d'emballement thermique, ou de déterioration d'une cellule
      2. Principe
         1. Surveillance de la température de chaque cellule et du BMS
         2. Surveillance de l'écart de tension maximal entre les cellules
            1. Fait
               1. Implémentation physique des fils pour la mesure de tension
               2. Script de base pour la mesure et la transmission des données en wifi via ESP32
            2. A faire
               1. Approvisionner des modules de pont diviseur (trouver la référence appropriée)
               2. Implémenter physiquement un MOSFET pour piloter la coupure du circuit de charge
               3. Approvisionner un chargeur adapté pour la charge complète
               4. Intégrer les thermistances pour la mesure de température des cellules et du BMS
               5. Définir les valeurs d'alerte et les valeurs de coupure
               6. Créer le script qui réaliser les opérations suivantes :
                  1. Attendre le choix de l'utilisateur pour le type de charge (stockage ou utilisation)
                  2. Fermer le circuit du chargeur correspondant
                  3. Monitorer les niveaux de tension de chaque cellule, ainsi que les températures de chaque cellule et du BMS, et les comparer avec les niveaux d'alerte et de coupure
                  4. Déclencher la coupure de la charge en cas de problème
   3. Surveillance du vieillissement
      1. Objectif : anticiper une éventuelle dégradation des performances d'une cellule
      2. Principe : surveiller l'évolution de l'écart maximal entre les cellules à chaque charge. Si l'écart maximal augmente, problème de vieillissement prématuré d'une cellule possible, ou problème sur le BMS