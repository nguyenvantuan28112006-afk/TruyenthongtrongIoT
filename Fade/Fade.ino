/*
  Fade

  This example shows how to fade an LED on pin 9 using the analogWrite()
  function.

  The analogWrite() function uses PWM, so if you want to change the pin you're
  using, be sure to use another PWM capable pin. On most Arduino, the PWM pins
  are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Fade/
*/

int led = 9;
int ledd = 10;         // the PWM pin the LED is attached to , Khai báo chân sử dụng trên arduino
int mucdoden = 0;  // how bright the LED is , khai báo mức độ đèn đầu tiên(tắt)
int fadeAmount = 3;  // how many points to fade the LED by, Khai báo bước mỗi lần tăng 

// the setup routine runs once when you press reset:
void setup() {    //khai báo hàm
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT); //khai báo chân led và là chân xuất tín hiệu 
  pinMode(ledd, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // set the brightness of pin 9:// Đặt độ sáng của chân số 9:
  analogWrite(led, mucdoden);   //Xuất tín hiệu từ chân 9 và mức độ đèn
  analogWrite(ledd, mucdoden); 

  // change the brightness for next time through the loop:
  mucdoden = mucdoden + fadeAmount;   //Thay đổi độ sáng của led

  // reverse the direction of the fading at the ends of the fade:
  // Kiểm tra: Nếu độ sáng giảm xuống dưới 0 hoặc vượt quá 255 (mức tối đa)
  if (mucdoden <= 0 || mucdoden >= 255) {
    fadeAmount = -fadeAmount;
    // Đảo ngược chiều tăng/giảm.
   // Nếu đang tăng (+3) sẽ thành giảm (-3) và ngược lại.
  }
  // wait for 50 milliseconds to see the dimming effect
  delay(50);
}
