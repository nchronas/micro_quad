#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS0.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!
#include "config.h"

char lsm_data[100];

unsigned int rpi_IP = 0, rpi_Port = 0;
unsigned int rpi_connected = false;
unsigned int tx_rate_cnt = 0;
int rpi_last_connection = 0;

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

void setup()
{
  Serial.begin(115200);
  Serial.println();

  comms_init();

  imu_init();
  
  motors_init();

  t_loop_2 = millis();
  tx_rate_cnt = t_loop_2;
}

void loop()
{
  float quad_r = 0, quad_p = 0, quad_y = 0, quad_h = 0;

  imu_update_pos();

  get_rpy(&quad_r, &quad_p, &quad_y, &quad_h);

  motor_mixing();

  comms_check_new_packet_arrival();

  if(t_loop_2 - tx_rate_cnt > 1000) {
    tx_rate_cnt = t_loop_2;

    packet_response_handler(2, lsm_data);
    Serial.println(lsm_data);
  }

  //calculating period of loop function
  t_loop_1 = t_loop_2;
  t_loop_2 = millis();

}

