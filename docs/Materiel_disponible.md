1. Contrôleur principal
   1. ESP32 Devkit v1
2. Capteurs et IMU
   1. Centrale inertielle
      1. Modèle : https://amzn.eu/d/2EdpWbS
      2. Utilisations
         1. Détection de la position du télescope
         2. Aide à la mise en station en pointant le nord
   2. Module GPS
      1. Modèle : https://amzn.eu/d/124hGGf
      2. Utilisations
         1. Aider à la mise en station en donnant à l'utilisateur la latitude pour l'inclinaison de la monture
         2. Donner l'heure exacte pour prendre en compte le déplacement des cibles lors de l'utilisation du GoTo
   3. Thermistances :
      1. Modèle : https://amzn.eu/d/8QBoP6i
      2. Utilisation : Suivre l'évolution de la température du battery pack et déclencher l'arrêt ou une alerte utilisateur en cas de problème
3. Alimentation
   1. Accus 18650 (16 unités pour battery pack 4S4P)
      1. Modèle : https://bestpiles.fr/piles-rechargeables-18650/1098-destockage-pile-rechargeable-18650-icr18650-30e-terrae-li-ion-37v-3000mah-6a.html
   2. Convertisseurs buck
      1. Modèle : https://amzn.eu/d/8eJ036c
      2. Utilisation : Création de lignes d'alimentation 12V, 7.4V et 6V
   3. Dummy Battery pour Canon EOS 500D
      1. Utilisation : alimentation sur battery pack de l'appareil photo
   4. Module régulateur de tension
      1. Modèle : https://amzn.eu/d/9PMnzDn
      2. Utilisation : Alimentation de l'ESP32 à partir du convertisseur buck 7.4V
   5. Câbles pour battery pack
      1. Modèle : https://amzn.eu/d/fRI3DDX
   6. Chargeur
      1. Modèle : https://amzn.eu/d/7lrD8UU
      2. Utilisation : Recharche du battery pack
4. Sécurité
   1. Fusibles
      1. Plusieurs modèles différents dans un coffret : https://amzn.eu/d/bfBrSov
      2. Utilisation
         1. Fusible général pour le battery pack
         2. Fusibles pour la coupure des convertisseurs buck en cas de dégradation de ceux-ci
   2. MOSFET
      1. Plusieurs modèles différents dans un coffret : https://amzn.eu/d/dRACiDE
      2. Utilisation : mise en court-circuit du fusible de protection d'un convertisseur buck en cas d'augmentation accidentelle de la tension
   3. Diodes Zener
      1. Plusieurs modèles différents dans un coffret : https://amzn.eu/d/inEQTMb
      2. Utilisation : Détecte l'augmentation de tension en sortie du convertisseur buck et déclenche le MOSFET associé pour court-circuit du fusible de protection
   4. Capteur d'intensité (non acheté pour l'instant)
      1. Modèle envisagé : https://amzn.eu/d/9AZDGR6
      2. Utilisation : A mettre sur le circuit de pilotage des moteurs pour détection d'une intensité anormale, qui pourrait traduire un blocage moteur
