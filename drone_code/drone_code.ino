#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


#include <WiFi.h>
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
WiFiServer tcpServer(8080);
WiFiClient client;

Adafruit_MPU6050 mpu;

const String FIRMWARE_VERSION = "1.1";

byte pinA = 4;
byte pinB = 5;
byte pinC = 3;
byte pinD = 6;

byte mode = 0;

const float MAX_ANGLE = 800;
const byte TURNING_THRUST_LIMIT = 120;
float P = 0.02;
float I = 0.00001;
float D = 5;

float yaw = 0;

float targetGyroX = 0;
float targetGyroY = 0;

float gyroOffsetX = 0;
float gyroOffsetY = 0;
float accOffsetX = 0;
float accOffsetY = 0;
float accOffsetZ = 0;

float gyroX = 0;
float gyroY = 0;
float lastGyroX = 0;
float lastGyroY = 0;
float I_valX = 0;
float I_valY = 0;

int thrustA = 0;
int thrustB = 0;
int thrustC = 0;
int thrustD = 0;

unsigned long lastTime = 0;
 
void setup() {
  Serial.begin(115200);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(7, HIGH); // LED blue
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);

  delay(3000);

  Wire.begin(11,10);
  if (!mpu.begin(0x68)) {
    Serial.println("Failed to find MPU6050 chip");
    digitalWrite(7, LOW); // LED red
    digitalWrite(8, HIGH);
    while (1) {delay(10);}
  }
  //mpu.calcOffsets(true, true);
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  
  sensors_event_t a, g, temp;

  unsigned int numCalibReadings = 3000;

  Serial.println("Callibrating, please wait");

  for (unsigned int i=0; i<numCalibReadings; i++) {
    mpu.getEvent(&a, &g, &temp);
    gyroOffsetX += g.gyro.x;
    gyroOffsetY += g.gyro.y;
    accOffsetX += a.acceleration.x;
    accOffsetY += a.acceleration.y;
    accOffsetZ += a.acceleration.z;
    delay(2);
  }
  gyroOffsetX /= numCalibReadings;
  gyroOffsetY /= numCalibReadings;
  accOffsetX /= numCalibReadings;
  accOffsetY /= numCalibReadings;
  accOffsetZ /= numCalibReadings;

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("AeroHacks Drone 1", "skibidi123");
  tcpServer.begin();

  Serial.println("ready");
  digitalWrite(7, LOW); // LED green
  digitalWrite(8, LOW);
  digitalWrite(9, HIGH);

  lastTime = millis();
}








void loop() {
  unsigned long newTime = millis();
  unsigned int dt = newTime - lastTime;
  lastTime = newTime;

  lastGyroX = gyroX;
  lastGyroY = gyroY;


  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float gyroVX = g.gyro.x - gyroOffsetX;
  float gyroVY = g.gyro.y - gyroOffsetY;
  gyroX -= gyroVX * dt;
  gyroY -= gyroVY * dt;
  /*
  float rawAccZ = a.acceleration.z - accOffsetZ;
  float rawAccX = a.acceleration.x - accOffsetX;
  float rawAccY = a.acceleration.y - accOffsetY;
  float accZ = rawAccZ * cos(gyroX * PI/180) * cos(gyroY * PI/180) - rawAccX * sin(gyroX * PI/180) - rawAccY * sin(gyroY * PI/180);
  float accX = rawAccX * cos(gyroX * PI/180) + rawAccZ * sin(gyroX * PI/180);
  float accY = rawAccY * cos(gyroY * PI/180) + rawAccZ * sin(gyroY * PI/180);
  */

  if (gyroX > MAX_ANGLE or gyroX < -MAX_ANGLE or gyroY > MAX_ANGLE or gyroY < -MAX_ANGLE) {
    mode = 0;
    digitalWrite(8, HIGH);
  }

  if (!client) {client = tcpServer.available();}
  else if (!client.connected()) {
    client.stop();
    mode = 0;
  }

  if (client.available()) {
    String instruct = client.readStringUntil('\n');

    if (instruct == "ping") {
      client.print("ping");
    }

    else if (instruct == "angX") {client.print(String(gyroX));}
    else if (instruct == "angY") {client.print(String(gyroY));}
    else if (instruct == "gyroX") {client.print(String(gyroVX));}
    else if (instruct == "gyroY") {client.print(String(gyroVY));}
    else if (instruct == "gMode") {client.print(String(mode));}
    else if (instruct == "vers") {client.print(FIRMWARE_VERSION);}
    else if (instruct == "lb1") {digitalWrite(7, HIGH);}
    else if (instruct == "lb0") {digitalWrite(7, LOW);}
    else if (instruct == "lr1") {digitalWrite(8, HIGH);}
    else if (instruct == "lr0") {digitalWrite(8, LOW);}
    else if (instruct == "lg1") {digitalWrite(9, HIGH);}
    else if (instruct == "lg0") {digitalWrite(9, LOW);}
    
    else if (instruct.startsWith("mode")) {
      instruct.remove(0, 4);
      mode = instruct.toInt();
      Serial.print("New Mode: ");
      Serial.print(mode);
    }
    
    else if (instruct.startsWith("gx")) {
      instruct.remove(0, 2);
      targetGyroX = instruct.toInt();
    }
    
    else if (instruct.startsWith("gy")) {
      instruct.remove(0, 2);
      targetGyroY = instruct.toInt();
    }
    
    else if (instruct.startsWith("gainP")) {
      instruct.remove(0, 5);
      P = instruct.toFloat();
    }
    
    else if (instruct.startsWith("gainI")) {
      instruct.remove(0, 5);
      I = instruct.toFloat();
    }
    
    else if (instruct.startsWith("gainD")) {
      instruct.remove(0, 5);
      D = instruct.toFloat();
    }
    
    else if (instruct.startsWith("yaw")) {
      instruct.remove(0, 3);
      yaw = instruct.toFloat();
    }
    
    else if (instruct == "irst") {
      I_valX = 0;
      I_valY = 0;
    }

    else if (instruct == "geti"){
      client.print(I_valX);
      client.print(',');
      client.print(I_valY);
    }

    else if (instruct == "manT") {
      thrustA = client.readStringUntil(',').toInt();
      thrustB = client.readStringUntil(',').toInt();
      thrustC = client.readStringUntil(',').toInt();
      thrustD = client.readStringUntil('\n').toInt();
    }

    else if (instruct == "incT") {
      thrustA += client.readStringUntil(',').toInt();
      thrustB += client.readStringUntil(',').toInt();
      thrustC += client.readStringUntil(',').toInt();
      thrustD += client.readStringUntil('\n').toInt();
    }




    client.print("\n");
  }

  float thrustOffA = 0;
  float thrustOffB = 0;
  float thrustOffC = 0;
  float thrustOffD = 0;

  if (mode == 2){
    I_valX += (gyroX - targetGyroX) * dt;
    I_valY += (gyroY - targetGyroY) * dt;

    thrustOffA -= P * (gyroX - targetGyroX) * dt;
    thrustOffB -= P * (gyroX - targetGyroX) * dt;
    thrustOffC += P * (gyroX - targetGyroX) * dt;
    thrustOffD += P * (gyroX - targetGyroX) * dt;

    thrustOffA -= I * I_valX * dt;
    thrustOffB -= I * I_valX * dt;
    thrustOffC += I * I_valX * dt;
    thrustOffD += I * I_valX * dt;

    thrustOffA += D * gyroVX * dt;
    thrustOffB += D * gyroVX * dt;
    thrustOffC -= D * gyroVX * dt;
    thrustOffD -= D * gyroVX * dt;


    thrustOffA -= P * (gyroY - targetGyroY) * dt;
    thrustOffB += P * (gyroY - targetGyroY) * dt;
    thrustOffC -= P * (gyroY - targetGyroY) * dt;
    thrustOffD += P * (gyroY - targetGyroY) * dt;

    thrustOffA -= I * I_valY * dt;
    thrustOffB += I * I_valY * dt;
    thrustOffC -= I * I_valY * dt;
    thrustOffD += I * I_valY * dt;

    thrustOffA += D * gyroVY * dt;
    thrustOffB -= D * gyroVY * dt;
    thrustOffC += D * gyroVY * dt;
    thrustOffD -= D * gyroVY * dt;
  }


  if (thrustA < 0) {thrustA = 0;}
  if (thrustB < 0) {thrustB = 0;}
  if (thrustC < 0) {thrustC = 0;}
  if (thrustD < 0) {thrustD = 0;}
  if (thrustA > 200) {thrustA = 200;}
  if (thrustB > 200) {thrustB = 200;}
  if (thrustC > 200) {thrustC = 200;}
  if (thrustD > 200) {thrustD = 200;}


  if (mode == 0) {
    yaw = 0;
    thrustA = 0;
    thrustB = 0;
    thrustC = 0;
    thrustD = 0;
  }

  if (mode <= 1){
    thrustOffA = 0;
    thrustOffB = 0;
    thrustOffC = 0;
    thrustOffD = 0;
  }

  if (thrustOffA < -TURNING_THRUST_LIMIT) {thrustOffA = -TURNING_THRUST_LIMIT;}
  if (thrustOffB < -TURNING_THRUST_LIMIT) {thrustOffB = -TURNING_THRUST_LIMIT;}
  if (thrustOffC < -TURNING_THRUST_LIMIT) {thrustOffC = -TURNING_THRUST_LIMIT;}
  if (thrustOffD < -TURNING_THRUST_LIMIT) {thrustOffD = -TURNING_THRUST_LIMIT;}
  if (thrustOffA > TURNING_THRUST_LIMIT) {thrustOffA = TURNING_THRUST_LIMIT;}
  if (thrustOffB > TURNING_THRUST_LIMIT) {thrustOffB = TURNING_THRUST_LIMIT;}
  if (thrustOffC > TURNING_THRUST_LIMIT) {thrustOffC = TURNING_THRUST_LIMIT;}
  if (thrustOffD > TURNING_THRUST_LIMIT) {thrustOffD = TURNING_THRUST_LIMIT;}

  int newThrustA = thrustA + thrustOffA - yaw;
  int newThrustB = thrustB + thrustOffB + yaw;
  int newThrustC = thrustC + thrustOffC + yaw;
  int newThrustD = thrustD + thrustOffD - yaw;


  if (newThrustA < 0) {newThrustA = 0;}
  if (newThrustB < 0) {newThrustB = 0;}
  if (newThrustC < 0) {newThrustC = 0;}
  if (newThrustD < 0) {newThrustD = 0;}
  if (newThrustA > 250) {newThrustA = 250;}
  if (newThrustB > 250) {newThrustB = 250;}
  if (newThrustC > 250) {newThrustC = 250;}
  if (newThrustD > 250) {newThrustD = 250;}

  analogWrite(pinA, newThrustA);
  analogWrite(pinB, newThrustB);
  analogWrite(pinC, newThrustC);
  analogWrite(pinD, newThrustD);
}
