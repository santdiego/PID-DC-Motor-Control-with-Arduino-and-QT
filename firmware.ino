//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar

#include <TimerOne.h>
#include <EEPROM.h>
#include "serialframedecoder.h"
#include "encoder.h"
#include "dcmotor.h"
#include "pidmodif.h"


DCMotor dcMotor(5, A0, 12);
Encoder encoder(500, 2, 3);
PID   pid(7.0, 3.0, 0, .005, 12, 0);

struct ControlTable{
  int16_t     goal_vel = 0;
  uint16_t    Ki = 0, Kp = 0, Kd = 0;
  int16_t     vel           = 0;
  int16_t    ts            = 0;
  int16_t    u            = 0;
} control_table;

const int   addr_goal_vel = 0;
const int   addr_Ki       = 2;
const int   addr_Kp       = 4;
const int   addr_Kd       = 6;
const int   addr_vel      = 8;
const int   addr_ts       = 10;
const int   addr_u        = 12;

float rps = 0;
float e = 0;
float ref = 0;
float u = 0;
unsigned long   timeold = 0;
unsigned long   us = 0;

void control() {

  us = micros();
  control_table.ts = us - timeold;                 //send sample Time[us]
  timeold = us;
  if (control_table.goal_vel >= 0)
    ref = float(control_table.goal_vel) / 1000;
  rps = encoder.getRPS();
  e = ref - rps;
  u = pid.getControlAction(e);
  control_table.u=u*1000;
  dcMotor.setVoltage(u);
  control_table.vel = rps * 1000;   //format velocity

}

void setup() {
  Serial.begin(115200);
  Serial.print("DC Motor PID Tuning. \n Authors: *Dr. Ing. Diego Santiago,  Dr. Ing. Marcos Toibero \n *dsantiago@inaut.unsj.edu.ar");
  Timer1.initialize(5000);
  Timer1.attachInterrupt(control);
}

byte          id                  = 1;
Instruction   inst                = Write;
byte          param[BUFFERSIZE]   = {0};
byte          frame[BUFFERSIZE]   = {0};
uint16_t      param_length        = 0;
byte16        addr;//address were data is stored.
int           datasize = 0; //siz of data too be stored
SerialFrameDecoder writeFrame;
SerialFrameDecoder readFrame;

void loop() {
  if (readFrame.newData()) {
    if (readFrame.getInstructionPacket(id, inst, param, param_length) == No_Error) {
      if (inst == Write) {
        readFrame.clean();
        addr.word[0]  = param[0];
        addr.word[1]  = param[1];
        datasize = param_length - 2; //remove address
        if (datasize < BUFFERSIZE) {
          for (int i = 0; i < datasize; i++) {
            //update Ki,Kp,Kd,goal_vel
            EEPROM.write(addr.value + i, param[i + 2]);
          }
        }
      }
    }
    //Take all control table and re-transmit, also update control_table with last incoming information
    EEPROM.put(addr_vel, control_table.vel);
    EEPROM.put(addr_ts, control_table.ts);
    EEPROM.put(addr_u, control_table.u);
    EEPROM.get(0, control_table);
    pid.setConstants(float(control_table.Kp)/1000,float( control_table.Ki)/1000,float(control_table.Kd)/1000, 0.005);
    writeFrame.clean();
    writeFrame.buildStatusPacket(1, (byte*)&control_table, sizeof(ControlTable), 0);
    writeFrame.getLastFrame(frame, param_length);
    Serial.write(frame, param_length);
  }
}

void serialEvent() {
  while (Serial.available()) {
    readFrame.appendFrameByte(Serial.read());
  }
}
