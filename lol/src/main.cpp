#include <M5StickCPlus.h>
#include <BluetoothSerial.h>

// 1 ------ 2
//   |    |
//   |    |
//   | M5 |
// 3 ------ 4

#define ROVERC_I2C_Address 0x38

int i = 0;

BluetoothSerial SerialBT;

int8_t motor_speeds[4] = {0,0,0,0};

int8_t speed_ramp(int8_t actual_speed, int8_t target_speed){
  // acceleration not larger than 5 can be applied directly
  // so init calc_speed with target_speed and check if the 
  // acceleration is higher than 5
  int8_t calc_speed = target_speed;
  // acceleration larger than 5?
  if(abs(target_speed - actual_speed) > 5){
    if( target_speed > actual_speed) 
      calc_speed = actual_speed + 5;
    else
      calc_speed = actual_speed - 5;
  }
  // ensure that the result is between -100 and +100
  calc_speed = (calc_speed > 100) ? 100 : calc_speed;
  calc_speed = (calc_speed < -100) ? -100 : calc_speed;
  return calc_speed;
}

void rover_set_motors(int8_t M1, int8_t M2, int8_t M3, int8_t M4) {
  motor_speeds[0] = speed_ramp(motor_speeds[0],M1); 
  motor_speeds[1] = speed_ramp(motor_speeds[1],M2);
  motor_speeds[2] = speed_ramp(motor_speeds[2],M3);
  motor_speeds[3] = speed_ramp(motor_speeds[3],M4);
  //Serial.print(motor_speeds[0]);Serial.print(" ");Serial.print(motor_speeds[1]);Serial.print(" ");
  //Serial.print(motor_speeds[2]);Serial.print(" ");Serial.println(motor_speeds[3]);
  // send the Motor speed to the Rover board via I2C
  Wire.beginTransmission(ROVERC_I2C_Address);
  short address = 0x00;
  Wire.write(address);
  Wire.write(motor_speeds[0]);
  Wire.write(motor_speeds[1]);
  Wire.write(motor_speeds[2]);
  Wire.write(motor_speeds[3]);
  Wire.endTransmission();
}

void rover_stop(){
  while(motor_speeds[0] != 0 || motor_speeds[1] != 0 || motor_speeds[2] != 0 || motor_speeds[3] != 0) {
    rover_set_motors(0, 0, 0, 0);
    delay(20);
  }
}

void move_rover(double angle, int speed) {
  // ensure that speed is between -100 and 100
  speed = (speed > 100) ? 100 : speed;
  speed = (speed < -100) ? -100 : speed;
  double vx = sin((angle * PI) / 180.0) * speed;
  double vy = cos((angle * PI) / 180.0) * speed;
  // calculate the target speed for each motor
  int8_t M1 = (int8_t) round(vy + vx);
  int8_t M2 = (int8_t) round(vy - vx);
  int8_t M3 = (int8_t) round(vy - vx);
  int8_t M4 = (int8_t) round(vy + vx);
  // repeat until desired speed is reached
  while(motor_speeds[0] != M1 || motor_speeds[1] != M2 || motor_speeds[2] != M3 || motor_speeds[3] != M4) {
    rover_set_motors(M1, M2, M3, M4);
    delay(20);
  }
}

void setup(){
  Serial.begin(115200);
  SerialBT.begin("tesla model 4");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(20,20);
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(ORANGE); 
  M5.Lcd.print("jedu!");
  M5.begin();
  Wire.begin(0, 26);

}

void loop(){
  M5.update();
  uint16_t vbatData = M5.Axp.GetVbatData();
  double vbat = vbatData * 1.1 / 1000;
  Serial.println(vbat);

if (SerialBT.available()) {
     char ch=SerialBT.read();
     if(ch=='j' || ch=='J')
        rover_set_motors(72,50,50,50);
     if(ch=='s' || ch=='S')
        rover_set_motors(0,0,0,0);   
  }
  delay(1000);
 
}