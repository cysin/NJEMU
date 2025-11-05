/******************************************************************************

	ticker.h

	SDL2 timing functions (64-bit microsecond counter)

******************************************************************************/

#ifndef SDL2_TICKER_H
#define SDL2_TICKER_H

typedef UINT64 TICKER;

#define TICKS_PER_SEC	1000000

TICKER ticker(void);

#endif /* SDL2_TICKER_H */
