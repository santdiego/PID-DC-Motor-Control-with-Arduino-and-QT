class PID {
    float          _K, _Ki, _Kd, ts; //control parameters
    float          A, B, C, D;     //control calculation constants
    float          _V_MAX, _V_MIN; //saturations
    float          e1, e2;          //control error
    float          u, u1;          //control action
    float          y, y1, y2;
    bool          pidModified;
  public:
    PID(float K = 1, float Ki = 0, float Kd = 0, float sampleTime = 0.001, float V_MAX = 10000, float V_MIN = 0, bool is_modified = false) {
      setConstants(K, Ki, Kd, sampleTime);
      setSaturations(V_MAX, V_MIN);
      e1, e2 = 0;
      u1, u = 0;
      y2, y1, y = 0;
      pidModified = is_modified;
    }
    void usePidModified(bool modified_on) {
      pidModified = modified_on;
    }
    void setConstants(float K = 1, float Ki = 0, float Kd = 0, float sampleTime = 0.001) {
      if (sampleTime == 0)    return;
      _K = K;
      _Ki = Ki;
      _Kd = Kd;
      ts = sampleTime;
      //PID MODIFIED;
      if (pidModified) {
        A           = - K * ( 1 + Kd / ts );
        B           = - K * ( -1 + 2 * Kd / ts );
        C           = - K * ( Kd / ts);
        D           = K * Ki * ts;
      } else {
        //REGULAR PID
        A           = K * ( 1 + Kd / ts );
        B           = K * ( Ki * ts - 2 * Kd / ts - 1 );
        C           = K * ( Kd / ts);
      }
    }
    void setSaturations(float V_MAX, float V_MIN = 0) {
      _V_MAX = V_MAX;
      _V_MIN = V_MIN;
    }

    float getControlAction(float e) {
      if (pidModified) {
        u         = u1 + A * y + B * y1 + C * y2 + D * e1;
        u1 = u;
        y2 = y1;
        y1 = y;
        e1 = e;
      } else {
        u = u1 + A * e + B * e1 + C * e2;
        u1 = u;
        e2 = e1;
        e1 = e;
      }
      if ( u > _V_MAX )
        u = _V_MAX;
      if ( u < _V_MIN )
        u = _V_MIN;

      return u;
    }
};
