/* 
*  - 통신 :  
아두이노 UNO - 블루투스 모듈(HC-06)  
         p2 - TXD  
         p3 - RXD  
         
- 조향 :  
아두이노 UNO - 모터 드라이버(DM322E)  
         p4 - enA 
         p5 - PUL(펄스)  
         p6 - DIR(디렉션)  
모터 드라이버 - 스텝모터(2번, 5번 미사용)  
         A+ - 모터 결선 좌측 1번째  
         A- - 모터 결선 좌측 3번째  
         B+ - 모터 결선 좌측 4번째  
         B- - 모터 결선 좌측 6번째  
모터 드라이버의 점퍼를 조정해 전류를 제어할 수 있음  
모터는 K6G50C 1:50 기어박스가 포함되어 있음  
현재 세팅은 점퍼 스위치 세팅 : 110010 (피크 전류 3.2A, RMS 3.0A, 200pulse/rev)  
- 페달모드 :
아두이노 - 페달
     A0 - 노랑
     7  - 스위치-전진
     8  - 스위치-후진
     9  - ground
- 구동 :  
아두이노 UNO - 모터 드라이버  
전진후진(P10) - dir입력(모터 회전 방향)  
PWM출력(p11) - PWM입력(파워)  
enB(p12) - 구동 여부 결정
- 5V 확인용 : p13
*/


#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

// 스텝 모터 제어
const int enA = 4;  // 구동 여부 결정
const int stepPin = 5; // 스텝 펄스
const int dirPinLR = 6;  // 좌우 회전

const int STEPS_PER_REV = 1600; // 모터 1회전 

int rotateLimit = 4;

// 드라이브 모터 제어
const int PWM = 10; // 파워
const int in1 = 11; // 신호 1 
const int in2 = 12; // 신호 2
const int vcc = 13; // 5V 확인용

const int valocity = 100;

// 페달 제어_전진, 후진 스위치 센싱
int pedalF = 7;
boolean pedalFVal = 0;

int pedalB = 8;
boolean pedalBVal = 0;

const int ground = 9;

// 페달 제어_페달 센싱
const int pedalSensor = A0;
int pedalVal = 0;

// 수동 모드, 앱 제어모드 변경, 0은 앱제어모드, 1은 수동 모드
boolean modeState = 1;

// 속도값 초기화
int i = 0;

// 입력 문자, 입력 문자 백업
char cmd = "";
char cmdM = "";

void setup() {
  //통신 설정
  Serial.begin(9600); // 시리얼 통신
  mySerial.begin(9600); // 블루투스 통신             

  // 스텝모터 핀 모드 설정
  pinMode(dirPinLR,OUTPUT);
  pinMode(stepPin,OUTPUT); 
  pinMode(enA, OUTPUT);
  digitalWrite(enA, HIGH);

  // 드라이브모터 핀 모드 설정
  pinMode(PWM,OUTPUT);
  pinMode(in1,OUTPUT); 
  pinMode(in2, OUTPUT);
  pinMode(vcc, OUTPUT);
  digitalWrite(vcc, HIGH);
         
  // 페달모드 전진 후진 신호
  pinMode(pedalF, INPUT_PULLUP);
  pinMode(pedalB, INPUT_PULLUP);
  pinMode(ground, OUTPUT);
  digitalWrite(ground, LOW);

  Serial.println("AI Go-Kart is Ready!");
}

void loop() {
  //  modestate가 1이면 페달제어 모드로 수행
  if(modeState == 1) {  
    Serial.print("Manual mode  ");  
    viewData();
  }
  if(modeState == 0) {  
    Serial.print("Autonomous mode  ");  
    viewData();
  }


         
  if (Serial.available() ){        // 블루투스 통신에 데이터가 있을 경우
    cmd = Serial.read();     // 블루투스의 데이터(문자 한 글자)를 'cmd' 변수에 저장
  
    // cmd 변수의 데이터가 m이면 수동모드로, i면 앱모드로 modeState 변수의 상태를 바꿈
    if (cmd == 'm') {
      modeState = 1;
      Serial.println("input 'm'");
      Serial.println("the mode is : manual control");
    }
    
    if (cmd == 'i') {
      modeState = 0;
      Serial.println("input 'i'");
      Serial.println("the mode is : app control");
    }    
 

    // 좌우 조향만 인공지능으로 할 때 주석해제, a는 좌회전, d는 우회전
    if ( cmd == 'a' ) {       // 아니고 만약 'cmd' 변수의 데이터가 a면
      right();
    } else if ( cmd == 'd' ) {       // 아니고 만약 'cmd' 변수의 데이터가 d면
      left();
    }

      

      // 기존 인공지능 모드를 사용할 때 주석 해제, a는 좌회전, d는 우회전, w는 전진, x는 후진, s는 차량정지
//      if (cmd == 'w' ){               // 만약 'cmd' 변수의 데이터가 w이면
//        Serial.println(cmdM);
//        if(cmdM == 'w'){
//          forward();
//        } else {
//          i = 0;
//          forward();
//        }
//        cmdM = 'w';
//      } else if ( cmd == 'x') {        // 아니고 만약 'cmd' 변수의 데이터가 x면
//        if(cmdM == 'x') {
//          backward();
//        } else {
//          i = 0;
//          backward();
//        }
//        cmdM = 'x';
//      } else if ( cmd == 'a' ) {       // 아니고 만약 'cmd' 변수의 데이터가 a면
//        right();
//      } else if ( cmd == 'd' ) {       // 아니고 만약 'cmd' 변수의 데이터가 d면
//        left();
//      } else if ( cmd == 's' ) {       // 아니고 만약 'cmd' 변수의 데이터가 s면
//        motorStop();
//      }
    }
  
}



void left() {
  // 조향 모터가 '반시계방향'으로 회전하도록 신호부여
  digitalWrite(dirPinLR,HIGH); 
  
  if (rotateLimit > 0) {
    // 1000마이크로초 주기로 모터 축이 1.5회전하는 코드
    // 1:50 기어박스 내장되어 있으므로, 모터 1회전에 바퀴 7.2도 회전함
    // 따라서, 모터가 1.5회전하면 바퀴가 10.8도 회전함
    for(int x = 0; x < STEPS_PER_REV*1.5; x++) {
      digitalWrite(enA,HIGH);
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(50);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(50); 
    }
    rotateLimit = rotateLimit - 1;
  } else {
    rotateLimit = 0;
  }
  Serial.println(rotateLimit);
}

void right() {
  // 조향 모터가 '시계방향'으로 회전하도록 신호부여
  digitalWrite(dirPinLR,LOW); 
  
  if (rotateLimit < 7) {
    for(int x = 0; x < STEPS_PER_REV*1.5; x++) {
      digitalWrite(enA,HIGH);
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(50);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(50); 
    }
    rotateLimit = rotateLimit + 1;
  } else {
    rotateLimit = 7;
  }
  Serial.println(rotateLimit);
}

void forward() {
  //드라이브 모터가 앞으로 회전하도록 신호부여
  digitalWrite(in1,HIGH); 
  digitalWrite(in2,LOW); 
  analogWrite(PWM, i);

  if(i != valocity) {
    for (i = 0; i < valocity; i = i + 10) {
      analogWrite(PWM, i);
      delay(100);
    }
  }
  Serial.println("forward");
}

void motorStop() {
  digitalWrite(in1,LOW); 
  digitalWrite(in2,LOW);
  
  analogWrite(PWM, 0);
  delay(100);
  Serial.println("motorStop");
}

void backward() {
  ////드라이브 모터가 뒤로 회전하도록 신호부여
  digitalWrite(in1,LOW); 
  digitalWrite(in2,HIGH); 
  analogWrite(PWM, i);
  
  if(i != valocity) {
    for (i = 0; i < valocity; i = i + 10) {
      analogWrite(PWM, i);
      delay(100);
    }
  }
  Serial.println("backward");
}

void viewData() {

  // 전진, 후진 스위치 값 저장
  pedalFVal = digitalRead(pedalF);
  pedalBVal = digitalRead(pedalB);

  // 페달값 센싱-매핑-한계범위설정
  pedalVal = analogRead(pedalSensor);
  pedalVal = map(pedalVal, 230, 850, 0, 255);
  pedalVal = constrain(pedalVal, 0, 255);

  // 페달 값 변화 시리얼 모니터링
  Serial.print(pedalFVal);
  Serial.print("  ");
  Serial.print(pedalBVal);
  Serial.print("  ");
  Serial.print(pedalVal);
  Serial.print("  ");

  // 페달 신호가 0이면 브레이킹
  if (pedalVal == 0) {
    digitalWrite(in1,LOW); 
    digitalWrite(in2,LOW);
    
    analogWrite(PWM, 0);
    delay(100);
  }

  // 전진, 후진 스위치 값에 따른 페달 동작
  if (pedalFVal == 1 && pedalBVal == 0) {
    digitalWrite(in1,HIGH); 
    digitalWrite(in2,LOW); 
    analogWrite(PWM, pedalVal);
    Serial.println("FFFF");
  } else if (pedalFVal == 0 && pedalBVal == 1) {
    digitalWrite(in1,LOW); 
    digitalWrite(in2,HIGH); 
    analogWrite(PWM, pedalVal);
    Serial.println("RRRR");
  } else {
    digitalWrite(in1,LOW); 
    digitalWrite(in2,LOW); 
    analogWrite(PWM, 0);
    Serial.println("SSSS");
  }
}
