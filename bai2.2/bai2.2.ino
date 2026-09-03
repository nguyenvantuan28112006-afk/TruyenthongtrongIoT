#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define FORCE_SET_TIME 0
#define PIN_LED  8
#define PIN_BUZZ 9

LiquidCrystal_I2C lcd(0x20, 16, 2);
RTC_DS1307 rtc;

// D4: MODE, D5: UP, D6: DOWN, D7: SET
volatile byte btnEvt = 0; // bit 0..3: co su kien release, bit 7: MODE giu 2s
volatile bool ringing = false;
volatile unsigned long tPress[4] = {0};
volatile byte lastPins = 0xF0;

byte mode = 0, step = 0, alarmH = 6, alarmM = 30, tVal[3];
bool alarmOn = true;
DateTime now;
unsigned long tRead = 0, tDraw = 0, tRing = 0;
int lastKey = -1;
const char *LBL[] = {"Gio ", "Phut", "Giay"};

// Ngat Pin Change tren PORTD (D4 - D7)
ISR(PCINT2_vect) {
  byte curr = PIND & 0xF0; // Doc D4..D7
  byte changed = curr ^ lastPins;
  unsigned long ms = millis();

  for (byte i = 0; i < 4; i++) {
    byte mask = 1 << (i + 4);
    if (changed & mask) {
      if (curr & mask) { // Vua nha nut (LOW -> HIGH)
        if (ms - tPress[i] > 30) {
          if (i == 0 && (ms - tPress[0] >= 2000)) btnEvt |= 0x80; // Giu 2s
          else btnEvt |= (1 << i);                                 // Nhan nha
        }
      } else {           // Vua nhan xuong (HIGH -> LOW)
        tPress[i] = ms;
      }
    }
  }
  lastPins = curr;
}

void saveAlarm() {
  EEPROM.update(0, 0x5A); EEPROM.update(1, alarmH);
  EEPROM.update(2, alarmM); EEPROM.update(3, alarmOn);
}

void printLine(byte row, const char *fmt, ...) {
  char buf[17]; va_list args;
  va_start(args, fmt); vsnprintf(buf, sizeof(buf), fmt, args); va_end(args);
  lcd.setCursor(0, row); lcd.print(buf);
  for (byte i = strlen(buf); i < 16; i++) lcd.print(' ');
}

void stopAlarm() {
  ringing = false; noTone(PIN_BUZZ);
  digitalWrite(PIN_BUZZ, HIGH); digitalWrite(PIN_LED, HIGH);
}

void setup() {
  DDRD &= ~0xF0;  // D4..D7 INPUT
  PORTD |= 0xF0;  // Bat PULLUP D4..D7
  PCICR |= (1 << PCIE2);   // Bat ngat PCINT nhom 2 (Port D)
  PCMSK2 |= 0xF0;          // Kich hoat ngat cho D4, D5, D6, D7

  pinMode(PIN_LED, OUTPUT);  digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_BUZZ, OUTPUT); digitalWrite(PIN_BUZZ, HIGH);

  lcd.init(); lcd.backlight();
  rtc.begin();
  now = rtc.now();
  if (FORCE_SET_TIME || !rtc.isrunning() || now.year() < 2020) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    now = rtc.now();
  }

  if (EEPROM.read(0) == 0x5A) {
    alarmH = constrain(EEPROM.read(1), 0, 23);
    alarmM = constrain(EEPROM.read(2), 0, 59);
    alarmOn = EEPROM.read(3);
  }
}

void loop() {
  // 1. Doc RTC moi 500ms
  if (millis() - tRead >= 500) {
    tRead = millis();
    DateTime t = rtc.now();
    if (t.year() >= 2020) now = t;
  }

  // 2. Kich hoat chuong
  int key = now.hour() * 60 + now.minute();
  if (key != lastKey) {
    lastKey = key;
    if (alarmOn && !ringing && now.hour() == alarmH && now.minute() == alarmM) {
      ringing = true; tRing = millis();
      tone(PIN_BUZZ, 2500); digitalWrite(PIN_LED, LOW);
    }
  }

  // 3. Lay su kien nut tu ISR
  byte evt = 0;
  if (btnEvt) {
    noInterrupts(); evt = btnEvt; btnEvt = 0; interrupts();
  }

  // Xu ly chuong
  if (ringing) {
    if (evt || millis() - tRing >= 30000) stopAlarm();
    return;
  }

  // 4. Xu ly nut MODE
  if (evt & 0x80) { // Giu 2 giay
    alarmOn = !alarmOn; saveAlarm(); mode = 0;
  } else if (evt & 0x01) { // Nhan nha
    mode = (mode + 1) % 3; step = 0;
    tVal[0] = (mode == 1) ? alarmH : now.hour();
    tVal[1] = (mode == 1) ? alarmM : now.minute();
    tVal[2] = now.second();
  }

  // 5. Xu ly UP(bit 1), DOWN(bit 2), SET(bit 3)
  if (mode > 0) {
    int d = (evt & 0x02) ? 1 : ((evt & 0x04) ? -1 : 0);
    byte limit = (step == 0) ? 24 : 60;
    if (d) tVal[step] = (tVal[step] + limit + d) % limit;

    if (evt & 0x08) {
      if (step < (mode == 1 ? 1 : 2)) step++;
      else {
        if (mode == 1) { alarmH = tVal[0]; alarmM = tVal[1]; saveAlarm(); }
        else rtc.adjust(DateTime(now.year(), now.month(), now.day(), tVal[0], tVal[1], tVal[2]));
        lastKey = -1; mode = 0;
      }
    }
  }

  // 6. Hien thi LCD (5Hz)
  if (millis() - tDraw >= 200) {
    tDraw = millis();
    if (mode == 0) {
      printLine(0, "%02d:%02d:%02d  %s", now.hour(), now.minute(), now.second(), alarmOn ? "AL ON" : "AL OFF");
      printLine(1, "%02d/%02d/%04d %02d:%02d", now.day(), now.month(), now.year(), alarmH, alarmM);
    } else if (mode == 1) {
      printLine(0, "CAI BAO THUC");
      printLine(1, "%s   %02d:%02d", LBL[step], tVal[0], tVal[1]);
    } else {
      printLine(0, "CAI THOI GIAN");
      printLine(1, "%s %02d:%02d:%02d", LBL[step], tVal[0], tVal[1], tVal[2]);
    }
  }
}