#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"

const char* ssid = SSID;
const char* password = PSWD;

WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char responsePacket[255];  // buffer for response packets

enum packet_requests {
  REQ_QUA = 1,
  REQ_RPY,
  REQ_SSI,
  REQ_SRW,
  REQ_JOY,
  REQ_NOP,
  REQ_LAST
};


void comms_init() {

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

}

void incoming_packet_parser(char *buf, int len, int *cmd) {

  if(strncmp(buf, "$MQJOY", 6) > 0) {
    packet_joy_parser(buf);
    *cmd = REQ_JOY;
  } else if(strncmp(buf, "$MQRPY", 6) > 0) {
    *cmd = REQ_RPY;
  } else if(strncmp(buf, "$MQQUA", 6) > 0) {
    *cmd = REQ_QUA;
  } else {
    *cmd = REQ_NOP;
  }

}

void packet_joy_parser(char *buf) {

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

void packet_response_handler(int cmd, char *buf) {

  int i_ax, i_ay, i_az, i_gx, i_gy, i_gz, i_mx, i_my, i_mz;
  float f_ax, f_ay, f_az, f_gx, f_gy, f_gz, f_mx, f_my, f_mz;
  float quad_r = 0, quad_p = 0, quad_y = 0, quad_h = 0;
  float quad_q[4];

  switch(cmd) {

    case REQ_QUA:

           get_quaternion(quad_q);

           sprintf(buf, "$MQQUA,%d,%f,%f,%f,%f,*", \
                     t_loop_2 - t_loop_1, \
                     quad_q[3], \
                     quad_q[2], \
                     quad_q[1], \
                     quad_q[0] \
                    );
           break;

    case REQ_RPY:

           get_rpy(&quad_r, &quad_p, &quad_y, &quad_h);

           sprintf(buf, "$MQRPY,%d,%f,%f,%f,%f,*", \
              t_loop_2 - t_loop_1, \
              quad_r, \
              quad_p, \
              quad_y, \
              quad_h \
             );
           break;

    case REQ_SSI:

           get_norm(&f_ax, &f_ay, &f_az, &f_gx, &f_gy, &f_gz, &f_mx, &f_my, &f_mz);
    
           sprintf(buf, "$MQSSI,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,*", \
              t_loop_2 - t_loop_1, \
              (int)1000 * f_ax, \
              (int)1000 * f_ay, \
              (int)1000 * f_az, \
              f_gx, \
              f_gy, \
              f_gz, \
              (int)1000 * f_mx, \
              (int)1000 * f_my, \
              (int)1000 * f_mz \
             );
           break;

    case REQ_SRW:

           get_raw(&i_ax, &i_ay, &i_az, &i_gx, &i_gy, &i_gz, &i_mx, &i_my, &i_mz);

           sprintf(buf, "$MQSRW,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,*", \
              t_loop_2 - t_loop_1, \
              (int)i_ax, \
              (int)i_ay, \
              (int)i_az, \
              (int)i_gx, \
              (int)i_gy, \
              (int)i_gz, \
              (int)i_mx, \
              (int)i_my, \
              (int)i_mz \
             );
           break;

    case REQ_JOY:
           sprintf(buf, "$MQOOK*");
           break;

    case REQ_NOP:
           buf[0] = 0;
           break;

    default: sprintf(buf, "$MQERR*");
  }
}

void comms_check_new_packet_arrival() {

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    int cmd = 0;
    incoming_packet_parser(incomingPacket, len, &cmd);
    packet_response_handler(cmd, responsePacket);

    rpi_IP   = Udp.remoteIP();
    rpi_Port = Udp.remotePort();
    rpi_last_connection = millis();

    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(rpi_IP, rpi_Port);
    Udp.write(responsePacket);
    Udp.endPacket();
  }
}

