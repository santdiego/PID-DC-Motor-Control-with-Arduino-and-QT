//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar

class DCMotor {
    float       _current;
    float       _maxVoltage;
    byte    pwm_motor;
    int     _pwm_pin;
    int     _current_pin;

  public:
    DCMotor(const int& pwm_pin, const long& current_pin,  const float& maxVoltage = 12): _pwm_pin(pwm_pin), _current_pin(current_pin), _maxVoltage(maxVoltage) {
      pinMode (pwm_pin , OUTPUT);
      analogWrite(pwm_pin, 0);
      pinMode (current_pin , INPUT);
      _current = analogRead(current_pin);
    }
    void setVoltage(const float& voltage) {
      pwm_motor   = byte(voltage * 255.0 / _maxVoltage);            // convierto a  PWM
      if ( ( pwm_motor > 255 )) {
        pwm_motor   = 255;
      }
      if(( pwm_motor < 0 )){
        pwm_motor   = 0;
      }
      analogWrite(_pwm_pin, pwm_motor);                    // la mando por el pin
    }
    float getCurrent() {
      return analogRead(_current_pin);
    }
};
