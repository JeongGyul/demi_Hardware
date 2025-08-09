#include <Servo.h>

// 초음파 센서 핀 번호
const int echoPins[3] = {2, 4, 6};
const int trigPins[3] = {3, 5, 7};

// 초음파 센서 거리 측정에 사용되는 변수 선언
unsigned long lastCycleTime = 0;
const unsigned long measurementCycleInterval = 5000;   // 전체 측정 주기 
const unsigned long sensorInterval = 200;              // 센서 간 시간차 

int currentSensor = 0;
bool measuring = false;
unsigned long lastSensorMeasureTime = 0;

long fillPercent[3] = {0};

// 레일 조작을 위해 사용하는 핀 번호 선언
const int PUL = 11;  // Pulse pin
const int DIR = 10;  // Direction pin
const int ENA = 9;  // Enable pin

// 서보 모터 조작을 위해 사용하는 핀 번호 선언
const int motor1 = 12;
const int motor2 = 13;
Servo servo1;
Servo servo2;

void setup() {
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);  // 모터 활성화 (LOW가 Enable임)

  for (int i = 0; i < 3; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  servo1.attach(motor1);
  servo2.attach(motor2);
  
  Serial.begin(9600);

  railInit();  // 레일의 위치를 중앙으로 초기화 시키는 사용자 정의 함수
}

void loop() {
  unsigned long now = millis();

  // 전체 측정 주기마다 거리 측정
  if (!measuring && now - lastCycleTime >= measurementCycleInterval) {
    measuring = true;
    currentSensor = 0;
    lastSensorMeasureTime = now;
    lastCycleTime = now;
  }

  // 측정 중이면 센서를 순차적으로 측정
  if (measuring && now - lastSensorMeasureTime >= sensorInterval) {
    fillPercent[currentSensor] = getFillLevel(trigPins[currentSensor], echoPins[currentSensor]);
    currentSensor++;
    lastSensorMeasureTime = now;

    if (currentSensor >= 3) {
      measuring = false;

      // 모든 센서 측정 완료 후 센서값 라즈베리파이로 전송
      Serial.print(fillPercent[0]);
      Serial.print(",");
      Serial.print(fillPercent[1]);
      Serial.print(",");
      Serial.print(fillPercent[2]);
      Serial.print("\n");
    }
  }
 
  // 라즈베리파이로부터 수신된 값이 있으면 아래 조건문 실행
  if (Serial.available() > 0) {
    char material = Serial.read();

    // 라즈베리파이가 탐지한 객체가 plastic이면 수행
    // 레일을 왼쪽으로 이동시킨 후 3초간 대기(쓰레기 투하)했다가 중앙으로 이동
    if (material == 'P') {
       for (int i = 0; i < 1600; i++) {
          digitalWrite(DIR, LOW);
          digitalWrite(PUL, HIGH);
          delayMicroseconds(300);
          digitalWrite(PUL, LOW);
          delayMicroseconds(300);
      }

      // 쓰레기 투하를 위한 서보모터 제어
      delay(1000);

      servo1.write(180);
      servo2.write(180);
     
      delay(1500);

      servo1.write(0);
      servo2.write(0);

      delay(1500);

      for (int i = 0; i < 1600; i++) {
          digitalWrite(DIR, HIGH);
          digitalWrite(PUL, HIGH);
          delayMicroseconds(300);
          digitalWrite(PUL, LOW);
          delayMicroseconds(300);
      }
    }

    // 라즈베리파이가 탐지한 객체가 can이면 수행
    // 레일을 오른쪽으로 이동시킨 후 3초간 대기(쓰레기 투하)했다가 중앙으로 이동
    else if (material == 'C') {
       for (int i = 0; i < 1600; i++) {
          digitalWrite(DIR, HIGH);
          digitalWrite(PUL, HIGH);
          delayMicroseconds(300);
          digitalWrite(PUL, LOW);
          delayMicroseconds(300);
      }
     
      // 쓰레기 투하를 위한 서보모터 제어
      delay(1000);

      servo1.write(180);
      servo2.write(180);
     
      delay(1500);

      servo1.write(0);
      servo2.write(0);

      delay(1500);

      for (int i = 0; i < 1600; i++) {
          digitalWrite(DIR, LOW);
          digitalWrite(PUL, HIGH);
          delayMicroseconds(300);
          digitalWrite(PUL, LOW);
          delayMicroseconds(300);
      }
    }

    // 라즈베리파이가 탐지한 객체가 glass이면 수행
    // 쓰레기통의 위치가 중앙이므로 레일 조작 불필요
    else if (material == 'G') {
      // 쓰레기 투하를 위한 서보모터 제어
      servo1.write(180);
      servo2.write(180);
     
      delay(1500);

      servo1.write(0);
      servo2.write(0);

      delay(1500);
    }
  }
}

// 레일의 위치를 중앙으로 초기화 시키는 사용자 정의 함수
void railInit() {
 
  // 레일의 위치를 오른쪽 끝으로 이동시킴
  for (int i = 0; i < 6400; i++) {
    digitalWrite(DIR, HIGH);
    digitalWrite(PUL, HIGH);
    delayMicroseconds(300);
    digitalWrite(PUL, LOW);
    delayMicroseconds(300);
  }

  // 왼쪽으로 전체 길이의 절반만 이동하여 레일이 중앙에 위치하도록 함
  for (int i = 0; i < 3200; i++) {
    digitalWrite(DIR, LOW);
    digitalWrite(PUL, HIGH);
    delayMicroseconds(300);
    digitalWrite(PUL, LOW);
    delayMicroseconds(300);
  }
}

// 초음파 센서를 이용해 쓰레기통의 포화도를 반환하는 사용자 정의 함수
long getFillLevel(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 최대 대기 30ms (30cm 이상)
  long distance = duration / 29 / 2; // cm로 변환

  int min_d = 28; // 쓰레기통이 가득 찼을 때 거리 값
  int max_d = 57; // 쓰레기통이 비어있을 때 거리 값
  
  // 측정된 거리 값을 기반으로 포화도 반환
  if (distance <= min_d) {
    return 100;
  }
  else if (distance >= max_d) {
    return 0;
  }
  else {
    return (int)round((float)(max_d - distance) / (max_d - min_d) * 100);
  }
}
