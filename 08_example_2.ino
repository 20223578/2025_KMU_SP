#define PIN_LED      9        // PWM uchun
#define PIN_TRIG     12       // sonar TRIGGER
#define PIN_ECHO     13       // sonar ECHO

// ====== Parametrlar ======
#define SND_VEL 346.0      // m/s (24°C)
#define INTERVAL 25        
#define PULSE_DURATION 10  // us: TRIG pulse
#define _DIST_MIN_PWM 100.0 // PWM minimal boshlanish masofasi
#define _DIST_MAX_PWM 300.0 // PWM maksimal tugash masofasi


#define TIMEOUT 2000 

#define SCALE   (0.001 * 0.5 * SND_VEL)


unsigned long last_sampling_time = 0;  // ms

unsigned long t0_ms = 0;
int           out_count = 0;
unsigned long prev_ms = 0;
bool          have_prev = false;
unsigned long sum_dt = 0, min_dt = 0xFFFFFFFF, max_dt = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);  
  Serial.println("Masofa(mm), PWM_Duty"); 

  
  last_sampling_time = millis() - INTERVAL;
  
  
  analogWrite(PIN_LED, 255);
}

void loop() {
  // Non-blocking rejalashtirish
  unsigned long now = millis();
  if ((unsigned long)(now - last_sampling_time) < INTERVAL) return;
  last_sampling_time += INTERVAL;  


  float distance = USS_measure(PIN_TRIG, PIN_ECHO); // mm
  int pwm_value = 255; // Boshlang'ich holat: LED o'chiq (Active Low)
  
  
  if (distance == 0.0) {
      distance = _DIST_MAX_PWM + 1.0; 
  }

  if (distance < _DIST_MIN_PWM || distance > _DIST_MAX_PWM) {
   
    pwm_value = 255;
    
  } else if (distance >= _DIST_MIN_PWM && distance <= 200.0) {
   
    // (distance - 100.0) oralig'i: 0 dan 100 gacha
    float ratio = (distance - _DIST_MIN_PWM) / 100.0; 
    pwm_value = (int)(255.0 - (ratio * 255.0)); // 255 (o'chiq) dan 0 (yorug') gacha
    
  } else if (distance > 200.0 && distance <= _DIST_MAX_PWM) {
   
    
    float ratio = (_DIST_MAX_PWM - distance) / 100.0;
    pwm_value = (int)(255.0 - (ratio * 255.0)); // 0 (yorug') dan 255 (o'chiq) gacha
    
  }
  
  // PWM qiymatini 0 dan 255 oralig'ida ushlab turish (float hisobidan chetlashishni oldini olish)
  if (pwm_value < 0) pwm_value = 0;
  if (pwm_value > 255) pwm_value = 255;

  analogWrite(PIN_LED, pwm_value);

  Serial.print(distance); 
  Serial.print(","); 
  Serial.println(255 - pwm_value); 

  unsigned long dt;
  if (!have_prev) { dt = INTERVAL; have_prev = true; }
  else            { dt = now - prev_ms; }
  prev_ms = now;

  // Qolgan statistika hisoblari avvalgidek qoladi...
  if (out_count == 0) t0_ms = now;
  out_count++;

  sum_dt += dt;
  if (dt < min_dt) min_dt = dt;
  if (dt > max_dt) max_dt = dt;

  if (out_count == 10) {
    // Serial Monitor uchun...
    unsigned long total = now - t0_ms;
    float avg = sum_dt / 10.0;

    Serial.print("=> 10 ta chiqish vaqti: ");
    Serial.print(total); Serial.print(" ms");
    Serial.print("  | avg="); Serial.print(avg, 1); Serial.print(" ms");
    Serial.print("  min=");  Serial.print(min_dt);  Serial.print(" ms");
    Serial.print("  max=");  Serial.print(max_dt);  Serial.println(" ms");

    out_count = 0; sum_dt = 0; min_dt = 0xFFFFFFFF; max_dt = 0;
    have_prev = false;
  }
}

// --- HC-SR04 o'lchovi: mm qaytaradi ---
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  // TIMEOUT 2000us
  unsigned long dur_us = pulseIn(ECHO, HIGH, TIMEOUT); // round-trip, us
  
  if (dur_us == 0) {
    return 0.0; // Muvaffaqiyatsiz o'lchov
  }
  
  return dur_us * SCALE; // mm
}
