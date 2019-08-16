#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

///
/// \brief The TaskScheduler class
///
/// */

#define MAXCALLS 50
class TaskScheduler
{
    long max_ticks; //maximun numbers of tiks;
    int fcn_index; //number of functions stored
    struct FcnCall {
      void(*ptrfcn)();
      int ticks2run;
    } fcn_call[MAXCALLS];
    static TaskScheduler* selfptr;
    long ticks_cnt; //stores actual tick cnt
    long last_tick;
  public:

    TaskScheduler();
    static void timeTicker();
    bool addTask(void(*ptrfcn)(), int tick_counts = 1);
    void dispatcher();
};

TaskScheduler* TaskScheduler::selfptr;
TaskScheduler::TaskScheduler()
{
  selfptr = this;
  ticks_cnt = 0;
  last_tick = 0;
  max_ticks = 0;
  fcn_index = 0;
}
void TaskScheduler::timeTicker() {
  selfptr->ticks_cnt++;
}
bool TaskScheduler::addTask(void(*ptrfcn)(), int tick_counts) {
  if (fcn_index > (MAXCALLS - 1))
    return false;
  fcn_call[fcn_index].ticks2run = tick_counts;
  fcn_call[fcn_index].ptrfcn = ptrfcn;
  fcn_index++;
  if (tick_counts > max_ticks)
    max_ticks = tick_counts;
  return true;
}
void TaskScheduler::dispatcher() {
  if (ticks_cnt == last_tick)
    return;
  last_tick = ticks_cnt;
  if (last_tick >= max_ticks) //reset tick count
    ticks_cnt = 0;
  for (int i = 0; i < fcn_index; i++) {
    if (fcn_call[i].ticks2run <= last_tick) {
      fcn_call[i].ptrfcn();
    }
  }
}

#endif // TASKSCHEDULER_H
