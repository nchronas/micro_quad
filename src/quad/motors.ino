#define M1_PIN 12
#define M2_PIN 13
#define M3_PIN 14
#define M4_PIN 15

void motors_init() {
  analogWrite(M1_PIN, 0);
  analogWrite(M2_PIN, 0);
  analogWrite(M3_PIN, 0);
  analogWrite(M4_PIN, 0);
}

void motor_limit(int *m) {
  if(*m > 1023)   { *m = 1023; }
  else if(*m < 0) { *m = 0; } 
}

void motor_mixing() {

  set_motors(ctrl_data.alt, ctrl_data.alt, ctrl_data.alt, ctrl_data.alt);
}

void set_motors(int m1, int m2,int m3, int m4) {

  motor_limit(&m1);
  motor_limit(&m2);
  motor_limit(&m3);
  motor_limit(&m4);

  if(ctrl_data.en) {
    //pwm motor test
    analogWrite(M1_PIN, m1);
    analogWrite(M2_PIN, m2);
    analogWrite(M3_PIN, m3);
    analogWrite(M4_PIN, m4);
  } else {
    analogWrite(M1_PIN, 0);
    analogWrite(M2_PIN, 0);
    analogWrite(M3_PIN, 0);
    analogWrite(M4_PIN, 0);
  }
}

