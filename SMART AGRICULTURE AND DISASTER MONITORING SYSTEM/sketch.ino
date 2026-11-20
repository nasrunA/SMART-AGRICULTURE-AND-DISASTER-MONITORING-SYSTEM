/***
 * BLYNK CONFIG
 ***/
#define BLYNK_TEMPLATE_ID "TMPL6BtxHIs3l"
#define BLYNK_TEMPLATE_NAME "SMART AGRICULTURE AND DISASTER SYSTEM"
#define BLYNK_AUTH_TOKEN "REPLACE WITH YOUR BLYNK AUTH TOKEN"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

/***
 * ORIGINAL LIBRARIES
 ***/
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

//THIS IS WIFI CONFIGURATION WORK ONLY ON WOKWI SIMULATOR DO CHANGES WHEN USE YOUR ARDUINO IDE
char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

/***
 * PIN DEFINITIONS
 ***/
#define LED1 32   // Flood LED
#define LED2 33   // Gas LED
#define TEMP_LED 26   // D26 - Temperature HOT LED (ON when temp > 35°C)
#define EARTHQUAKE_LED 25   // D25 - EARTHQUAKE LED (ON when earthquake detected)

#define TRIG_PIN 5
#define ECHO_PIN 19

#define GAS_PIN 34
#define DHT_PIN 27
#define DHT_TYPE DHT22

#define BUZZER_PIN 14

/***
 * OBJECTS
 ***/
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_MPU6050 mpu;

/***
 * THRESHOLDS - UPDATED
 ***/
#define FLOOD_DISTANCE 50.0
#define GAS_THRESHOLD 1000    // Gas triggers at > 800

#define HOT_TEMP 35.0          // TEMP_LED (D26) ON when temperature > 35°C
#define COLD_TEMP 10.0         // TEMP alert when temperature < 10°C

// Humidity thresholds - UPDATED WITH ALERTS
#define DRY_HUM 40.0           // Humidity alert when < 40%
#define WET_HUM 80.0           // Humidity alert when > 80%

#define VIBRATION_THRESHOLD 11.90 //Earthquake alert 9.90

/***
 * GLOBAL VARIABLES
 ***/
long duration;
float distance;
bool mpuConnected = false;

// Alert state tracking to prevent spam
bool floodAlertSent = false;
bool gasAlertSent = false;
bool tempAlertSent = false;
bool humidityAlertSent = false;
bool earthquakeAlertSent = false;

// Previous states for comparison
bool prevFloodState = false;
bool prevGasState = false;
bool prevTempAlertState = false;
bool prevHumidityAlertState = false;
bool prevEarthquakeState = false;

/***
 * BUZZER CONTROL FOR ACTIVE BUZZER
 ***/
void forceBuzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
  analogWrite(BUZZER_PIN, 0);
  pinMode(BUZZER_PIN, INPUT);
  delay(5);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void warningBeep() {
  forceBuzzerOff();
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  forceBuzzerOff();
  delay(150);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  forceBuzzerOff();
}

void emergencyBeep() {
  forceBuzzerOff();
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    forceBuzzerOff();
    delay(100);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    forceBuzzerOff();
    if (i < 2) delay(100);
  }
}

/***
 * BLYNK NOTIFICATION FUNCTIONS
 ***/
void sendFloodAlert(float distance) {
  String message = "🚨 FLOOD ALERT!\n";
  message += "Distance: " + String(distance, 1) + " cm\n";
  message += "Threshold: " + String(FLOOD_DISTANCE) + " cm\n";
  message += "Flood detected! Please check area.";
  
  Blynk.logEvent("flood_alert", message.c_str());
  Serial.println("✅ Flood alert notification sent to Blynk");
}

void sendFloodClear() {
  String message = "✅ FLOOD CLEARED\n";
  message += "Flood level is now normal.";
  
  Blynk.logEvent("flood_clear", message.c_str());
  Serial.println("✅ Flood cleared notification sent to Blynk");
}

void sendGasAlert(int gasValue) {
  String message = "⚠️ GAS LEAK ALERT!\n";
  message += "Gas Level: " + String(gasValue) + "\n";
  message += "Threshold: " + String(GAS_THRESHOLD) + "\n";
  message += "Potential gas leak detected!";
  
  Blynk.logEvent("gas_alert", message.c_str());
  Serial.println("✅ Gas alert notification sent to Blynk");
}

void sendGasClear() {
  String message = "✅ GAS LEVEL NORMAL\n";
  message += "Gas level is now safe.";
  
  Blynk.logEvent("gas_clear", message.c_str());
  Serial.println("✅ Gas cleared notification sent to Blynk");
}

void sendTempAlert(float temperature, bool isHot) {
  String message;
  if (isHot) {
    message = "🔥 HIGH TEMPERATURE ALERT!\n";
    message += "Temperature: " + String(temperature, 1) + "°C\n";
    message += "Threshold: " + String(HOT_TEMP) + "°C\n";
    message += "Temperature is too high!";
  } else {
    message = "❄️ LOW TEMPERATURE ALERT!\n";
    message += "Temperature: " + String(temperature, 1) + "°C\n";
    message += "Threshold: " + String(COLD_TEMP) + "°C\n";
    message += "Temperature is too low!";
  }
  
  Blynk.logEvent("temperature_alert", message.c_str());
  Serial.println("✅ Temperature alert notification sent to Blynk");
}

void sendTempNormal(float temperature) {
  String message = "✅ TEMPERATURE NORMAL\n";
  message += "Current: " + String(temperature, 1) + "°C\n";
  message += "Temperature is now in normal range.";
  
  Blynk.logEvent("temperature_normal", message.c_str());
  Serial.println("✅ Temperature normal notification sent to Blynk");
}

void sendHumidityAlert(float humidity, bool isHigh) {
  String message;
  if (isHigh) {
    message = "💧 HIGH HUMIDITY ALERT!\n";
    message += "Humidity: " + String(humidity, 1) + "%\n";
    message += "Threshold: " + String(WET_HUM) + "%\n";
    message += "Humidity is too high!";
  } else {
    message = "🏜️ LOW HUMIDITY ALERT!\n";
    message += "Humidity: " + String(humidity, 1) + "%\n";
    message += "Threshold: " + String(DRY_HUM) + "%\n";
    message += "Humidity is too low!";
  }
  
  Blynk.logEvent("humidity_alert", message.c_str());
  Serial.println("✅ Humidity alert notification sent to Blynk");
}

void sendHumidityNormal(float humidity) {
  String message = "✅ HUMIDITY NORMAL\n";
  message += "Current: " + String(humidity, 1) + "%\n";
  message += "Humidity is now in normal range.";
  
  Blynk.logEvent("humidity_normal", message.c_str());
  Serial.println("✅ Humidity normal notification sent to Blynk");
}

void sendEarthquakeAlert(float vibration) {
  String message = "🚨 EARTHQUAKE ALERT!\n";
  message += "Vibration: " + String(vibration, 2) + " m/s²\n";
  message += "Threshold: " + String(VIBRATION_THRESHOLD) + " m/s²\n";
  message += "Earthquake detected! Take cover immediately!";
  
  Blynk.logEvent("earthquake_alert", message.c_str());
  Serial.println("✅ Earthquake alert notification sent to Blynk");
}

void sendEarthquakeClear() {
  String message = "✅ EARTHQUAKE CLEARED\n";
  message += "Vibration levels are now normal.";
  
  Blynk.logEvent("earthquake_clear", message.c_str());
  Serial.println("✅ Earthquake cleared notification sent to Blynk");
}

/***
 * SETUP
 ***/
void setup() {
  Serial.begin(115200);
  
  Serial.println("=========================================");
  Serial.println("  SMART AGRICULTURE & DISASTER SYSTEM  ");
  Serial.println("=========================================");
  Serial.println();
  Serial.println("UPDATED THRESHOLDS WITH ALERTS:");
  Serial.print("  Flood Alert: <");
  Serial.println(FLOOD_DISTANCE);
  Serial.print("  Gas Alert: >");
  Serial.println(GAS_THRESHOLD);
  Serial.print("  Temperature Alerts:\n");
  Serial.print("    Hot Alert: > ");
  Serial.print(HOT_TEMP);
  Serial.println("°C");
  Serial.print("    Cold Alert: < ");
  Serial.print(COLD_TEMP);
  Serial.println("°C");
  Serial.print("  Humidity Alerts:\n");
  Serial.print("    Dry Alert: < ");
  Serial.print(DRY_HUM);
  Serial.println("%");
  Serial.print("    Wet Alert: > ");
  Serial.print(WET_HUM);
  Serial.println("%");
  Serial.print("  Earthquake Alert: > ");
  Serial.print(VIBRATION_THRESHOLD);
  Serial.println(" m/s²");
  Serial.println();

  Wire.begin(21, 22);
  delay(100);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(TEMP_LED, OUTPUT);
  pinMode(EARTHQUAKE_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  forceBuzzerOff();
  delay(100);
  
  Serial.print("Testing buzzer... ");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  forceBuzzerOff();
  Serial.println("OK");
  
  Serial.print("Initializing DHT22... ");
  dht.begin();
  Serial.println("OK");
  
  Serial.print("Initializing MPU6050... ");
  if (!mpu.begin()) {
    Serial.println("FAILED - Skipping MPU6050");
    mpuConnected = false;
  } else {
    Serial.println("SUCCESS");
    mpuConnected = true;
  }
  
  Serial.print("Connecting to Blynk... ");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected");
  
  Serial.println();
  Serial.println("ALERT NOTIFICATIONS ENABLED:");
  Serial.println("  • Flood alerts");
  Serial.println("  • Gas leak alerts");
  Serial.println("  • Temperature alerts");
  Serial.println("  • Humidity alerts");
  Serial.println("  • Earthquake alerts");
  
  Serial.println();
  Serial.println("=========================================");
  Serial.println("SYSTEM STARTED WITH NOTIFICATIONS");
  Serial.println("=========================================");
  delay(2000);
}

/***
 * LOOP
 ***/
void loop() {
  Blynk.run();

  bool emergency = false;
  bool warning = false;
  
  static int cycleCount = 0;
  cycleCount++;
  
  Serial.println();
  Serial.println("=========================================");
  Serial.print("READING CYCLE: ");
  Serial.println(cycleCount);
  Serial.println("=========================================");

  // ================= FLOOD SENSOR =================
  Serial.print("FLOOD: ");
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  distance = duration * 0.034 / 2;

  bool currentFloodState = false;
  
  if (distance > 0) {
    Serial.print(distance, 1);
    Serial.print(" cm");
    
    if (distance <= FLOOD_DISTANCE) {
      digitalWrite(LED1, HIGH);
      warning = true;
      currentFloodState = true;
      Serial.print(" [FLOOD ALERT!]");
      
      // Send flood alert notification (only once when state changes)
      if (!prevFloodState) {
        sendFloodAlert(distance);
        floodAlertSent = true;
      }
    } else {
      digitalWrite(LED1, LOW);
      currentFloodState = false;
      Serial.println(" [SAFE]");
      
      // Send flood clear notification
      if (prevFloodState) {
        sendFloodClear();
        floodAlertSent = false;
      }
    }
  } else {
    Serial.println("ERROR reading distance");
    digitalWrite(LED1, LOW);
    currentFloodState = false;
  }
  
  prevFloodState = currentFloodState;

  // ================= GAS SENSOR =================
  Serial.print("GAS: ");
  int gasValue = analogRead(GAS_PIN);
  Serial.print(gasValue);
  Serial.print("/");
  Serial.print(GAS_THRESHOLD);

  bool currentGasState = false;
  
  if (gasValue > GAS_THRESHOLD) {
    digitalWrite(LED2, HIGH);
    warning = true;
    currentGasState = true;
    Serial.print(" [GAS ALERT!]");
    
    // Send gas alert notification
    if (!prevGasState) {
      sendGasAlert(gasValue);
      gasAlertSent = true;
    }
  } else {
    digitalWrite(LED2, LOW);
    currentGasState = false;
    Serial.println(" [SAFE]");
    
    // Send gas clear notification
    if (prevGasState) {
      sendGasClear();
      gasAlertSent = false;
    }
  }
  
  prevGasState = currentGasState;

  // ================= DHT22 SENSOR =================
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  digitalWrite(TEMP_LED, LOW);

  // Temperature alerts
  bool currentTempAlertState = false;
  Serial.print("TEMP: ");
  if (!isnan(temperature)) {
    Serial.print(temperature, 1);
    Serial.print("°C");
    
    if (temperature > HOT_TEMP) {
      digitalWrite(TEMP_LED, HIGH);
      warning = true;
      currentTempAlertState = true;
      Serial.print(" [HOT ALERT!]");
      
      // Send hot temperature alert
      if (!prevTempAlertState) {
        sendTempAlert(temperature, true);
        tempAlertSent = true;
      }
    } else if (temperature < COLD_TEMP) {
      warning = true;
      currentTempAlertState = true;
      Serial.print(" [COLD ALERT!]");
      
      // Send cold temperature alert
      if (!prevTempAlertState) {
        sendTempAlert(temperature, false);
        tempAlertSent = true;
      }
    } else {
      currentTempAlertState = false;
      Serial.println(" [NORMAL]");
      
      // Send temperature normal notification
      if (prevTempAlertState) {
        sendTempNormal(temperature);
        tempAlertSent = false;
      }
    }
  } else {
    Serial.println("ERROR reading temperature");
    currentTempAlertState = false;
  }
  
  prevTempAlertState = currentTempAlertState;

  // Humidity alerts
  bool currentHumidityAlertState = false;
  Serial.print("HUMIDITY: ");
  if (!isnan(humidity)) {
    Serial.print(humidity, 1);
    Serial.print("%");
    
    if (humidity > WET_HUM) {
      warning = true;
      currentHumidityAlertState = true;
      Serial.print(" [HIGH HUMIDITY ALERT!]");
      
      // Send high humidity alert
      if (!prevHumidityAlertState) {
        sendHumidityAlert(humidity, true);
        humidityAlertSent = true;
      }
    } else if (humidity < DRY_HUM) {
      warning = true;
      currentHumidityAlertState = true;
      Serial.print(" [LOW HUMIDITY ALERT!]");
      
      // Send low humidity alert
      if (!prevHumidityAlertState) {
        sendHumidityAlert(humidity, false);
        humidityAlertSent = true;
      }
    } else {
      currentHumidityAlertState = false;
      Serial.println(" [NORMAL]");
      
      // Send humidity normal notification
      if (prevHumidityAlertState) {
        sendHumidityNormal(humidity);
        humidityAlertSent = false;
      }
    }
  } else {
    Serial.println("ERROR reading humidity");
    currentHumidityAlertState = false;
  }
  
  prevHumidityAlertState = currentHumidityAlertState;

  // ================= MPU6050 SENSOR =================
  digitalWrite(EARTHQUAKE_LED, LOW);
  
  float currentVibration = 0.0;
  bool currentEarthquakeState = false;
  
  if (mpuConnected) {
    sensors_event_t a, g, t;
    
    if (mpu.getEvent(&a, &g, &t)) {
      currentVibration = abs(a.acceleration.x) + abs(a.acceleration.y) + abs(a.acceleration.z);
      
      Serial.print("EARTHQUAKE: ");
      Serial.print("Vibration: ");
      Serial.print(currentVibration, 2);
      Serial.print(" m/s² | Threshold: ");
      Serial.print(VIBRATION_THRESHOLD);
      Serial.print(" m/s²");
      
      if (currentVibration > VIBRATION_THRESHOLD) {
        emergency = true;
        digitalWrite(EARTHQUAKE_LED, HIGH);
        currentEarthquakeState = true;
        Serial.print(" [EARTHQUAKE DETECTED!]");
        
        // Send earthquake alert notification
        if (!prevEarthquakeState) {
          sendEarthquakeAlert(currentVibration);
          earthquakeAlertSent = true;
        }
      } else {
        currentEarthquakeState = false;
        Serial.println(" [NORMAL]");
        
        // Send earthquake clear notification
        if (prevEarthquakeState) {
          sendEarthquakeClear();
          earthquakeAlertSent = false;
        }
      }
    } else {
      Serial.println("EARTHQUAKE: ERROR reading MPU6050");
      currentEarthquakeState = false;
    }
  } else {
    Serial.println("EARTHQUAKE: MPU6050 not connected");
    currentEarthquakeState = false;
  }
  
  prevEarthquakeState = currentEarthquakeState;

  // ================= LED STATUS =================
  Serial.print("LED STATUS: ");
  Serial.print("Flood(D32)=");
  Serial.print(digitalRead(LED1));
  Serial.print(" Gas(D33)=");
  Serial.print(digitalRead(LED2));
  Serial.print(" Temp(D26)=");
  Serial.print(digitalRead(TEMP_LED));
  Serial.print(" Earthquake(D25)=");
  Serial.println(digitalRead(EARTHQUAKE_LED));

  // ================= SYSTEM STATUS =================
  Serial.print("SYSTEM STATUS: ");
  if (emergency) {
    Serial.println("🚨 EMERGENCY - EARTHQUAKE DETECTED");
  } else if (warning) {
    Serial.println("⚠️  WARNING - HAZARD DETECTED");
  } else {
    Serial.println("✅ ALL SYSTEMS NORMAL");
  }

  // ================= SEND DATA TO BLYNK =================
  Serial.println("SENDING DATA TO BLYNK:");
  
  Blynk.virtualWrite(V0, digitalRead(LED1));
  Serial.print("  V0 (Flood LED): ");
  Serial.println(digitalRead(LED1));
  
  Blynk.virtualWrite(V1, digitalRead(LED2));
  Serial.print("  V1 (Gas LED): ");
  Serial.println(digitalRead(LED2));
  
  Blynk.virtualWrite(V2, digitalRead(TEMP_LED));
  Serial.print("  V2 (Temperature LED): ");
  Serial.println(digitalRead(TEMP_LED));
  
  Blynk.virtualWrite(V3, temperature);
  Serial.print("  V3 (Temperature): ");
  if (!isnan(temperature)) {
    Serial.print(temperature, 1);
    Serial.println("°C");
  } else {
    Serial.println("ERROR");
  }
  
  Blynk.virtualWrite(V4, humidity);
  Serial.print("  V4 (Humidity): ");
  if (!isnan(humidity)) {
    Serial.print(humidity, 1);
    Serial.println("%");
  } else {
    Serial.println("ERROR");
  }
  
  if (mpuConnected) {
    Blynk.virtualWrite(V5, digitalRead(EARTHQUAKE_LED));
    Serial.print("  V5 (Earthquake LED): ");
    Serial.println(digitalRead(EARTHQUAKE_LED));
  } else {
    Blynk.virtualWrite(V5, 0);
    Serial.print("  V5 (Earthquake LED): ");
    Serial.println("0 (MPU6050 not connected)");
  }
  
  Blynk.virtualWrite(V6, distance);
  Serial.print("  V6 (Flood Value): ");
  Serial.print(distance, 1);
  Serial.println(" cm");
  
  Blynk.virtualWrite(V7, gasValue);
  Serial.print("  V7 (Gas Value): ");
  Serial.print(gasValue);
  Serial.print("/");
  Serial.println(GAS_THRESHOLD);
  
  Serial.println("✅ ALL DATA SENT TO BLYNK");

  // ================= BUZZER CONTROL =================
  Serial.print("BUZZER: ");
  
  forceBuzzerOff();
  
  if (emergency) {
    Serial.println("🚨 EMERGENCY PATTERN");
    emergencyBeep();
  }
  else if (warning) {
    Serial.println("⚠️  WARNING PATTERN");
    warningBeep();
  }
  else {
    Serial.println("🔇 SILENT");
    forceBuzzerOff();
  }
  
  forceBuzzerOff();

  Serial.println("=========================================");
  Serial.println("⏱️  Next reading in 2 seconds...");
  Serial.println("=========================================");
  
  forceBuzzerOff();
  delay(2000);
}