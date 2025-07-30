#include <Servo.h>

// 초음파 센서 핀 번호
int echo1 = 1;
int trig1 = 2;
int echo2 = 3;
int trig2 = 4;
int echo3 = 5;
int trig3 = 6;

// 레일 조작을 위해 사용하는 핀 번호 선언
int PUL = 9;  // Pulse pin
int DIR = 10;  // Direction pin
int ENA = 11;  // Enable pin

// 서보 모터 조작을 위해 사용하는 핀 번호 선언
int motor1 = 12;
int motor2 = 13;
Servo servo1;
Servo servo2;

void setup() {
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);  // 모터 활성화 (LOW가 Enable임)

  servo1.attach(motor1);
  servo2.attach(motor2);
  
  Serial.begin(9600);

  moveTo();  // 레일의 위치를 중앙으로 초기화 시키는 사용자 정의 함수
}

void loop() {
 
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
void moveTo() {
 
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
