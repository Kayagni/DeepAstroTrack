# 🛰️ DeepAstroTrack

Projet de motorisation et d'automatisation complète d'une monture équatoriale **SkyWatcher EQ5**, intégrant :

- Suivi sidéral
- GoTo inertiel
- Dithering
- Sécurité moteur & pack batterie
- Interface Wi-Fi
- Mise à jour OTA
- Monitoring du battery pack

---

## 📁 Structure du projet

### `src/`
Code source embarqué sur ESP32, organisé en modules :

| Répertoire / fichier                  | Description |
|--------------------------------------|-------------|
| `imu_fusion/imu_10dof_v0.ino`        | Centrale inertielle 10DOF avec filtre Madgwick et auto-calibration |
| `motor_control/motors_eq5_v0.ino`    | Pilotage des moteurs de la monture EQ5 (direction, fréquence, sécurité) |
| `battery_monitor_ota/`               | Monitoring du battery pack + interface Web + OTA |
| `wifi_helper.h`                      | Connexion automatique au réseau Wi-Fi avec le signal le plus fort |
| `ota_helper.h`                       | Initialisation du mode OTA (Over-The-Air update) |
| `secrets_template.h`                 | Fichier modèle pour renseigner les identifiants Wi-Fi |
| `secrets.h` *(non versionné)*        | Fichier réel contenant les identifiants (à créer localement) |

---

## 🔐 Sécurité & confidentialité

Le fichier `src/secrets.h` contient vos identifiants Wi-Fi et **ne doit pas être versionné**.

**Astuce :**  
Copiez `secrets_template.h` → `secrets.h` puis remplissez vos SSID et mots de passe :

```cpp
const WifiCredential WIFI_KNOWN[] = {
  { "MonSSID", "MotDePasse" },
  ...
};
```

---

## 📚 Documentation

### `docs/` → Documentation modulaire

| Fichier                                       | Contenu |
|----------------------------------------------|---------|
| `Projet_global.md`                            | Vision et architecture du projet |
| `Materiel_disponible.md`                      | Inventaire du matériel utilisé |
| `Synthese_boîtier_EQ5.md`                     | Détail du boîtier EQ5 motorisé |
| `fonction_Alimentation/Synthese.md`           | Design et suivi du battery pack |
| `fonction_Detection_Position/Synthese.md`     | Logique d’orientation (IMU, azimut/altitude, etc.) |
| `fonction_Interface/Synthese.md`              | Interfaces ESP32, boutons, Wi-Fi, OTA |
| `fonction_Motorisation/Synthese.md`           | Pilotage des moteurs, drivers, fréquence, sécurité |
| `fonction_securite/Synthese.md`               | Risques et sécurités (alimentation, moteur, thermique) |

---

## 📊 Données expérimentales

| Fichier                                       | Description |
|----------------------------------------------|-------------|
| `data/Donnees_Charge_Battery_Pack/Charge1.xlsx` | Suivi complet de la courbe de charge du pack 4S4P |

---

## 🧪 Tests

| Fichier                    | Description |
|---------------------------|-------------|
| `test/test_motor_freqs.ino` | Expérimentations autour du réglage de la fréquence moteur |

---

## 🚀 Premiers pas

1. Cloner le repo
2. Copier `secrets_template.h` en `secrets.h` dans `src/`
3. Flasher un des sketchs (`battery_monitor_ota` ou autre)
4. Profiter de l'OTA, du monitoring et des modules prêts à l'emploi
