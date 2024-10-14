#include <Servo.h>

#define PIN_LED   9   
#define PIN_TRIG  12  
#define PIN_ECHO  13  
#define PIN_SERVO 10  

#define SND_VEL 346.0     
#define INTERVAL 25      
#define PULSE_DURATION 10 
#define DIST_MIN 180.0   
#define DIST_MAX 360.0   

#define TIMEOUT ((INTERVAL / 2) * 1000.0) 
#define SCALE (0.001 * 0.5 * SND_VEL) 

#define EMA_ALPHA 0.30    

#define TARGET_LOW  180.0
#define TARGET_HIGH 360.0

#define DUTY_MIN 550 
#define DUTY_NEU ((DUTY_MIN + DUTY_MAX) / 2)
#define DUTY_MAX 2450 

float dist_ema, dist_prev = DIST_MAX; 
unsigned long last_sampling_time;       

Servo myservo;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);    
  pinMode(PIN_ECHO, INPUT);     
  digitalWrite(PIN_TRIG, LOW);  

  myservo.attach(PIN_SERVO); 
  myservo.writeMicroseconds(DUTY_NEU);

  dist_prev = DIST_MIN; 

  Serial.begin(57600);
}

void loop() {
  float dist_raw;
  
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  dist_raw = measureDistance(PIN_TRIG, PIN_ECHO); 

  if (dist_raw == 0.0 || dist_raw > DIST_MAX) {
    dist_raw = dist_prev;           
    digitalWrite(PIN_LED, HIGH);    
  } else if (dist_raw < DIST_MIN) {
    dist_raw = dist_prev;           
    digitalWrite(PIN_LED, HIGH);    
  } else {
    dist_prev = dist_raw;
    digitalWrite(PIN_LED, LOW);     
  }

  dist_ema = (EMA_ALPHA * dist_raw) + ((1 - EMA_ALPHA) * dist_ema);

  if (dist_raw <= 180) {
    myservo.writeMicroseconds(DUTY_MIN); 
  } else if (dist_raw > 180 && dist_raw < 220) {
    myservo.writeMicroseconds(DUTY_NEU); 
  } else {
    myservo.writeMicroseconds(DUTY_MAX); 
  }

  float duty_analog = (((dist_ema - 180) / 180) * 1900) + 550;
  if (dist_raw == 0.0 || dist_raw < TARGET_LOW) {
    dist_raw = dist_prev;           
    myservo.writeMicroseconds(DUTY_MIN);       
  } else if (dist_raw > TARGET_HIGH) {
    dist_raw = dist_prev;           
    myservo.writeMicroseconds(DUTY_MAX);       
  } else {
    dist_prev = dist_raw;
    myservo.writeMicroseconds(duty_analog);       
  }

  Serial.print("Min:");    Serial.print(DIST_MIN);
  Serial.print(",dist:");  Serial.print(dist_raw);
  Serial.print(",EMA:");   Serial.print(dist_ema);
  Serial.print(",Servo:"); Serial.print(myservo.read());
  Serial.print(",Max:");   Serial.print(DIST_MAX);
  Serial.println("");      

  last_sampling_time += INTERVAL;
}

float measureDistance(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; 
}
