#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS0.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!
#include "config.h"

#define M1_PIN 12
#define M2_PIN 13
#define M3_PIN 14
#define M4_PIN 15

// i2c
Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0();
char lsm_data[100];

const char* ssid = SSID;
const char* password = PSWD;

WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets

unsigned int rpi_IP = 0, rpi_Port = 0;
unsigned int rpi_connected = false;

int t_loop_1 = 0, t_loop_2 = 0;

struct _ctrl_data{
  int alt;
  int x;
  int y;
  int en;
};

struct _ctrl_data ctrl_data = { .alt = 0, \
                                .x   = 0, \
                                .y   = 0, \
                                .en  = 0, \
                                };

void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_6G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS0_MAGGAIN_2GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_12GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_2000DPS);

  //aRes = aScale == A_SCALE_16G ? 16.0 / 32768.0 : 
  //(((float) aScale + 1.0) * 2.0) / 32768.0;

  //gRes = 245.0 / 32768.0;

  //mRes = mScale == M_SCALE_2GS ? 2.0 / 32768.0 : 
  //(float) (mScale << 2) / 32768.0;
}

void packet_ctrl_parser(char *buf, int len) {

  char *field = 0;
  char *saveptr = buf;

  field = strtok_r(saveptr, ",", &saveptr);
  //if(strncmp("", field) ) 

  field = strtok_r(saveptr, ",", &saveptr);
  ctrl_data.alt = atoi(field);
  
  field = strtok_r(saveptr, ",", &saveptr);
  ctrl_data.x = atoi(field);
 
  field = strtok_r(saveptr, ",", &saveptr);
  ctrl_data.y = atoi(field);

  field = strtok_r(saveptr, ",", &saveptr);
  ctrl_data.en = atoi(field);

  Serial.print("CTRL: ");
  Serial.print(ctrl_data.alt);
  Serial.print(" ");
  Serial.print(ctrl_data.x);
  Serial.print(" ");
  Serial.print(ctrl_data.y);
  Serial.print(" ");
  Serial.println(ctrl_data.en);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS0. Check your wiring!");
    while (1);
  }
  Serial.println("Found LSM9DS0 9DOF");
  Serial.println("");
  Serial.println("");

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  //pwm motor init
  analogWrite(M1_PIN, 0);
  analogWrite(M2_PIN, 0);
  analogWrite(M3_PIN, 0);
  analogWrite(M4_PIN, 0);

  t_loop_2 = millis();
}


void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    packet_ctrl_parser(incomingPacket, len);

    rpi_IP   = Udp.remoteIP();
    rpi_Port = Udp.remotePort();
    rpi_connected = true;
  }

  sensors_event_t accel, mag, gyro, temp;

  lsm.getEvent(&accel, &mag, &gyro, &temp); 
  AHRS(accel.acceleration.x, \
       accel.acceleration.y, \
       accel.acceleration.z, \
       gyro.gyro.x, \
       gyro.gyro.y, \
       gyro.gyro.z, \
       mag.magnetic.x, \
       mag.magnetic.y, \
       mag.magnetic.z \
       );

  sprintf(lsm_data, "$MQD,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,*", \
                          t_loop_2 - t_loop_1, \
                          (int)lsm.accelData.x, \
                          (int)lsm.accelData.y, \
                          (int)lsm.accelData.z, \
                          (int)lsm.gyroData.x, \
                          (int)lsm.gyroData.y, \
                          (int)lsm.gyroData.z, \
                          (int)lsm.magData.x, \
                          (int)lsm.magData.y, \
                          (int)lsm.magData.z \
                          );

  Serial.println(lsm_data);

  // send back a reply, to the IP address and port we got the packet from
  if(rpi_connected) {
    Udp.beginPacket(rpi_IP, rpi_Port);
    Udp.write(lsm_data);
    Udp.endPacket();
  }

  //pwm motor test
  analogWrite(M1_PIN, ctrl_data.alt);
  analogWrite(M2_PIN, ctrl_data.alt);
  analogWrite(M3_PIN, ctrl_data.alt);
  analogWrite(M4_PIN, ctrl_data.alt);

  //calculating period of loop function
  t_loop_1 = t_loop_2;
  t_loop_2 = millis();

}

