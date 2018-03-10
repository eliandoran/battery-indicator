#include <time.h>

void sleep_ms(int delay)
{
	const struct timespec sleep_ts = {
		delay / 1000,
		(delay % 1000) * 1000000
	};

	nanosleep(&sleep_ts, NULL);
}
