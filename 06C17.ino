#define PIN_LED 7

//도전 과제 2


int g_period_us   = 1000; // 주기 (단위: us, 100 ~ 10000)
int g_duty_percent = 0;   // 듀티 (단위: %, 0 ~ 100)

void set_period(int period) {
  if (period < 100)   period = 100;
  if (period > 10000) period = 10000;
  g_period_us = period;
}

void set_duty(int duty) {
  if (duty < 0)   duty = 0;
  if (duty > 100) duty = 100;
  g_duty_percent = duty;
}

void pwm_one_cycle() {
  long on_time  = (long)g_period_us * g_duty_percent / 100;   // LED ON 시간
  long off_time = g_period_us - on_time;                      // LED OFF 시간

  // LED ON (LOW)
  if (on_time > 0) {
    digitalWrite(PIN_LED, LOW);          // LED 켜기 (ACTIVE-LOW)
    delayMicroseconds(on_time);
  }

  // LED OFF (HIGH)
  if (off_time > 0) {
    digitalWrite(PIN_LED, HIGH);         // LED 끄기 (ACTIVE-LOW)
    delayMicroseconds(off_time);
  }
}

void triangle_1sec() {
  const int total_steps = 202;                 // 0→100 (101) + 100→0 (101)
  long total_time_us = 1000000L;              // 1초 = 1,000,000us

  int cycles_per_step = (int)(total_time_us / (total_steps * (long)g_period_us));
  if (cycles_per_step < 1) cycles_per_step = 1;  // 최소 1번은 수행

  // 0% → 100%
  for (int d = 0; d <= 100; d++) {
    set_duty(d);
    for (int i = 0; i < cycles_per_step; i++) {
      pwm_one_cycle();
    }
  }

  // 100% → 0%
  for (int d = 100; d >= 0; d--) {
    set_duty(d);
    for (int i = 0; i < cycles_per_step; i++) {
      pwm_one_cycle();
    }
  }
}
void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);   // ACTIVE-LOW: HIGH = LED OFF (초기상태 끔)
}

void loop() {
  // 1) period = 10ms (10000us)
  set_period(10000);
  triangle_1sec();
  delay(500); // 잠깐 쉬기 (영상에서 구분 잘 되도록)

  // 2) period = 1ms (1000us)
  set_period(1000);
  triangle_1sec();
  delay(500);

  // 3) period = 0.1ms (100us)
  set_period(100);
  triangle_1sec();
  delay(1000); // 다음 반복까지 대기
}
