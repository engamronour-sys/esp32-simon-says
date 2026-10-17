/*
  STEP 1 - Hardware test for the 3-button Simon Says (ESP32)

  Upload this first. Then open Tools > Serial Monitor, set it to 115200 baud,
  and press the EN (or RST) button on the ESP32 to run the test from the start.

  Wiring this sketch expects:
    LED 1 -> GPIO 25    Button 1 -> GPIO 32
    LED 2 -> GPIO 26    Button 2 -> GPIO 33
    LED 3 -> GPIO 27    Button 3 -> GPIO 14
    Buzzer (+) -> GPIO 13
    Everything else -> GND
*/

const int LED_PINS[3]    = {25, 26, 27};   // LED 1, LED 2, LED 3
const int BUTTON_PINS[3] = {32, 33, 14};   // Button 1, Button 2, Button 3
const int BUZZER_PIN     = 13;
const int NOTES[3]       = {523, 659, 784}; // a different beep for each LED

bool wasPressed[3] = {false, false, false};

void setup() {
  Serial.begin(115200);
  delay(1000);  // give the Serial Monitor a moment to connect

  for (int i = 0; i < 3; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);  // button not pressed = HIGH, pressed = LOW
  }
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println();
  Serial.println("=== Hardware test ===");

  // Part A: each LED with a beep
  Serial.println("Part A: each LED lights for 1 second, with a beep.");
  for (int i = 0; i < 3; i++) {
    Serial.printf("  LED %d (GPIO %d) should be ON now\n", i + 1, LED_PINS[i]);
    digitalWrite(LED_PINS[i], HIGH);
    tone(BUZZER_PIN, NOTES[i]);
    delay(1000);
    noTone(BUZZER_PIN);
    digitalWrite(LED_PINS[i], LOW);
    delay(300);
  }

  // Part B: buzzer pin simply switched on (tells active and passive buzzers apart)
  Serial.println("Part B: buzzer pin switched plainly ON for 1 second.");
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("  Beeped in Part A, but only a click (or nothing) in Part B -> PASSIVE buzzer.");
  Serial.println("     Leave  ACTIVE_BUZZER = false  in the game.");
  Serial.println("  Steady beep in Part B -> ACTIVE buzzer.");
  Serial.println("     Change it to  ACTIVE_BUZZER = true  in the game.");

  // Part C: buttons (handled in loop)
  Serial.println("Part C: press each button. Its LED should light while you hold it.");
}

void loop() {
  for (int i = 0; i < 3; i++) {
    bool pressed = (digitalRead(BUTTON_PINS[i]) == LOW);
    digitalWrite(LED_PINS[i], pressed ? HIGH : LOW);
    if (pressed != wasPressed[i]) {
      Serial.printf("  Button %d (GPIO %d) %s\n", i + 1, BUTTON_PINS[i],
                    pressed ? "PRESSED" : "released");
      wasPressed[i] = pressed;
    }
  }
  delay(10);
}
