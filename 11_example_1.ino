#include <Servo.h>

/* ===== Pins ===== */
#define PIN_LED    9
#define PIN_TRIG  12
#define PIN_ECHO  13
#define PIN_SERVO 10

/* ===== Sonar config ===== */
#define SND_VEL        346.0         // m/s @24°C
#define INTERVAL       25            // ms
#define PULSE_DURATION 10            // us
#define _DIST_MIN      180.0         // mm  (18 cm)
#define _DIST_MAX      360.0         // mm  (36 cm)
#define TIMEOUT        ((INTERVAL / 2) * 1000.0)  // us
#define SCALE          (0.001 * 0.5 * SND_VEL)    // pulse(us) -> mm
#define _EMA_ALPHA     0.30          // 0..1

/* ===== Servo (kalibrlangan qiymatlar) =====
   Kerak bo‘lsa kalibrlash sketchi bilan moslab o‘zgartiring.
*/
#define _DUTY_MIN 1000   // 0°   (µs)
#define _DUTY_MAX 2000   // 180° (µs)
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX) / 2)

/* ===== Globals ===== */
Servo myservo;
float dist_prev = _DIST_MIN;  // mm
float dist_ema  = _DIST_MIN;  // mm
unsigned long last_sampling_time = 0;

static inline float clampf(float v, float lo, float hi) {
  return (v < lo) ? lo : (v > hi) ? hi : v;
}

float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;  // mm
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(57600);
}

void loop() {
  if (millis() < last_sampling_time + INTERVAL) return;

  /* 1) Raw distance */
  float dist_raw  = USS_measure(PIN_TRIG, PIN_ECHO);
  float dist_filt;

  /* 2) Range filter: faqat 18~36cm qabul qilamiz */
  if (dist_raw == 0.0 || dist_raw < _DIST_MIN || dist_raw > _DIST_MAX) {
    dist_filt = dist_prev;            // tashqarida bo‘lsa – oldingi qiymatni ushlab tur
    digitalWrite(PIN_LED, LOW);       // oraliqdan tashqarida
  } else {
    dist_filt = dist_raw;
    dist_prev = dist_raw;
    digitalWrite(PIN_LED, HIGH);      // oraliq ichida
  }

  /* 3) EMA filter */
  static bool ema_inited = false;
  if (!ema_inited) { dist_ema = dist_filt; ema_inited = true; }
  else             { dist_ema = _EMA_ALPHA*dist_filt + (1.0-_EMA_ALPHA)*dist_ema; }

  /* 4) Continuous angle control (18cm->0°, 36cm->180°) */
  float d_mm   = clampf(dist_ema, _DIST_MIN, _DIST_MAX);
  float ratio  = (d_mm - _DIST_MIN) / (_DIST_MAX - _DIST_MIN); // 0..1
  int duty_us  = (int)(_DUTY_MIN + ratio * (_DUTY_MAX - _DUTY_MIN));
  myservo.writeMicroseconds(duty_us);

  // Gradusni ham chiqaramiz (plotterda ko‘rish qulay bo‘lsin)
  int servo_deg = (int)(ratio * 180.0f + 0.5f);

  /* 5) Serial Plotter chiqishi (slayd formatiga mos + ServoUS) */
  Serial.print("Min:");     Serial.print(_DIST_MIN);
  Serial.print(",dist:");   Serial.print(dist_raw);
  Serial.print(",ema:");    Serial.print(dist_ema);
  Serial.print(",Servo:");  Serial.print(servo_deg);     // 0..180°
  Serial.print(",ServoUS:");Serial.print(duty_us);       // µs – diagnostika uchun
  Serial.print(",Max:");    Serial.print(_DIST_MAX);
  Serial.println("");

  last_sampling_time += INTERVAL;
}
