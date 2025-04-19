#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>      // Accéléromètre + magnétomètre LSM303DLHC
#include <Adafruit_L3GD20_U.h>      // Gyroscope L3GD20 (adresse I2C 0x69 sur breakout 10DOF)
#include <Adafruit_BMP085.h>        // Capteur de pression BMP180 (compatible BMP085 lib)
#include <MadgwickAHRS.h>
#include <EEPROM.h>

// ======================= Définition des structures et constantes =======================
struct CalibrationData {
  uint32_t magic;
  float beta;          // Paramètre beta du filtre Madgwick (gain algorithme)
  float sampleRate;    // Fréquence d’échantillonnage en Hz
  float alpha;         // Coefficient de lissage pour le gyroscope [0-1]
  float accThresh;     // Seuil de détection de mouvement (accélération) en m/s^2
  float gyroThresh;    // Seuil de détection de mouvement (gyroscope) en rad/s
  float magOffsetX, magOffsetY, magOffsetZ;      // Offsets de calibrage du magnétomètre
  float accelOffsetX, accelOffsetY, accelOffsetZ; // Offsets de calibrage de l'accéléromètre
  float gyroOffsetX, gyroOffsetY, gyroOffsetZ;    // Offsets de calibrage du gyroscope
} calData;

const uint32_t CALIBRATION_MAGIC = 0xCAFEBABE;   // Valeur magique pour indiquer la présence de données calibrées
const int EEPROM_SIZE = 128;                    // Taille d'EEPROM à allouer (en octets)

// Seuils de stabilité acceptables (écart-type max en degrés) pour considérer la calibration suffisante
const float STABILITY_THRESHOLD_DEG = 0.5;      // 0.5° d'écart-type maximum

// ======================= Objets capteurs et filtre =======================
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
Adafruit_L3GD20_Unified gyro    = Adafruit_L3GD20_Unified(30303, 0x69);
Adafruit_BMP085        bmp;
Madgwick filter;  // Filtre de fusion de capteurs (AHRS Madgwick)

// ======================= Prototypes des fonctions =======================
bool loadCalibration(CalibrationData &cal);
void saveCalibration(const CalibrationData &cal);
void calibrateSensors(CalibrationData &cal);
void calibrateMagnetometer(CalibrationData &cal);
void calibrateGyroAccelOffsets(CalibrationData &cal);
void calibrateOrientationCheck();
void tuneFilterParameters(CalibrationData &cal);
void measureStability(float &stdRoll, float &stdPitch, float &stdYaw);

// ======================= Setup =======================
void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }  // Attendre l'ouverture du port série (utile sur certaines plateformes)
  Serial.println(F("\n\n--- Demarrage du programme de centrale inertielle 10DOF ---"));
  
  // Initialisation des capteurs I2C
  Wire.begin();
  if (!accel.begin()) {
    Serial.println(F("Erreur : accelerometre LSM303 non detecte"));
  }
  if (!mag.begin()) {
    Serial.println(F("Erreur : magnetometre LSM303 non detecte"));
  }
  if (!gyro.begin(GYRO_RANGE_250DPS)) {
    Serial.println(F("Erreur : gyroscope L3GD20 non detecte"));
  }
  if (!bmp.begin()) {
    Serial.println(F("Erreur : capteur de pression BMP180 non detecte"));
  }
  
  // Configuration par defaut des parametres de calibration
  calData = {0};
  calData.magic = 0;  // Par défaut, indique pas de calibration chargée
  
  // Initialiser l'EEPROM et tenter de charger des paramètres existants
  EEPROM.begin(EEPROM_SIZE);
  if (loadCalibration(calData)) {
    Serial.println(F("Parametres de calibration trouves en EEPROM."));
    Serial.println(F("Appuyez sur 'c' dans les 15 secondes pour relancer la calibration..."));
    bool doCalibrate = false;
    unsigned long t0 = millis();
    // Attente pendant 15s un éventuel appui sur 'c'
    while (millis() - t0 < 15000) {
      if (Serial.available()) {
        char ch = Serial.read();
        if (ch == 'c' || ch == 'C') {
          doCalibrate = true;
          break;
        }
      }
      delay(5);
    }
    if (doCalibrate) {
      Serial.println(F("\n*** Recalibration demandee par l'utilisateur ***"));
      calibrateSensors(calData);
    } else {
      Serial.println(F("Calibration conservee. Utilisation des parametres enregistres."));
    }
  } else {
    Serial.println(F("Aucun parametre enregistre. Lancement de la procedure de calibration initiale."));
    calibrateSensors(calData);
  }
  
  // Démarrer le filtre Madgwick avec les paramètres calibrés
  filter.begin(calData.sampleRate);
  // Régler beta du filtre si possible (via manipulation directe du membre prive beta)
  // Note: MadgwickAHRS n'offre pas de setter, on modifie directement en memoire
  float *betaPtr = (float*) &filter;
  *betaPtr = calData.beta;
  
  Serial.println(F("\n--- Debut de la boucle principale (fusion de capteurs) ---"));
  Serial.println(F("Envoi des angles (Pitch, Roll, Yaw) au port serie...\n"));
}

// ======================= Boucle Principale =======================
void loop() {
  static unsigned long microsPrevious = 0;
  static unsigned long microsPerReading = 0;
  if (microsPerReading == 0) {
    microsPerReading = 1000000UL / (unsigned long)calData.sampleRate;
    microsPrevious = micros();
  }
  
  // Maintenir la fréquence d'échantillonnage désirée
  unsigned long microsNow = micros();
  if (microsNow - microsPrevious < microsPerReading) {
    // Pas encore l'heure de la prochaine lecture
    return;
  }
  microsPrevious += microsPerReading;
  
  // Lecture des capteurs bruts
  sensors_event_t eventAcc, eventMag, eventGyro;
  accel.getEvent(&eventAcc);
  mag.getEvent(&eventMag);
  gyro.getEvent(&eventGyro);
  
  // Appliquer les offsets de calibration aux mesures brutes
  float ax = eventAcc.acceleration.x - calData.accelOffsetX;
  float ay = eventAcc.acceleration.y - calData.accelOffsetY;
  float az = eventAcc.acceleration.z - calData.accelOffsetZ;
  float gx = eventGyro.gyro.x - calData.gyroOffsetX;
  float gy = eventGyro.gyro.y - calData.gyroOffsetY;
  float gz = eventGyro.gyro.z - calData.gyroOffsetZ;
  // Convertir les vitesses angulaires de rad/s en deg/s pour le filtre Madgwick
  gx *= 57.2958f;
  gy *= 57.2958f;
  gz *= 57.2958f;
  
  // Calcul de la norme d'acceleration pour detection de mouvement
  float accNorm = sqrt(ax*ax + ay*ay + az*az);
  // Calcul de la norme du vecteur gyroscope (en rad/s)
  float gyroNorm = sqrt((eventGyro.gyro.x * eventGyro.gyro.x) + 
                        (eventGyro.gyro.y * eventGyro.gyro.y) + 
                        (eventGyro.gyro.z * eventGyro.gyro.z));
  
  bool moving = false;
  // Si l'acceleration devie de plus de accThresh de la gravite (~9.81) OU si le gyro depasse son seuil, on considere qu'il y a mouvement
  if (fabs(accNorm - 9.80665f) > calData.accThresh || gyroNorm > calData.gyroThresh) {
    moving = true;
  }
  
  // Mise à jour du filtre de fusion (Madgwick):
  if (moving) {
    // Si en mouvement, on ignore le magnetometre pour eviter les perturbations
    filter.updateIMU(gx, gy, gz, ax, ay, az);
  } else {
    // Si stable, on utilise le magnetometre pour corriger le cap (yaw)
    // Convertir les valeurs magnétiques brutes en unités adaptées (le filtre normalise de toute façon)
    float mx = eventMag.magnetic.x - calData.magOffsetX;
    float my = eventMag.magnetic.y - calData.magOffsetY;
    float mz = eventMag.magnetic.z - calData.magOffsetZ;
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
  }
  
  // Récupérer les angles d'Euler (en degrés)
  float roll  = filter.getRoll();
  float pitch = filter.getPitch();
  float yaw   = filter.getYaw();
  // Optionnel: ajuster yaw pour qu'il soit entre 0-360 (le getYaw() le retourne déjà 0-360 normalement)

// Envoi des angles au port série  
// Version pour Processing
Serial.print(pitch, 1); Serial.print(","); Serial.print(roll, 1); Serial.print(","); Serial.println(yaw, 1);

// // Version pour debug humain
// Serial.print(F("Pitch: "));
// Serial.print(pitch, 1);
// Serial.print(F("\tRoll: "));
// Serial.print(roll, 1);
// Serial.print(F("\tYaw: "));
// Serial.println(yaw, 1);

}

// ======================= Chargement de la calibration depuis EEPROM =======================
bool loadCalibration(CalibrationData &cal) {
  EEPROM.get(0, cal);
  if (cal.magic == CALIBRATION_MAGIC) {
    // Les données en EEPROM semblent valides
    return true;
  }
  // Aucune calibration valide
  return false;
}

// ======================= Sauvegarde de la calibration dans l'EEPROM =======================
void saveCalibration(const CalibrationData &cal) {
  EEPROM.put(0, cal);
  EEPROM.commit();  // Nécessaire sur ESP32 pour vraiment écrire en flash
}

// ======================= Procedure complete de calibration des capteurs =======================
void calibrateSensors(CalibrationData &cal) {
  // Étape 1: Calibration des offsets gyroscope et accelerometre sur surface plane
  calibrateGyroAccelOffsets(cal);
  
  // Étape 2: Calibration du magnétomètre (figure en 8)
  calibrateMagnetometer(cal);
  
  // Étape 3: Vérification de l'orientation (inclinaison à 45°)
  calibrateOrientationCheck();
  
  // Étape 4: Ajustement des paramètres du filtre Madgwick (beta, fréquence, lissage, seuils)
  tuneFilterParameters(cal);
  
  // Sauvegarde des paramètres calibrés en EEPROM
  cal.magic = CALIBRATION_MAGIC;
  saveCalibration(cal);
  
  Serial.println(F("Calibration terminee. Parametres enregistres en EEPROM."));
  Serial.print(F("Beta=")); Serial.print(cal.beta, 3);
  Serial.print(F(", Frequence=")); Serial.print(cal.sampleRate);
  Serial.print(F(" Hz, Alpha=")); Serial.print(cal.alpha, 3);
  Serial.print(F(", accThresh=")); Serial.print(cal.accThresh, 3);
  Serial.print(F(", gyroThresh=")); Serial.println(cal.gyroThresh, 3);
}

// ======================= Calibration des offsets du gyroscope et de l'accelerometre =======================
void calibrateGyroAccelOffsets(CalibrationData &cal) {
  Serial.println(F("\n[Calibration] Posez l'appareil immobile sur une surface plane et horizontale."));
  Serial.println(F("Appuyez sur entree pour commencer la calibration gyroscope/accelerometre..."));
  // Attendre confirmation de l'utilisateur
  while (!Serial.available()) {
    // attente
  }
  // Vider l'entrée
  while (Serial.available()) { Serial.read(); }
  
  Serial.println(F("Calibration en cours... Ne bougez pas l'appareil."));
  // Mesurer pendant environ 2 secondes pour estimer les offsets
  const int samples = 200;
  float gxSum = 0, gySum = 0, gzSum = 0;
  float axSum = 0, aySum = 0, azSum = 0;
  for (int i = 0; i < samples; ++i) {
    sensors_event_t eventAcc, eventGyro;
    accel.getEvent(&eventAcc);
    gyro.getEvent(&eventGyro);
    gxSum += eventGyro.gyro.x;
    gySum += eventGyro.gyro.y;
    gzSum += eventGyro.gyro.z;
    axSum += eventAcc.acceleration.x;
    aySum += eventAcc.acceleration.y;
    azSum += eventAcc.acceleration.z;
    delay(10); // ~100 Hz
  }
  // Moyenne des mesures
  float gxOffset = gxSum / samples;
  float gyOffset = gySum / samples;
  float gzOffset = gzSum / samples;
  float axOffset = axSum / samples;
  float ayOffset = aySum / samples;
  float azOffset = azSum / samples;
  // Pour l'accéléromètre: sur surface plane horizontale, on s'attend à ax=0, ay=0, az=+9.81 m/s^2 (acceleration gravitationnelle)
  // Donc le biais est ce qu'on mesure moins la valeur attendue
  cal.gyroOffsetX = gxOffset;
  cal.gyroOffsetY = gyOffset;
  cal.gyroOffsetZ = gzOffset;
  cal.accelOffsetX = axOffset;            // devrait être proche de 0
  cal.accelOffsetY = ayOffset;            // devrait être proche de 0
  cal.accelOffsetZ = azOffset - 9.80665f; // devrait être proche de 0 si aligné avec la gravité
  
  Serial.println(F("Offsets gyroscope trouves (deg/s):"));
  Serial.print(F("  GX offset = ")); Serial.println(cal.gyroOffsetX, 4);
  Serial.print(F("  GY offset = ")); Serial.println(cal.gyroOffsetY, 4);
  Serial.print(F("  GZ offset = ")); Serial.println(cal.gyroOffsetZ, 4);
  Serial.println(F("Offsets accelerometre trouves (m/s^2):"));
  Serial.print(F("  AX offset = ")); Serial.println(cal.accelOffsetX, 4);
  Serial.print(F("  AY offset = ")); Serial.println(cal.accelOffsetY, 4);
  Serial.print(F("  AZ offset = ")); Serial.println(cal.accelOffsetZ, 4);
}

// ======================= Calibration du magnetometre (offsets hard-iron) =======================
void calibrateMagnetometer(CalibrationData &cal) {
  Serial.println(F("\n[Calibration] Calibrage du magnetometre (effectuez des mouvements en 8)."));
  Serial.println(F("Tournez et inclinez l'appareil dans toutes les directions pendant 10 secondes."));
  delay(2000);
  Serial.println(F("Debut de l'acquisition des donnees magnetometre..."));
  
  // Initialiser min et max avec des valeurs inverses extrêmes
  float minX = 1000, minY = 1000, minZ = 1000;
  float maxX = -1000, maxY = -1000, maxZ = -1000;
  unsigned long start = millis();
  while (millis() - start < 10000) {  // pendant 10 secondes
    sensors_event_t eventMag;
    mag.getEvent(&eventMag);
    float mx = eventMag.magnetic.x;
    float my = eventMag.magnetic.y;
    float mz = eventMag.magnetic.z;
    // Mettre à jour les bornes min/max
    if (mx < minX) minX = mx;
    if (my < minY) minY = my;
    if (mz < minZ) minZ = mz;
    if (mx > maxX) maxX = mx;
    if (my > maxY) maxY = my;
    if (mz > maxZ) maxZ = mz;
    delay(20);
  }
  // Calcul des offsets (décalage hard-iron) comme milieu des min et max
  cal.magOffsetX = (maxX + minX) / 2.0f;
  cal.magOffsetY = (maxY + minY) / 2.0f;
  cal.magOffsetZ = (maxZ + minZ) / 2.0f;
  
  Serial.println(F("Calibration magnetometre terminee. Offsets trouves :"));
  Serial.print(F("  MX offset = ")); Serial.println(cal.magOffsetX, 2);
  Serial.print(F("  MY offset = ")); Serial.println(cal.magOffsetY, 2);
  Serial.print(F("  MZ offset = ")); Serial.println(cal.magOffsetZ, 2);
  Serial.println(F("(* Appliques automatiquement aux mesures magneto lors de la fusion *)"));
}

// ======================= Verification orientation (inclinaison 45 deg) =======================
void calibrateOrientationCheck() {
  Serial.println(F("\n[Calibration] Verification de l'orientation :"));
  Serial.println(F("Inclinez la centrale d’environ 45° sur l’un de ses axes (ex: axe X) et maintenez la position."));
  Serial.println(F("Appuyez sur entree lorsque l'appareil est en position."));
  while (!Serial.available()) {
    // attente de l'utilisateur
  }
  while (Serial.available()) { Serial.read(); } // flush
  
  // Lire quelques echantillons accelerometre pour estimer l'angle
  const int samples = 50;
  float accX = 0, accY = 0, accZ = 0;
  for (int i = 0; i < samples; ++i) {
    sensors_event_t eventAcc;
    accel.getEvent(&eventAcc);
    accX += eventAcc.acceleration.x;
    accY += eventAcc.acceleration.y;
    accZ += eventAcc.acceleration.z;
    delay(20);
  }
  accX /= samples;
  accY /= samples;
  accZ /= samples;
  // Soustraire offsets accel pour avoir valeurs calibrées
  accX -= calData.accelOffsetX;
  accY -= calData.accelOffsetY;
  accZ -= calData.accelOffsetZ;
  
  // Calculer les angles de roulis (roll) et tangage (pitch) estimés à partir de l'accéléromètre
  float rollDeg  = atan2(accY, accZ) * 57.2958f;
  float pitchDeg = atan2(-accX, sqrt(accY*accY + accZ*accZ)) * 57.2958f;
  Serial.print(F("Angle mesure (approx) - Roll: "));
  Serial.print(rollDeg, 1);
  Serial.print(F("°, Pitch: "));
  Serial.print(pitchDeg, 1);
  Serial.println(F("°."));
  Serial.println(F("(L'un de ces angles devrait etre proche de 45° si l'inclinaison est correcte)"));
}

// ======================= Ajustement des parametres du filtre et evaluation de la stabilite =======================
void tuneFilterParameters(CalibrationData &cal) {
  // Valeurs initiales par defaut
  cal.sampleRate = 100.0f;  // Fréquence d'échantillonnage initiale (Hz)
  cal.beta = 0.1f;          // Valeur par défaut du filtre Madgwick (pour 512Hz). On l'ajustera plus tard.
  cal.alpha = 0.1f;         // Lissage faible initial sur le gyroscope
  // Seuils de détection de mouvement par défaut (peu sensibles)
  cal.accThresh = 0.5f;     // 0.5 m/s^2
  cal.gyroThresh = 0.02f;   // ~0.02 rad/s (~1.15°/s)
  
  // Ajuster beta initial en fonction de la fréquence (loi empirique: beta*freq ~ 50)
  cal.beta = 50.0f / cal.sampleRate;
  if (cal.beta < 0.05f) cal.beta = 0.05f;  // Eviter trop petit
  if (cal.beta > 1.0f) cal.beta = 1.0f;    // Limiter beta max
  
  Serial.println(F("\n[Calibration] Ajustement des parametres du filtre Madgwick..."));
  Serial.print(F("Frequence echantillonnage initiale: ")); Serial.print(cal.sampleRate); Serial.println(F(" Hz"));
  Serial.print(F("Beta initial: ")); Serial.println(cal.beta, 3);
  Serial.print(F("Lissage gyroscope (alpha): ")); Serial.println(cal.alpha, 3);
  
  // Initialiser le filtre avec ces paramètres pour évaluation
  filter.begin(cal.sampleRate);
  // Appliquer le beta choisi via manipulation memoire (pas de setter disponible)
  float *betaPtr = (float*) &filter;
  *betaPtr = cal.beta;
  
  // Mesurer la stabilité initiale
  float stdRoll, stdPitch, stdYaw;
  measureStability(stdRoll, stdPitch, stdYaw);
  Serial.println(F("Stabilite initiale (ecart-type en degres):"));
  Serial.print(F("  Roll: ")); Serial.print(stdRoll, 2);
  Serial.print(F(", Pitch: ")); Serial.print(stdPitch, 2);
  Serial.print(F(", Yaw: ")); Serial.println(stdYaw, 2);
  
  // Boucle d'ajustement iterative
  const int MAX_ITER = 5;
  for (int iter = 1; iter <= MAX_ITER; ++iter) {
    bool stable = (stdRoll < STABILITY_THRESHOLD_DEG && stdPitch < STABILITY_THRESHOLD_DEG && stdYaw < STABILITY_THRESHOLD_DEG);
    if (stable) {
      Serial.println(F(">> Les mesures d'orientation sont stables (ecart-type < seuil)."));
    } else {
      Serial.println(F(">> La stabilite peut etre amelioree."));
    }
    // Demander à l'utilisateur s'il souhaite affiner ou accepter
    Serial.println(F("Appuyez sur 'i' puis Entree pour iterer et ajuster les parametres, ou Entree seule pour continuer."));
    bool iterate = false;
    unsigned long t0 = millis();
    while (millis() - t0 < 10000) { // attendre jusqu'à 10s la reponse
      if (Serial.available()) {
        char ch = Serial.read();
        if (ch == 'i' || ch == 'I') {
          iterate = true;
        }
        // Vider le reste de la ligne
        while (Serial.available()) { Serial.read(); }
        break;
      }
    }
    if (!iterate) {
      Serial.println(F("Ajustement termine."));
      break;
    }
    // Ajuster les parametres en fonction des résultats
    if (stdRoll > STABILITY_THRESHOLD_DEG || stdPitch > STABILITY_THRESHOLD_DEG) {
      // Si le roll/pitch ont trop de variation -> augmenter le lissage du gyroscope
      cal.alpha += 0.1f;
      if (cal.alpha > 0.9f) cal.alpha = 0.9f;
      Serial.print(F("Augmentation du lissage gyroscope (alpha) -> ")); Serial.println(cal.alpha, 2);
    }
    if (stdYaw > STABILITY_THRESHOLD_DEG) {
      // Si le yaw varie trop (probable derive) -> augmenter beta pour corriger plus vite la derive
      cal.beta += 0.1f;
      if (cal.beta > 1.5f) cal.beta = 1.5f;
      Serial.print(F("Augmentation du beta du filtre -> ")); Serial.println(cal.beta, 2);
    }
    // On pourrait aussi ajuster la frequence d'echantillonnage si necessaire
    if (!stable && iter == 1) {
      // Par exemple, si la première iteration est instable, essayons de reduire la frequence a 50 Hz
      cal.sampleRate = 50.0f;
      Serial.println(F("Reduction de la frequence d'echantillonnage a 50 Hz pour ameliorer la stabilite."));
    }
    // Réinitialiser le filtre avec les nouveaux parametres
    filter.begin(cal.sampleRate);
    *betaPtr = cal.beta;
    // Mesurer a nouveau la stabilite avec les nouveaux parametres
    measureStability(stdRoll, stdPitch, stdYaw);
    Serial.println(F("Nouvelle mesure de stabilite :"));
    Serial.print(F("  Roll std: ")); Serial.print(stdRoll, 2);
    Serial.print(F(", Pitch std: ")); Serial.print(stdPitch, 2);
    Serial.print(F(", Yaw std: ")); Serial.println(stdYaw, 2);
    // Re-boucle pour éventuellement d'autres itérations
    if (iter == MAX_ITER) {
      Serial.println(F("Limite d'iterations atteinte."));
    }
  }
  
  // Appliquer les seuils de mouvement definitifs:
  // On peut calibrer les seuils a partir des données acquises pendant la stabilité
  // Par exemple, conserver cal.accThresh et cal.gyroThresh par défaut ou ajuster si besoin.
  // Ici, on garde les valeurs par défaut ou modifiées manuellement par l'utilisateur si souhaité.
  
  Serial.println(F("Parametres filtres finalises."));
  Serial.print(F("-> Frequence: ")); Serial.print(cal.sampleRate); Serial.println(F(" Hz"));
  Serial.print(F("-> Beta: ")); Serial.println(cal.beta, 3);
  Serial.print(F("-> Alpha (lissage gyro): ")); Serial.println(cal.alpha, 3);
  Serial.print(F("-> Seuil mouvement accel: ")); Serial.println(cal.accThresh, 3);
  Serial.print(F("-> Seuil mouvement gyro: ")); Serial.println(cal.gyroThresh, 3);
}

// ======================= Mesure de la stabilite de l'orientation =======================
void measureStability(float &stdRoll, float &stdPitch, float &stdYaw) {
  // Mesure l'ecart-type des angles (roll, pitch, yaw) sur une periode de 2 sec environ
  const int sampleCount = 200;
  double sumRoll = 0, sumPitch = 0, sumYaw = 0;
  double sumSqRoll = 0, sumSqPitch = 0, sumSqYaw = 0;
  
  // Demander à l'utilisateur de garder l'appareil immobile pendant la mesure
  Serial.println(F("Mesure de stabilite... (gardez l'appareil immobile)"));
  for (int i = 0; i < sampleCount; ++i) {
    // Lire les capteurs
    sensors_event_t eventAcc, eventMag, eventGyro;
    accel.getEvent(&eventAcc);
    mag.getEvent(&eventMag);
    gyro.getEvent(&eventGyro);
    // Appliquer offsets
    float ax = eventAcc.acceleration.x - calData.accelOffsetX;
    float ay = eventAcc.acceleration.y - calData.accelOffsetY;
    float az = eventAcc.acceleration.z - calData.accelOffsetZ;
    float gx = (eventGyro.gyro.x - calData.gyroOffsetX) * 57.2958f;
    float gy = (eventGyro.gyro.y - calData.gyroOffsetY) * 57.2958f;
    float gz = (eventGyro.gyro.z - calData.gyroOffsetZ) * 57.2958f;
    // Toujours utiliser le magnetometre dans ce test (appareil immobile)
    float mx = eventMag.magnetic.x - calData.magOffsetX;
    float my = eventMag.magnetic.y - calData.magOffsetY;
    float mz = eventMag.magnetic.z - calData.magOffsetZ;
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
    // Récupérer les angles actuels
    float roll = filter.getRoll();
    float pitch = filter.getPitch();
    float yaw = filter.getYaw();
    // Convertir yaw de [0,360] vers [-180,180] pour eviter les effets de passage 360->0
    if (yaw > 180.0f) {
      yaw -= 360.0f;
    }
    // Accumuler pour stats
    sumRoll  += roll;
    sumPitch += pitch;
    sumYaw   += yaw;
    sumSqRoll  += roll * roll;
    sumSqPitch += pitch * pitch;
    sumSqYaw   += yaw * yaw;
    delay(10); // ~100Hz
  }
  // Calcul des écarts-types
  double meanRoll  = sumRoll / sampleCount;
  double meanPitch = sumPitch / sampleCount;
  double meanYaw   = sumYaw / sampleCount;
  double varianceRoll  = (sumSqRoll / sampleCount) - (meanRoll * meanRoll);
  double variancePitch = (sumSqPitch / sampleCount) - (meanPitch * meanPitch);
  double varianceYaw   = (sumSqYaw / sampleCount) - (meanYaw * meanYaw);
  if (varianceRoll < 0) varianceRoll = 0;
  if (variancePitch < 0) variancePitch = 0;
  if (varianceYaw < 0) varianceYaw = 0;
  stdRoll  = sqrt(varianceRoll);
  stdPitch = sqrt(variancePitch);
  stdYaw   = sqrt(varianceYaw);
}