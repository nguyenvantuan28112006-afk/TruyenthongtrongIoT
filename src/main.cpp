#include <Arduino.h>
#include <SoftwareSerial.h>

// ===========================================
// CHAN
// ===========================================
#define UART_RX_PIN 10
#define UART_TX_PIN 11

#define LED_PIN 6

// D10 = RX
// D11 = TX
SoftwareSerial ESP32UART(UART_RX_PIN, UART_TX_PIN);

// ===========================================
// UART
// ===========================================
unsigned long currentBaud = 9600;

unsigned long pendingBaud = 0;
bool baudSwitchPending = false;

unsigned long baudSwitchTime = 0;

// ===========================================
// UART BUFFER
// ===========================================
String uartBuffer = "";

// ===========================================
// LED NON-BLOCKING
// ===========================================
bool ledPatternActive = false;
bool ledState = false;

uint8_t blinksRemaining = 0;

unsigned long ledPreviousTime = 0;

unsigned int ledOnTime = 200;
unsigned int ledOffTime = 200;

// ==================================================
// BAT DAU NHAY LED
// ==================================================
void startBlink(
    uint8_t count,
    unsigned int onTime,
    unsigned int offTime)
{
    // Ghi de pattern cu
    ledPatternActive = true;

    blinksRemaining = count;

    ledOnTime = onTime;
    ledOffTime = offTime;

    ledState = true;

    digitalWrite(LED_PIN, HIGH);

    ledPreviousTime = millis();
}

// ==================================================
// TAT LED
// ==================================================
void stopBlink()
{
    ledPatternActive = false;

    ledState = false;

    blinksRemaining = 0;

    digitalWrite(LED_PIN, LOW);
}

// ==================================================
// CAP NHAT LED KHONG DELAY
// ==================================================
void updateLED()
{
    if (!ledPatternActive)
        return;

    unsigned long now = millis();

    // LED dang BAT
    if (ledState)
    {
        if (now - ledPreviousTime >= ledOnTime)
        {
            digitalWrite(LED_PIN, LOW);

            ledState = false;

            ledPreviousTime = now;

            // Da hoan thanh 1 lan nhay
            if (blinksRemaining > 0)
            {
                blinksRemaining--;
            }

            // Het so lan nhay
            if (blinksRemaining == 0)
            {
                ledPatternActive = false;
            }
        }
    }

    // LED dang TAT
    else
    {
        if (now - ledPreviousTime >= ledOffTime)
        {
            digitalWrite(LED_PIN, HIGH);

            ledState = true;

            ledPreviousTime = now;
        }
    }
}

// ==================================================
// GUI PHAN HOI CHO ESP32
// ==================================================
void sendReply(const String &message)
{
    ESP32UART.println(message);

    Serial.print("[Arduino -> ESP32] ");
    Serial.println(message);
}

// ==================================================
// XU LY LENH NHAN TU ESP32
// ==================================================
void processCommand(String command)
{
    command.trim();

    if (command.length() == 0)
        return;

    Serial.print("[ESP32 -> Arduino] ");
    Serial.println(command);

    // ========================================
    // VAO CONFIG
    // ========================================
    if (command == "CFG_ON")
    {
        Serial.println("VAO CHE DO CONFIG");

        // Nhay nhanh 5 lan
        startBlink(
            5,
            100,
            100);

        sendReply("ACK_CFG_ON");

        return;
    }

    // ========================================
    // THOAT CONFIG
    // ========================================
    if (command == "CFG_OFF")
    {
        Serial.println("THOAT CHE DO CONFIG");

        stopBlink();

        sendReply("ACK_CFG_OFF");

        return;
    }

    // ========================================
    // DOI BAUD
    // ========================================
    if (command.startsWith("SET_BAUD:"))
    {
        String baudString = command.substring(9);

        unsigned long newBaud = baudString.toInt();

        // ----------------------------
        // 9600
        // ----------------------------
        if (newBaud == 9600)
        {
            Serial.println();
            Serial.println("Chon UART = 9600 bps");

            // LED nhay 1 lan
            startBlink(
                1,
                300,
                300);
        }

        // ----------------------------
        // 115200
        // ----------------------------
        else if (newBaud == 115200)
        {
            Serial.println();
            Serial.println("Chon UART = 115200 bps");

            // LED nhay 2 lan
            startBlink(
                2,
                300,
                300);
        }

        else
        {
            Serial.println("Baud khong hop le!");

            sendReply("ERROR_BAUD");

            return;
        }

        // -----------------------------------
        // ACK bang BAUD CU
        // -----------------------------------
        String reply = "ACK_BAUD:";
        reply += String(newBaud);

        sendReply(reply);

        // Cho mot chut de ACK duoc gui xong
        // NHUNG KHONG DUNG delay()
        pendingBaud = newBaud;

        baudSwitchPending = true;

        baudSwitchTime = millis() + 200;

        return;
    }
}

// ==================================================
// DOC UART
// ==================================================
void updateUART()
{
    while (ESP32UART.available())
    {
        char c = ESP32UART.read();

        if (c == '\n')
        {
            processCommand(uartBuffer);

            uartBuffer = "";
        }
        else if (c != '\r')
        {
            uartBuffer += c;

            // Chong tran buffer
            if (uartBuffer.length() > 100)
            {
                uartBuffer = "";
            }
        }
    }
}
// ==================================================
// DOI BAUD UART
// ==================================================
void updateBaud()
{
    if (!baudSwitchPending)
        return;

    if ((long)(millis() - baudSwitchTime) >= 0)
    {
        baudSwitchPending = false;

        ESP32UART.flush();

        ESP32UART.end();

        currentBaud = pendingBaud;

        ESP32UART.begin(currentBaud);

        pendingBaud = 0;

        Serial.println();
        Serial.println("===============================");
        Serial.print("UART ARDUINO DA DOI THANH: ");
        Serial.print(currentBaud);
        Serial.println(" bps");
        Serial.println("===============================");
    }
}

// ==================================================
// SETUP
// ==================================================
void setup()
{
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    // Serial Monitor Arduino
    Serial.begin(115200);

    // UART Arduino <-> ESP32
    ESP32UART.begin(currentBaud);

    Serial.println();
    Serial.println("===============================");
    Serial.println("ARDUINO UNO - BAI 1.2");
    Serial.println("===============================");

    Serial.println("D10 = RX");
    Serial.println("D11 = TX");
    Serial.println("D6  = LED");

    Serial.print("UART ban dau: ");
    Serial.print(currentBaud);
    Serial.println(" bps");
}

// ==================================================
// LOOP
// ==================================================
void loop()
{
    updateUART();

    updateLED();

    updateBaud();

    // KHONG CO delay()
}