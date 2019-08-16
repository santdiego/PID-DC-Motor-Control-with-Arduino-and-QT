//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar

class Encoder {
  private:
    int      _pinA, _pinB; //interrupt pins
    float    rps;
    unsigned ppr;//pulses per revolution
    unsigned long pulse_ctr;
    static Encoder*  sEncoder;
    float    ts; //sample time
    unsigned long t_us; //actual time in us
    unsigned long timeold; //last entry time in us;
    
  public:
    Encoder(unsigned pulses_per_revolution, const int& pinA, const int& pinB) {
      sEncoder=this;
      ppr = pulses_per_revolution;
      _pinA = pinA;
      _pinB = pinB;
      pulse_ctr=0;
      rps = 0;
      pinMode (_pinA, INPUT);
      pinMode (_pinB, INPUT);
      attachInterrupt(digitalPinToInterrupt(_pinA), encoderInputISR, RISING);
    }
    float getPulseCount() {
      return pulse_ctr;
    }
    float getRPS() {
      t_us = micros();
      ts          = float(t_us - timeold) / 1e6;
      timeold = t_us;
      rps         = (float(pulse_ctr) / float(ppr)) / ts;
      pulse_ctr = 0;
      return rps;
    }
  public :
    void pulseCounter() {
      pulse_ctr++;
    }
    static void encoderInputISR() {
      sEncoder->pulseCounter();
    }
};
Encoder* Encoder::sEncoder;
