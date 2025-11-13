#define PIN_LED 7   // LED tashqi ulanishda ishlatiladigan pin

void setup() {
  pinMode(PIN_LED, OUTPUT);   // Pin 7 ni chiqish rejimiga o‘tkazamiz
}

void loop() {

  // 1) Dastur boshlanishida LED 1 sekund davomida YONIQ
  digitalWrite(PIN_LED, HIGH);   // LED ON
  delay(1000);                   // 1 sekund kutish

  // 2) Keyingi 1 sekund ichida LEDni 5 marta blink qilish
  for (int i = 0; i < 5; i++) {
    digitalWrite(PIN_LED, HIGH);   // LED ON
    delay(100);                    // 0.1s
    digitalWrite(PIN_LED, LOW);    // LED OFF
    delay(100);                    // 0.1s
  }

  // 3) LEDni O‘CHIRIB, cheksiz loopga o‘tish
  digitalWrite(PIN_LED, LOW);  // LED OFF

  while (1) {
    // Infinite loop: dastur shu yerda tugaydi
  }
}
