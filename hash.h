#ifndef WINGLET_HASH_H_
#define WINGLET_HASH_H_

#include <ctime>
#include "defines.h"

// random  64-bit keys to give every position an 'almost' unique signature:

struct HashKeys
{
	
	// total size = 1093 * 8 = 8744 bytes (minimum required is 6312):
	U64 keys[64][16];  // position, piece (only 12 out of 16 piece are values used)
	U64 side;          // side to move (black)
	U64 ep[64];        // ep targets (only 16 used)
	U64 wk;            // white king-side castling right
	U64 wq;            // white queen-side castling right
	U64 bk;            // black king-side castling right
	U64 bq;            // black queen-side castling right
  uint64_t s;

inline void init()
{
	// initialize all random 64-bit numbers

	int i,j;
	time_t now;

	// use current time (in seconds) as random seed:
	unsigned char seed = std::time(&now) & 0x7f;
  s = seed;

	for (i = 0; i < 64; i++)
	{
		ep[i] = rand64();
		for (j=0; j < 16; j++) keys[i][j] = rand64();
	}
	side = rand64();
	wk = rand64();
	wq = rand64();
	bk = rand64();
	bq = rand64();

	return;
}

//inline U64 HashKeys::rand64()
//{
//	return rand()^((U64)rand()<<15)^((U64)rand()<<30)^((U64)rand()<<45)^((U64)rand()<<60);
//}

inline uint64_t rand64() {
    s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
//		s ^= (s ^= (s ^= s >> 12) << 25) >> 27;
    return s * 2685821657736338717LL;
  }
};

#endif
