// 초음파 센서 핀 번호
const int echoPins[3] = {2, 4, 6};
const int trigPins[3] = {3, 5, 7};

// 초음파 센서 거리 측정에 사용되는 변수 선언
unsigned long lastCycleTime = 0;
const unsigned long measurementCycleInterval = 5000;   // 전체 측정 주기 
const unsigned long sensorInterval = 200;              // 센서 간 시간차 

// 레일 조작을 위해 사용하는 핀 번호 선언
const int DIR = 10;  // Direction pin
const int ENA = 9;  // Enable pin
const int PUL = 8;  // Pulse pin

// DC 모터 제어를 위한 핀 번호 선언
const int motor1EnablePin = 11;
const int motor1_1 = 12;
const int motor1_2 = 13;

// 초음파 센서 측정을 위한 변수 선언
int currentSensor = 0;
bool measuring = false;
unsigned long lastSensorMeasureTime = 0;
long fillPercent[3] = {0};

void setup() {
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);  // 레일 모터 활성화 (LOW가 Enable임)

  pinMode(motor1_1, OUTPUT);
  pinMode(motor1_2, OUTPUT);
  pinMode(motor1EnablePin, OUTPUT);
  analogWrite(motor1EnablePin, 0); // DC 모터 초기 설정

  for (int i = 0; i < 3; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
  
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

      dropTrash();
      clearSerialBuffer();

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
     
      dropTrash();
      clearSerialBuffer();

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
      dropTrash();
      clearSerialBuffer();
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

// 쓰레기 투하를 위해 DC모터를 제어하는 사용자 정의 함수
void dropTrash() {
    digitalWrite(motor1_1, HIGH);
    digitalWrite(motor1_2, LOW); 
    
    analogWrite(motor1EnablePin, 190);
    delay(2500);

    analogWrite(motor1EnablePin, 0);
    delay(1000);

    digitalWrite(motor1_1, LOW);
    digitalWrite(motor1_2, HIGH);

    analogWrite(motor1EnablePin, 190);  
    delay(2500);

    analogWrite(motor1EnablePin, 0);
    delay(1000);
}

// dropTrash() 끝나고 버퍼에 쌓인 시리얼 값 제거
void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
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

  int min_d = 38; // 쓰레기통이 가득 찼을 때 거리 값
  int max_d = 67; // 쓰레기통이 비어있을 때 거리 값
  
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
