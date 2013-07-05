#ifndef STATEMACHINE_H
#define STATEMACHINE_H

enum DrawOptions {
  TERRAIN    = 0x01,
  TERRAIN_LINES     = 0x02,
  TERRAIN_SELECT   = 0x04,
  TERRAIN_2D  = 0x08,
  TERRAIN_CONTOR    = 0x10
};

class StateMachine
{
public:
  StateMachine();

  bool isEnabled(int state_);

  void enable(int state_);
  void disable(int state_);

  void toggle(int state_);

private:
  int state;
};

#endif //STATEMACHINE_H
