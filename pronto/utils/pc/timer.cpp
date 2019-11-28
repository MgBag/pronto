#include "timer.h"

namespace pronto
{
  std::chrono::high_resolution_clock Timer::clock_;
  uint64_t Timer::frame_counter_ = 0;
  double Timer::elapsed_seconds_ = 0.0;
  double Timer::time_since_start_ = 0.0;
  double Timer::delta_time_ = 0.0;

  void Timer::Update()
  {
    static auto t0 = clock_.now();
    auto t1 = clock_.now();
    std::chrono::duration<double, std::milli> delta_time = t1 - t0;
    delta_time_ = delta_time.count();

    t0 = t1;
    time_since_start_ += delta_time.count();
    std::chrono::duration<double, std::ratio<1, 1>> delta_seconds = delta_time;
    //elapsed_seconds += delta_seconds.count();

    //if (elapsed_seconds > 0.05)
    //{
    //  auto fps = frame_counter / elapsed_seconds;
    //  printf_s("MS: %f FPS: %f\n", delta_time.count(), fps);
    //  //OutputDebugString(buffer);

    //  frame_counter = 0;
    //  elapsed_seconds = 0.0;
    //}

    frame_counter_++;
  }
  
  uint64_t Timer::frame_count()
  {
    return frame_counter_;
  }

  double Timer::delta_time()
  {
    return delta_time_;
  }

  float Timer::delta_time_f()
  {
    return (float)delta_time_;
  }
}
