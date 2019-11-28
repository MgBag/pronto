#pragma once
#include <chrono>

namespace pronto
{
  class Timer
  {
  public:
    static void Update();
    static uint64_t frame_count();
    static double delta_time();
    static float delta_time_f();

  private:
    static std::chrono::high_resolution_clock clock_;
    static uint64_t frame_counter_;
    static double elapsed_seconds_;
    static double time_since_start_;
    static double delta_time_;
  };
}