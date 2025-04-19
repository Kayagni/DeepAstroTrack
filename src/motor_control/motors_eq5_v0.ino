#include <TMCStepper.h>

// === Configuration PIN ===
#define EN_PIN       25  // Enable moteur
#define DIR_PIN      27  // Direction
#define STEP_PIN     26  // Step pulse
#define SERIAL_PORT  Serial2
#define R_SENSE      0.11f

#define TMC_ADDRESS  0b00
#define FREQ_SIDEREAL 154.158

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, TMC_ADDRESS);

// Flag pour STEP
volatile bool doStep = false;

// Timer haute résolution
esp_timer_handle_t step_timer;

// Callback pour timer
void IRAM_ATTR onTimerStep(void* arg) {
  doStep = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  // UART vers TMC2209
  SERIAL_PORT.begin(115200, SERIAL_8N1, 16, 17);

  // Init driver
  driver.begin();
  driver.toff(4);
  driver.rms_current(350);
  driver.microsteps(16);
  driver.en_spreadCycle(false);
  driver.pwm_autoscale(true);
  driver.pwm_autograd(true);

  // Configurer le timer avec la nouvelle API
  const esp_timer_create_args_t timer_args = {
    .callback = &onTimerStep,
    .arg = nullptr,
    .name = "sidereal_step_timer"
  };

  esp_timer_create(&timer_args, &step_timer);
  esp_timer_start_periodic(step_timer, 1000000.0 / FREQ_SIDEREAL); // µs
  Serial.println("Timer sidéral démarré.");
}

void loop() {
  if (doStep) {
    doStep = false;
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(STEP_PIN, LOW);
  }
}
