#include "StateMachine.h"

StateMachine::StateMachine()
  : state(0)
{

}

bool StateMachine::isEnabled(int state_)
{
  return (state & state_) == state_;
}

void StateMachine::enable(int state_)
{
  state |= state_;
}

void StateMachine::disable(int state_)
{
  state &= state_;
}

void StateMachine::toggle(int state_)
{
  state ^= state_;
}
