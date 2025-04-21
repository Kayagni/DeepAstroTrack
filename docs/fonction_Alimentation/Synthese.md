Pas encore démarré. La quasi totalité du matériel a été approvisionné, à l'exception des thermistances, qui pourront être ajoutées à la suite.
1. Objectif : Créer une alimentation portable et stable pour le setup astrophotographique, basée sur des accus 18650 en configuration 4S4P (14.8V nominal), capable d'alimenter :
   1. Une monture EQ5 motorisée (6V pour la motorisation actuelle)
   2. Un canon EOS 500D via dummy battery (7.4V)
   3. L'ESP32 et capteurs associés (3.3V)
   4. Des moteurs NEMA17, si changement de motorisation
2. Opérations réalisées
   1. Vérification des cellules réalisée au voltmètre, test individuel des 16 cellules
   2. Tensions de départ varient entre 3.530V et 3.545V, soit un écart maximum de 15mV
   3. Conclusion : les cellules étaient suffisamment équilibrées pour un montage direct sans pré-équilibrage actif.
3. Organisation retenue
   1. Topologie physique
      1. 2 lignes de 8 accus (format 2x8), préférée à un format 4x4 pour maximiser la surface d'échange thermique
      2. Chaque groupe parallèle de 4 cellules (4P) est constitué de 2 rangées de 2 accus, soudés entre eux par bandes de nickel
      3. Les 4 groupes sont ensuite reliés en série (4S)
   2. Méthode de câblage
      1. Fixation des bandes de nickel à l'aide d'une soudeuse par point
      2. Connexions du BMS par fils électriques cuivre multibrins de section 1.5mm²
      3. Connexion de charge par connecteur tamiya
      4. Câblage BMS réalsé en respectant
         1. B- : masse globale
         2. B1 à B4 : connexions intermédiaires
         3. P-/P+ : sorties principales
   3. BMS et sécurité
      1. Modèle : https://amzn.eu/d/0Z8kjHN avec équilibrage passif
      2. Fonctionnalités : protection contre surcharge, décharge profonde, court-circuit
      3. BMS installé sur le côté du pack
   4. Observations techniques
      1. Montage solide, équilibré, avec très bonne accessibilité pour le futur monitoring
      2. Design thermique optimisé grâce au format étendu 2x8
      3. Possibilité d'intégrer facilement dans une malette de transport
   5. Chargeur
      1. Modèle : https://amzn.eu/d/iLdoivL
      2. Courant de charge de 2A
      3. Adaptation nécessaire : supression de la fiche initiale pour une fiche tamiya
4. Tests à réaliser
   1. Première charge : surveillance quasi-constante, avec relevés des tensions pour chaque groupe de cellule toutes les 30 à 45 minutes, puis avec une fréquence rapprochée en phase de tension constante
   2. Tests d'alimentation des convertisseurs buck
   3. Installation et test d'un système de sécurité crowbar (augmentation de la tension du convertisseur pour vérifier la mise en court-circuit et la destruction du fusible)
5. Améliorations ultérieures
   1. Installation de thermistances pour surveillance de température
   2. Installation de connexions pour suivi de charge via une ESP32 (vérification des courbes de charge et de l'état de santé des cellules)
      1. Objectifs
         1. Vérifier le comportement en charge (fonction sécurité)
         2. Avoir la possibilité de charger le pack à la capacité de stockage si pas d'utilisation prévue dans les 24-48h
   3. Installation d'une gaine thermo XL pour finalisation