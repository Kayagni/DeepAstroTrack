# DeepAstroTrack

Projet de motorisation autonome pour monture EQ5 basé sur ESP32, avec suivi sidéral, GoTo inertiel, et alimentation DIY.

---

## Objectifs

- Suivi précis, silencieux (StealthChop)
- GoTo basé sur IMU (BNO055)
- Détection de blocage moteur (INA219 + télémétrie)
- Dithering entre poses astrophoto
- Alimentation portable à base de 18650

## Organisation du code

```
src/
  ├── motor_control/       ← Contrôle RA/DEC
  ├── imu_fusion/          ← IMU BNO055 ou 10DOF + Madgwick
  └── main.ino             ← Futur point de fusion
```

## Matériel
- Monture EQ5
- Moteurs 42PM48L (ou NEMA17 futur)
- Drivers TMC2209
- ESP32
- Centrale inertielle BNO055 (ou 10DOF Madgwick)
- Capteur INA219
- Canon EOS 500D partiellement défiltré

## Roadmap
- [x] Pilotage moteur RA (TMC2209)
- [x] Centrale inertielle avec calibration automatique
- [ ] Contrôle DEC + fréquences multiples
- [ ] Dithering programmable
- [ ] Interface Wi-Fi / appli mobile
- [ ] GoTo par IMU + GPS
- [ ] Monitoring courant moteur + vibration
