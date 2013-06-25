#include <spin/spin.hpp>


int main()
{
  spin::event_loop e1;
  spin::event_loop e2;
  spin::event_loop e3;
  e1.run();
  e2.run();
  e3.run();
  return 0;
}
