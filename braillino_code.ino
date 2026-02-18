#include <Servo.h>

// ========== PINS ==========
// IMPORTANT: pin 9 = RIGHT, pin 10 = LEFT
#define RIGHT_SERVO_PIN 9
#define LEFT_SERVO_PIN 10

Servo leftServo;
Servo rightServo;

// ========== SERVO CALIBRATION =========
// Index = state 0..7 (binary 000..111)

// LEFT SERVO (pin 10)
int LEFT_POS[8] = {
  180,  // 000
  40,   // 001
  30,   // 010
  142,  // 011
  10,   // 100
  52,   // 101
  75,   // 110
  100   // 111
};

// RIGHT SERVO (pin 9)
int RIGHT_POS[8] = {
  180,  // 000
  0,    // 001
  20,   // 010
  70,   // 011
  35,   // 100
  42,   // 101
  130,  // 110
  90    // 111
};

// ========== DOT BIT DEFINITIONS ==========
#define DOT_TOP 0b100
#define DOT_MID 0b010
#define DOT_BOT 0b001

// ========== TEXT ==========
String currentText = "DANIEL";
int indexChar = 0;
unsigned long lastUpdate = 0;
const unsigned long DISPLAY_TIME = 4000;
const unsigned long DOUBLE_LETTER_GAP = 400; // short "dots down" gap between double letters

bool showingGap = false;
char lastDisplayedChar = ' ';
bool wordFinished = false; // NEW: track if we've displayed the whole word

void setup() {
  Serial.begin(9600);

  // Attach with corrected pin mapping
  rightServo.attach(RIGHT_SERVO_PIN); // pin 9
  leftServo.attach(LEFT_SERVO_PIN);   // pin 10

  // start blank
  moveServo(leftServo, 0, true);
  moveServo(rightServo, 0, false);

  Serial.println("BRAILLINO - PIN 9 = RIGHT, PIN 10 = LEFT");
  Serial.println("Double letters will show brief gap (e.g., HELLO shows HEL-L-O)");
  Serial.println("Word will display once and stop. Enter new word to restart.");
  Serial.println("Now supports: A-Z, 0-9, and punctuation (. , ? ! ; : - ' \" ( ) / @ #)");
}

void loop() {
  if (Serial.available()) {
    String in = Serial.readStringUntil('\n');
    in.trim();
    if (in.length() > 0) {
      in.toUpperCase();
      currentText = in + " ";
      indexChar = 0;
      showingGap = false;
      lastDisplayedChar = ' ';
      wordFinished = false; // Reset when new word entered
      lastUpdate = millis();
      Serial.print("Displaying: ");
      Serial.println(in);
    }
  }

  // Stop if word is finished
  if (wordFinished) {
    return;
  }

  if (millis() - lastUpdate >= (showingGap ? DOUBLE_LETTER_GAP : DISPLAY_TIME)) {

    // If we just finished showing a gap, now show the repeated letter
    if (showingGap) {
      showingGap = false;
      char c = currentText.charAt(indexChar);
      showBraille(c);
      lastDisplayedChar = c;
      
      // Move to next character
      indexChar++;
      if (indexChar >= currentText.length()) {
        wordFinished = true; // Word complete!
        Serial.println("Word finished. Enter new word to continue.");
        return;
      }
      
      lastUpdate = millis();
      return;
    }

    // Normal character display
    char c = currentText.charAt(indexChar);
    
    // Check if this is a repeated letter (same as last displayed)
    if (c != ' ' && c == lastDisplayedChar) {
      // Show blank gap first
      showBraille(' ');
      showingGap = true;
      lastUpdate = millis();
      // Don't increment indexChar yet - we'll show the repeated letter after the gap
    } else {
      // Show the character normally
      showBraille(c);
      lastDisplayedChar = c;
      
      // Move to next character
      indexChar++;
      if (indexChar >= currentText.length()) {
        wordFinished = true; // Word complete!
        Serial.println("Word finished. Enter new word to continue.");
        return;
      }
      
      lastUpdate = millis();
    }
  }
}

// ========== BRAILLE LOGIC ==========
void showBraille(char c) {
  uint8_t L = 0;
  uint8_t R = 0;

  switch (c) {
    // Letters A-J
    case 'A': L = DOT_TOP; break;
    case 'B': L = DOT_TOP | DOT_MID; break;
    case 'C': L = DOT_TOP; R = DOT_TOP; break;
    case 'D': L = DOT_TOP; R = DOT_TOP | DOT_MID; break;
    case 'E': L = DOT_TOP; R = DOT_MID; break;
    case 'F': L = DOT_TOP | DOT_MID; R = DOT_TOP; break;
    case 'G': L = DOT_TOP | DOT_MID; R = DOT_TOP | DOT_MID; break;
    case 'H': L = DOT_TOP | DOT_MID; R = DOT_MID; break;
    case 'I': L = DOT_MID; R = DOT_TOP; break;
    case 'J': L = DOT_MID; R = DOT_TOP | DOT_MID; break;

    // Letters K-T
    case 'K': L = DOT_TOP | DOT_BOT; break;
    case 'L': L = DOT_TOP | DOT_MID | DOT_BOT; break;
    case 'M': L = DOT_TOP | DOT_BOT; R = DOT_TOP; break;
    case 'N': L = DOT_TOP | DOT_BOT; R = DOT_TOP | DOT_MID; break;
    case 'O': L = DOT_TOP | DOT_BOT; R = DOT_MID; break;
    case 'P': L = DOT_TOP | DOT_MID | DOT_BOT; R = DOT_TOP; break;
    case 'Q': L = DOT_TOP | DOT_MID | DOT_BOT; R = DOT_TOP | DOT_MID; break;
    case 'R': L = DOT_TOP | DOT_MID | DOT_BOT; R = DOT_MID; break;
    case 'S': L = DOT_MID | DOT_BOT; R = DOT_TOP; break;
    case 'T': L = DOT_MID | DOT_BOT; R = DOT_TOP | DOT_MID; break;

    // Letters U-Z
    case 'U': L = DOT_TOP | DOT_BOT; R = DOT_BOT; break;
    case 'V': L = DOT_TOP | DOT_MID | DOT_BOT; R = DOT_BOT; break;
    case 'W': L = DOT_MID; R = DOT_TOP | DOT_MID | DOT_BOT; break;
    case 'X': L = DOT_TOP | DOT_BOT; R = DOT_TOP | DOT_BOT; break;
    case 'Y': L = DOT_TOP | DOT_BOT; R = DOT_TOP | DOT_MID | DOT_BOT; break;
    case 'Z': L = DOT_TOP | DOT_BOT; R = DOT_MID | DOT_BOT; break;

    // Numbers (0-9)
    case '0': L = DOT_MID; R = DOT_TOP | DOT_MID; break;  // same as J
    case '1': L = DOT_TOP; break;  // same as A
    case '2': L = DOT_TOP | DOT_MID; break;  // same as B
    case '3': L = DOT_TOP; R = DOT_TOP; break;  // same as C
    case '4': L = DOT_TOP; R = DOT_TOP | DOT_MID; break;  // same as D
    case '5': L = DOT_TOP; R = DOT_MID; break;  // same as E
    case '6': L = DOT_TOP | DOT_MID; R = DOT_TOP; break;  // same as F
    case '7': L = DOT_TOP | DOT_MID; R = DOT_TOP | DOT_MID; break;  // same as G
    case '8': L = DOT_TOP | DOT_MID; R = DOT_MID; break;  // same as H
    case '9': L = DOT_MID; R = DOT_TOP; break;  // same as I

    // Punctuation
    case '.': L = DOT_MID; R = DOT_MID | DOT_BOT; break;  // period
    case ',': L = DOT_MID; break;  // comma
    case '?': L = DOT_MID | DOT_BOT; R = DOT_BOT; break;  // question mark
    case '!': L = DOT_MID | DOT_BOT; R = DOT_MID; break;  // exclamation
    case ';': L = DOT_MID | DOT_BOT; break;  // semicolon
    case ':': L = DOT_MID; R = DOT_MID; break;  // colon
    case '-': L = 0; R = DOT_BOT; break;  // hyphen/dash
    case '\'': L = 0; R = 0; break;  // apostrophe (blank - adjust as needed)
    case '"': L = DOT_MID | DOT_BOT; R = DOT_MID | DOT_BOT; break;  // quotation mark
    case '(': L = DOT_MID | DOT_BOT; R = DOT_MID | DOT_BOT; break;  // left paren (same as quote)
    case ')': L = DOT_MID | DOT_BOT; R = DOT_TOP | DOT_BOT; break;  // right paren
    case '/': L = 0; R = DOT_TOP | DOT_BOT; break;  // forward slash
    case '@': L = 0; R = DOT_TOP | DOT_MID; break;  // at symbol
    case '#': L = 0; R = DOT_TOP | DOT_MID | DOT_BOT; break;  // hash/number sign

    // Space
    case ' ': L = 0; R = 0; break;
    
    // Default case for unsupported characters
    default: L = 0; R = 0; break;
  }

  moveServo(leftServo, L, true);   // LEFT → pin 10
  moveServo(rightServo, R, false); // RIGHT → pin 9
}

// ========== SERVO MOVE ==========
void moveServo(Servo &s, uint8_t state, bool isLeft) {
  state &= 0b111;
  int pos = isLeft ? LEFT_POS[state] : RIGHT_POS[state];
  s.write(constrain(pos, 0, 180));
}
