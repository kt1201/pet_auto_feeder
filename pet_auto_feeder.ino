#include<Wire.h>
#include<LiquidCrystal_I2C.h>
#include<core_build_options.h>
#include<swRTC.h>
#include<pitches.h>

LiquidCrystal_I2C lcd(0x27,16,2);
String lcdString = "";  //객체 선언 : 출력 할 글자 저장

swRTC rtc; 

int RELAY = 2;
int BUTTON = 3;
int piezo = 6;
int lastbuttonState = LOW;
int temp;

//AM PM을 구분해 주는 함수
void Set_AMPM(int hour) {
  if(hour >=12) 
    lcd.print("PM");
  else 
    lcd.print("AM");

  lcd.print(hour%12, DEC);     //시간 출력
}

//10보다 작은수를 출력할때 앞에 0을 출력하게 하는 함수
void Set_lowThanTen(int time) {
  if(time < 10) {
    lcd.print("0");
    lcd.print(time%10);
  }
  else
    lcd.print(time);
}

//유효한 알람시간인지 체크하는 함수
int checkTheAlarmClock(int time) {
  if(time/100 < 24 && time %100 < 60) {
    Serial.println("Success");
    return time;
  }
  else {
    Serial.println("Failed");
    return 0;
  }  
}

//알람이 울릴시간인지 체크하는 함수
void checkTheAlarmTime(int alarmHour, int alarmMinute) {
  if(alarmHour == rtc.getHours() && alarmMinute == rtc.getMinutes() && rtc.getSeconds() < 1) {
      digitalWrite(RELAY, HIGH);
      delay(500);  //  모터의 회전속도 조절
      digitalWrite(RELAY, LOW);
      analogWrite(piezo, 128);
      Serial.println("Feed complete");
  }
  else {
    noTone(6);
  }
}

//버튼을 눌러 사료급여를 하는 함수
void buttonpushfeeding() {
  int buttonState = digitalRead(BUTTON);  //  버튼의 상태값

  if(buttonState == HIGH && lastbuttonState == LOW){  //  버튼이 띄어져있다가 눌렸다
    digitalWrite(RELAY, HIGH);
    delay(500);  //  모터의 회전속도 조절
    digitalWrite(RELAY, LOW);
    analogWrite(piezo, 128);
    Serial.println("Feed complete");
  }
  lastbuttonState = buttonState;
}

void setup() {                   
  lcd.init();             //화면 초기화
  lcd.backlight();
  
  rtc.stopRTC();           //정지
  rtc.setTime(14,05,50);    //시간, 분, 초 초기화
  rtc.setDate(24, 8, 2014);  //일, 월, 년 초기화 
  rtc.startRTC();          //시작
  
  pinMode(piezo, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUTTON, INPUT);
  digitalWrite(RELAY, LOW);
  Serial.begin(9600);      //시리얼 포트 초기화 
}

void loop() {
  int day;
  lcd.setCursor(0,0);                    //커서를 0,0에 지정 
  
  //1초 단위로 갱신하며 현재시간을 LCD에 출력
  Set_AMPM(rtc.getHours()); 
  lcd.print(":");
  Set_lowThanTen(rtc.getMinutes());
  lcd.print(":");
  Set_lowThanTen(rtc.getSeconds());
  //날짜를 LCD에 출력
  lcd.print("[");
  Set_lowThanTen(rtc.getMonth());
  lcd.print("/");
  Set_lowThanTen(rtc.getDay());
  lcd.print("]");
  //세팅된 알람시간을 LCD에 출력
  lcd.setCursor(0,1);
  lcd.print("FeedTime ");
  Set_AMPM(temp/100);
  lcd.print(":");
  Set_lowThanTen(temp%100); 
  
  //1초마다 LCD갱신
  lcdString = "";                      //문자열 초기화
  lcd.print("               ");        //전 글씨 삭제
  delay(1000);
  
  //알람이 울릴 시간인지 체크
  checkTheAlarmTime(temp/100, temp%100);
   
  //시리얼 통신을 통해 알람시간을 입력받고 시리얼 모니터에 출력 
  char theDay[4];
  int i = 0;
  if(Serial.available()) {
    while(Serial.available()) {
      theDay[i] = Serial.read();
      i++;
  }
    day = atoi(theDay);
    if(day/100 >= 12) {
      Serial.print("PM");
      Serial.print((day/100)-12);
    }
    else {
      Serial.print("AM");
      Serial.print(day/100);
    }
    Serial.print(":");
    if(day%100 < 10)
      Serial.print("0");
    Serial.println(day%100);
    temp = checkTheAlarmClock(day);
  }
  buttonpushfeeding();
}
