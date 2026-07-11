#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Define DHT sensor settings
#define DHTPIN A1         // DHT sensor connected to analog pin A1
#define DHTTYPE DHT11     // Using DHT11 sensor

// Define pin connections
#define BUZZER_PIN 7      // Buzzer connected to digital pin 7
#define BUTTON_PIN 2      // Push button connected to digital pin 2

// Motor control pins
#define MOTOR_ENABLE_PIN 8  // PWM pin to control motor speed
#define MOTOR_IN1 10        // Motor direction pin 1
#define MOTOR_IN2 9         // Motor direction pin 2

// HC-SR04 Ultrasonic sensor pins
#define TRIG_PIN 5         // Trigger pin
#define ECHO_PIN 4         // Echo pin

// On-board LED pin
#define LED_PIN 13         // LED connected to digital pin 13

// Initialize DHT sensor and LCD
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD at I2C address 0x27, 16 columns x 2 rows

// Timing variables for LCD update
unsigned long previousDisplayTime = 0;           // Last time LCD was updated
const unsigned long displayInterval = 1000;      // Update interval for LCD (1 second)

// Timing variables for buzzer toggle
unsigned long buzzerToggleTime = 0;              // Last time buzzer toggled
const unsigned long buzzerInterval = 300;        // Interval for buzzer toggle (300 ms)
bool buzzerState = false;                        // Current state of buzzer

// Button press management
unsigned long buttonPressedTime = 0;             // Last time button was pressed
bool buttonCooldown = false;                     // Prevent repeated button presses within 10 seconds

// Distance measurement timing
unsigned long previousDistanceTime = 0;          // Last time distance was measured
const unsigned long distanceInterval = 70;       // Distance check interval (70 ms)

// Distance and LED timer variables
float distanceCm = 0;                            // Measured distance in centimeters
unsigned long ledOnStartTime = 0;                // Time when LED first turned on
bool ledOnContinuously = false;                  // Flag for LED being on continuously for 15 seconds

void setup() {
  // Setup pin modes
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  // Set motor direction (IN1 high, IN2 low => forward)
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);

  // Initialize LCD and DHT sensor
  lcd.init();
  lcd.backlight();
  dht.begin();
}

void loop() {
  unsigned long currentTime = millis(); // Get current time

  // Handle button press to disable buzzer temporarily
  if (digitalRead(BUTTON_PIN) == LOW && !buttonCooldown) {
    buttonCooldown = true;
    buttonPressedTime = currentTime;
    noTone(BUZZER_PIN); // Turn off buzzer immediately when button pressed
  }

  // Reset button cooldown after 10 seconds
  if (buttonCooldown && (currentTime - buttonPressedTime >= 10000)) {
    buttonCooldown = false;
  }

  // Measure distance at intervals
  if (currentTime - previousDistanceTime >= distanceInterval) {
    previousDistanceTime = currentTime;
    long duration = measureDistance();        // Get echo time
    distanceCm = duration / 58.0;             // Convert to centimeters

    // Handle out-of-range or invalid values
    if (distanceCm == 0 || distanceCm > 400) {
      distanceCm = 0;
    }
  }

  // Control LED based on measured distance
  if (distanceCm > 10) {
    digitalWrite(LED_PIN, HIGH);

    // Start or continue LED-on timer
    if (!ledOnContinuously) {
      if (ledOnStartTime == 0) {
        ledOnStartTime = currentTime;  // Start timing
      } else if (currentTime - ledOnStartTime >= 15000) {
        ledOnContinuously = true;      // Set flag after 15 seconds
      }
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    ledOnStartTime = 0;                // Reset timer and flag
    ledOnContinuously = false;
  }

  // Read temperature and humidity from DHT11
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Update LCD display every 1 second
  if (currentTime - previousDisplayTime >= displayInterval) {
    previousDisplayTime = currentTime;

    lcd.setCursor(0, 0);
    lcd.print("TEMP(C): ");
    if (isnan(temp)) {
      lcd.print("Error ");
    } else {
      lcd.print(temp, 1); // Show 1 decimal place
      lcd.print("   ");
    }

    lcd.setCursor(0, 1);
    lcd.print("HUM (%): ");
    if (isnan(hum)) {
      lcd.print("Error ");
    } else {
      lcd.print(hum, 1);  // Show 1 decimal place
      lcd.print("   ");
    }
  }

  // Control motor speed based on temperature
  if (temp <= 20.0) {
    setMotorSpeed(0);        // Stop motor if temperature is too low
  } else {
    setMotorSpeed(255);      // Run motor at full speed otherwise
  }

  // Activate buzzer in 300ms pulses if LED was on for 15 seconds and button was not pressed
  if (ledOnContinuously && !buttonCooldown) {
    if (currentTime - buzzerToggleTime >= buzzerInterval) {
      buzzerToggleTime = currentTime;
      buzzerState = !buzzerState;

      if (buzzerState) {
        tone(BUZZER_PIN, 1000); // Turn buzzer on (1kHz)
      } else {
        noTone(BUZZER_PIN);     // Turn buzzer off
      }
    }
  } else {
    noTone(BUZZER_PIN);         // Ensure buzzer is off
  }
}

// Function to measure distance using ultrasonic sensor (HC-SR04)
long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);     // Send 10us HIGH pulse to trigger
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read duration of pulse from echo pin (timeout after 30ms)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  return duration;
}

// Function to set motor speed using PWM (value from 0 to 255)
void setMotorSpeed(int speed) {
  analogWrite(MOTOR_ENABLE_PIN, speed);
}
