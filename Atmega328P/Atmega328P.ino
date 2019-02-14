/*连线方法
  MPU-UNO
  VCC-5V
  GND-GND
  SCL-A5
  SDA-A4
  ADO-GND
  HC05-UNO
  VCC-5V
  GND-GND
  RX-D10
  TX-11*/
//未使用中断功能，即没有做 INT-digital pin 2 (interrupt pin 0) 这样的接线

//参考手册：MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2

#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial Serial1(10, 11); //TX10,RX11
#define DYNAMIC_PRECISION   0.25      /*动态精度*/
#define MOST_ACTIVE_NULL      0        /*未找到最活跃轴*/
#define MOST_ACTIVE_X         1    /*最活跃轴X*/
#define MOST_ACTIVE_Y         2        /*最活跃轴Y*/
#define MOST_ACTIVE_Z         3        /*最活跃轴Z*/


int Count = 0;
static int Step = 0;
int sum;
int str = 1;
typedef struct {
  float x;
  float y;
  float z;
} Acceleration;

Acceleration inputOld, inputNew;

long accelX, accelY, accelZ;      // 定义为全局变量，可直接在函数内部使用
float gForceX, gForceY, gForceZ;

long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Wire.begin();
  setupMPU();
}

void loop() {
  Acceleration lastInput, OutPut, thresholdInput;
  recordAccelRegisters();//读取加速度
  //recordGyroRegisters();
  lastInput.x = gForceX;
  lastInput.y = gForceY;
  lastInput.z = gForceZ;
  smoothingFilter(gForceX, gForceY, gForceZ, &OutPut);
  dynamicThreshold(&OutPut, &thresholdInput);
  if (judgment(&lastInput)) {
    detectStep(&OutPut, &lastInput, &thresholdInput);
  }
  printData();
  Count++;
  inputOld.x = inputNew.x;
  inputOld.y = inputNew.y;
  inputOld.z = inputNew.z;
  inputNew.x = lastInput.x;
  inputNew.y = lastInput.y;
  inputNew.z = lastInput.z;
  delay(5);
  Serial.println(Step);
  str = 1;
}

void setupMPU() {
  // REGISTER 0x6B/REGISTER 107:Power Management 1
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet Sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B/107 - Power Management (Sec. 4.30)
  Wire.write(0b00000000); //Setting SLEEP register to 0, using the internal 8 Mhz oscillator
  Wire.endTransmission();

  // REGISTER 0x1b/REGISTER 27:Gyroscope Configuration
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4)
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s (转化为rpm:250/360 * 60 = 41.67rpm) 最高可以转化为2000deg./s
  Wire.endTransmission();

  // REGISTER 0x1C/REGISTER 28:ACCELEROMETER CONFIGURATION
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000); //Setting the accel to +/- 2g（if choose +/- 16g，the value would be 0b00011000）
  Wire.endTransmission();
}

void recordAccelRegisters() {
  // REGISTER 0x3B~0x40/REGISTER 59~64
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000, 6); //Request Accel Registers (3B - 40)

  // 使用了左移<<和位运算|。Wire.read()一次读取1bytes，并在下一次调用时自动读取下一个地址的数据
  while (Wire.available() < 6); // Waiting for all the 6 bytes data to be sent from the slave machine （必须等待所有数据存储到缓冲区后才能读取）
  accelX = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX （自动存储为定义的long型值）
  accelY = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}

void processAccelData() {
  gForceX = accelX / 16384.0;     //float = long / float
  gForceY = accelY / 16384.0;
  gForceZ = accelZ / 16384.0;
}

void recordGyroRegisters() {
  // REGISTER 0x43~0x48/REGISTER 67~72
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000, 6); //Request Gyro Registers (43 ~ 48)
  while (Wire.available() < 6);
  gyroX = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0;
  rotZ = gyroZ / 131.0;
}

void printData() {
  Serial.print("Gyro (deg)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.print(rotZ);
  Serial.print(" Accel (g)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.println(gForceZ);
  Serial1.print(str);
  Serial.print(str);
}
//平滑滤波
void smoothingFilter(int GFX, int GFY, int GFZ, Acceleration *Output) {
  static float sumx, sumy, sumz;
  sumx += GFX; sumy += GFY; sumz += GFZ;
  if (Count % 4 == 0) {
    Output->x = sumx / 4.000;
    Output->y = sumy / 4.000;
    Output->z = sumz / 4.000;
    sumx = 0;
    sumy = 0;
    sumz = 0;
  }
}
//动态阀值
void dynamicThreshold(Acceleration *Iutput, Acceleration *Output) {
  static float xmax, ymax, zmax, xmin, ymin, zmin;
  xmax = max(Iutput->x, xmax);
  ymax = max(Iutput->y, ymax);
  zmax = max(Iutput->z, zmax);
  xmin = min(Iutput->x, xmin);
  ymin = min(Iutput->y, ymin);
  zmin = min(Iutput->z, zmin);
  if (Count % 50 == 0) {
    Output->x = dynamicAccuracy(xmax, xmin);
    Output->y = dynamicAccuracy(ymax, ymin);
    Output->z = dynamicAccuracy(zmax, zmin);
  }
}

float dynamicAccuracy(float Max, float Min) {
  return (Max + Min) / 2.000;
}
//判断是否晃动
bool judgment(Acceleration *lastInput) {
  bool res = 0;
  if (abs(inputNew.x - inputOld.x) > DYNAMIC_PRECISION) {
    inputOld.x = inputNew.x;
    inputNew.x = lastInput->x;
    res = 1;
  }
  else {
    inputOld.x = inputNew.x;
  }
  if (abs(inputNew.y - inputOld.y) > DYNAMIC_PRECISION) {
    inputOld.y = inputNew.y;
    inputNew.y = lastInput->y;
    res = 1;
  }
  else {
    inputOld.y = inputNew.y;
  }
  if (abs(inputNew.z - inputOld.z) > DYNAMIC_PRECISION) {
    inputOld.z = inputNew.z;
    inputNew.z = lastInput->z;
    res = 1;
  }
  else {
    inputOld.z = inputNew.z;
  }
  return res;
}
//判断最活跃轴
char mostActiveAxis(Acceleration *oldInput, Acceleration *newInput) {
  char res = MOST_ACTIVE_NULL;
  float x_change = abs(newInput->x - oldInput->x);
  float y_change = abs(newInput->y - oldInput->y);
  float z_change = abs(newInput->z - oldInput->z);
  if (x_change > y_change) {
    if (y_change > z_change) {
      res = MOST_ACTIVE_X;
    }
    else if (x_change > z_change) {
      res = MOST_ACTIVE_X;
    }
    else {
      res = MOST_ACTIVE_Z;
    }
  }
  else if (y_change > z_change) {
    res = MOST_ACTIVE_Y;
  }
  else {
    res = MOST_ACTIVE_Z;
  }
  return res;
}
//判断是否有效晃动
void detectStep(Acceleration *oldInput, Acceleration *newInput, Acceleration *thresholdInput) {
  char res = mostActiveAxis(oldInput, newInput);
  switch (res) {
    case MOST_ACTIVE_NULL: {
        break;

      }
    case MOST_ACTIVE_X: {
        if (oldInput->x > thresholdInput->x && newInput->x < thresholdInput->x) {
          Step++;
          str = 0;
        }
        break;
      }
    case MOST_ACTIVE_Y: {
        if (oldInput->y > thresholdInput->y && newInput->y < thresholdInput->y) {
          Step++;
          str = 0;
        }
        break;
      }
    case MOST_ACTIVE_Z: {
        if (oldInput->z > thresholdInput->z && newInput->z < thresholdInput->z) {
          Step++;
          str = 0;
        }
        break;
      }
  }

}

