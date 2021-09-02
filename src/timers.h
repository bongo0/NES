#ifndef TIMERS_H
#define TIMERS_H

#include <time.h>
#define CLOCK_INIT(id)                                                         \
  clock_t start_##id;                                                          \
  clock_t end_##id;                                                            \
  double time_##id = 0;                                                        \
  size_t n_measures_##id = 0;
#define CLOCK_START(id) start_##id = clock();
#define CLOCK_END(id)                                                          \
  end_##id = clock();                                                          \
  n_measures_##id++;                                                           \
  time_##id = (end_##id - start_##id);
#define CLOCK_RESULT_FULL_TIME_S(id) (time_##id / (double)CLOCKS_PER_SEC)
#define CLOCK_RESULT_AVERAGE_S(id)   ((time_##id / (double)CLOCKS_PER_SEC) / (double)n_measures_##id)
#define CLOCK_RESULT_AVERAGE_uS(id)   (time_##id / (double)n_measures_##id)
#define CLOCK_RESULT_FULL_TIME_uS(id) (time_##id)
#define CLOCK_RESULT_N_MEAS(id) (n_measures_##id)



#endif // TIMERS_H