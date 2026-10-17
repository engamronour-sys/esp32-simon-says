/*
  STEP 2 - Simon Says game: 3 LEDs, 3 buttons, 1 buzzer, ESP32

  How to play:
    - The LEDs chase around while the game waits. Press any button to start.
    - Watch the LEDs flash a pattern, then press the matching buttons in order.
    - Every round adds one more step and plays a little faster.
    - A wrong press or taking too long ends the game. The correct LED blinks
      3 times, then all LEDs blink once for every round you completed (your score).
    - Scores also appear in Tools > Serial Monitor at 115200 baud.

  Wiring:
    LED 1 -> GPIO 25    Button 1 -> GPIO 32
    LED 2 -> GPIO 26    Button 2 -> GPIO 33
    LED 3 -> GPIO 27    Button 3 -> GPIO 14
    Buzzer (+) -> GPIO 13
    Everything else -> GND
*/

// ---------------- Settings you can change ----------------
const bool ACTIVE_BUZZER = true;    // this build uses an active buzzer; set to false for a passive one

const int LED_PINS[3]    = {25, 26, 27};
const int BUTTON_PINS[3] = {32, 33, 14};
const int BUZZER_PIN     = 13;
const int NOTES[3]       = {523, 659, 784};  // C, E, G: one note per LED

const int MAX_STEPS = 50;                    // remember this many and you win
const unsigned long TIME_TO_ANSWER = 3000;   // milliseconds allowed for each press
// ----------------------------------------------------------

const int NUM_BUTTONS = 3;
int sequence[MAX_STEPS];   // the pattern the player has to copy
int level = 0;             // how many steps the pattern has right now

// ---------- Small helpers ----------

void buzzerOn(int frequency) {
  if (ACTIVE_BUZZER) {
    digitalWrite(BUZZER_PIN, HIGH);   // active buzzers make their own sound
  } else {
    tone(BUZZER_PIN, frequency);      // passive buzzers need a note
  }
}

void buzzerOff() {
  if (ACTIVE_BUZZER) {
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    noTone(BUZZER_PIN);
  }
}

void setAllLeds(bool on) {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    digitalWrite(LED_PINS[i], on ? HIGH : LOW);
  }
}

bool isPressed(int i) {
  return digitalRead(BUTTON_PINS[i]) == LOW;  // LOW means pressed (INPUT_PULLUP)
}

// Returns 0, 1 or 2 for the button being held, or -1 if none
int whichButtonIsPressed() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (isPressed(i)) return i;
  }
  return -1;
}

void waitUntilAllReleased() {
  while (whichButtonIsPressed() != -1) delay(5);
  delay(30);  // let the contacts settle
}

// Light one LED with its note
void flash(int i, int ms) {
  digitalWrite(LED_PINS[i], HIGH);
  buzzerOn(NOTES[i]);
  delay(ms);
  buzzerOff();
  digitalWrite(LED_PINS[i], LOW);
}

// ---------- Setup ----------

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);  // give the Serial Monitor a moment to connect
  printSettings();
}

// Prints what this program expects, so you can check the right code is on the board
void printSettings() {
  Serial.println();
  Serial.println("Simon Says (ESP32) - game loaded and running");
  Serial.printf("  LEDs:    GPIO %d, %d, %d\n", LED_PINS[0], LED_PINS[1], LED_PINS[2]);
  Serial.printf("  Buttons: GPIO %d, %d, %d\n", BUTTON_PINS[0], BUTTON_PINS[1], BUTTON_PINS[2]);
  Serial.printf("  Buzzer:  GPIO %d (%s)\n", BUZZER_PIN, ACTIVE_BUZZER ? "active" : "passive");
}

// ---------- Main loop: wait for a start press, then play ----------

void loop() {
  waitForStart();
  playOneGame();
}

// LEDs chase around until any button is pressed
void waitForStart() {
  Serial.println("Press any button to start.");
  int i = 0;
  while (true) {
    digitalWrite(LED_PINS[i], HIGH);
    unsigned long started = millis();
    while (millis() - started < 200) {
      if (whichButtonIsPressed() != -1) {
        setAllLeds(false);
        randomSeed(micros());  // the exact moment you pressed makes every game different
        waitUntilAllReleased();
        delay(600);
        return;
      }
      delay(1);
    }
    digitalWrite(LED_PINS[i], LOW);
    i = (i + 1) % NUM_BUTTONS;
  }
}

void playOneGame() {
  level = 0;
  while (level < MAX_STEPS) {
    sequence[level] = random(NUM_BUTTONS);  // add one new random step
    level++;
    Serial.printf("Level %d\n", level);

    showSequence();

    for (int step = 0; step < level; step++) {
      int pressed = readPress();
      if (pressed != sequence[step]) {
        gameOver(pressed, sequence[step]);
        return;
      }
    }
    delay(800);  // short pause before the next, longer pattern
  }
  youWin();
}

// Play the whole pattern; it speeds up as it gets longer
void showSequence() {
  int onTime = max(180, 500 - (level - 1) * 20);
  for (int s = 0; s < level; s++) {
    flash(sequence[s], onTime);
    delay(onTime / 2);
  }
}

// Wait for the player to press a button. Returns 0-2, or -1 if they ran out of time.
int readPress() {
  unsigned long started = millis();
  int b = -1;
  while (b == -1) {
    if (millis() - started > TIME_TO_ANSWER) return -1;
    b = whichButtonIsPressed();
    delay(1);
  }
  // Light the LED and play its note for as long as the button is held
  digitalWrite(LED_PINS[b], HIGH);
  buzzerOn(NOTES[b]);
  delay(30);                       // ignore contact bounce
  while (isPressed(b)) delay(5);
  buzzerOff();
  digitalWrite(LED_PINS[b], LOW);
  delay(30);
  return b;
}

void gameOver(int pressed, int correct) {
  int score = level - 1;  // rounds fully completed
  if (pressed == -1) {
    Serial.printf("Too slow! Game over. Score: %d\n", score);
  } else {
    Serial.printf("Wrong button! Game over. Score: %d\n", score);
  }

  // Low "fail" sound with all LEDs on
  setAllLeds(true);
  buzzerOn(150);
  delay(700);
  buzzerOff();
  setAllLeds(false);
  delay(400);

  // Show which LED was the right one
  for (int k = 0; k < 3; k++) {
    digitalWrite(LED_PINS[correct], HIGH);
    delay(200);
    digitalWrite(LED_PINS[correct], LOW);
    delay(200);
  }
  delay(700);

  // Blink all LEDs once per point
  for (int k = 0; k < score; k++) {
    setAllLeds(true);
    delay(150);
    setAllLeds(false);
    delay(250);
  }
  delay(1000);
  waitUntilAllReleased();
}

void youWin() {
  Serial.printf("You win! You remembered all %d steps!\n", MAX_STEPS);
  for (int r = 0; r < 5; r++) {
    for (int i = 0; i < NUM_BUTTONS; i++) flash(i, 90);
  }
  delay(1000);
}
