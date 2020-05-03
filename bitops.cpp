#include <iostream>
#include <iomanip>
#ifdef _MSC_VER
#include <intrin.h>
#endif
#include "defines.h"
#include "extglobals.h"
inline unsigned int bitCnt(U64 b)
{
 
	//// MIT HAKMEM algorithm, see http://graphics.stanford.edu/~seander/bithacks.html
 //
	//static const U64  M1 = 0x5555555555555555;  // 1 zero,  1 one ...
	//static const U64  M2 = 0x3333333333333333;  // 2 zeros,  2 ones ...
	//static const U64  M4 = 0x0f0f0f0f0f0f0f0f;  // 4 zeros,  4 ones ...
	//static const U64  M8 = 0x00ff00ff00ff00ff;  // 8 zeros,  8 ones ...
	//static const U64 M16 = 0x0000ffff0000ffff;  // 16 zeros, 16 ones ...
	//static const U64 M32 = 0x00000000ffffffff;  // 32 zeros, 32 ones
 //
	//bitmap = (bitmap & M1 ) + ((bitmap >>  1) & M1 );   //put count of each  2 bits into those  2 bits
	//bitmap = (bitmap & M2 ) + ((bitmap >>  2) & M2 );   //put count of each  4 bits into those  4 bits
	//bitmap = (bitmap & M4 ) + ((bitmap >>  4) & M4 );   //put count of each  8 bits into those  8 bits
	//bitmap = (bitmap & M8 ) + ((bitmap >>  8) & M8 );   //put count of each 16 bits into those 16 bits
	//bitmap = (bitmap & M16) + ((bitmap >> 16) & M16);   //put count of each 32 bits into those 32 bits
	//bitmap = (bitmap & M32) + ((bitmap >> 32) & M32);   //put count of each 64 bits into those 64 bits

	//return (int)bitmap;
	
#ifdef _MSC_VER
#ifndef _WIN64
	unsigned lo = b, hi = (b >> 32);
	return __popcnt(lo) + __popcnt(hi);
#else
	return __popcnt64(b);
#endif
#elif defined(__GNUC__)
return __builtin_popcountll(b);
#else
	// ===========================
	// Use if bit count > 15
	// ===========================
/*#ifndef IS32BIT
	b -=  (b >> 1) & 0x5555555555555555ULL;
  b  = ((b >> 2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
  b  = ((b >> 4) + b) & 0x0F0F0F0F0F0F0F0FULL;
  return (b * 0x0101010101010101ULL) >> 56;
#else
	 unsigned w = unsigned(b >> 32), v = unsigned(b);
  v -=  (v >> 1) & 0x55555555; // 0-2 in 2 bits
  w -=  (w >> 1) & 0x55555555;
  v  = ((v >> 2) & 0x33333333) + (v & 0x33333333); // 0-4 in 4 bits
  w  = ((w >> 2) & 0x33333333) + (w & 0x33333333);
  v  = ((v >> 4) + v + (w >> 4) + w) & 0x0F0F0F0F;
  return (v * 0x01010101) >> 24;
#endif
	*/
	// ==============================
	// Use this ONLY IF bitcount < 15
	// ==============================
#ifndef IS32BIT
	b -=  (b >> 1) & 0x5555555555555555ULL;
  b  = ((b >> 2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
  return (b * 0x1111111111111111ULL) >> 60;
#else
	 unsigned w = unsigned(b >> 32), v = unsigned(b);
  v -=  (v >> 1) & 0x55555555; // 0-2 in 2 bits
  w -=  (w >> 1) & 0x55555555;
  v  = ((v >> 2) & 0x33333333) + (v & 0x33333333); // 0-4 in 4 bits
  w  = ((w >> 2) & 0x33333333) + (w & 0x33333333);
  return ((v + w) * 0x11111111) >> 28;
#endif
#endif
}

inline int firstOne(const U64 &bitmap)
{
#ifndef _MSC_VER
	// De Bruijn Multiplication, see http://chessprogramming.wikispaces.com/BitScan
	// don't use this if bitmap = 0
	static const int INDEX64[64] = {
	63,  0, 58,  1, 59, 47, 53,  2,
	60, 39, 48, 27, 54, 33, 42,  3,
	61, 51, 37, 40, 49, 18, 28, 20,
	55, 30, 34, 11, 43, 14, 22,  4,
	62, 57, 46, 52, 38, 26, 32, 41,
	50, 36, 17, 19, 29, 10, 13, 21,
	56, 45, 25, 31, 35, 16,  9, 12,
	44, 24, 15,  8, 23,  7,  6,  5  };
 
	static const U64 DEBRUIJN64 = U64(0x07EDD5E59A4E28C2);
 
	// here you would get a warming: "unary minus operator applied to unsigned type",
	// that's intended and OK so I'll disable it
#pragma warning (disable: 4146)
	return INDEX64[((bitmap & -bitmap) * DEBRUIJN64) >> 58];
	#elif !defined(_WIN64)
	unsigned long idx;
	if (unsigned (bitmap))
	{
		_BitScanForward(&idx, bitmap);
		return idx;
	} else
	{
		_BitScanForward(&idx, (bitmap >> 32));
		return idx + 32;
	}
#else
	unsigned long idx;
	_BitScanForward64(&idx, bitmap);
	return idx;
	#endif
}
 
inline unsigned int lastOne(U64 bitmap)
{
	#ifndef _MSC_VER
	// this is Eugene Nalimov's bitScanReverse
	// use firstOne if you can, it is faster than lastOne.
	// don't use this if bitmap = 0
 
	int result = 0;
	if (bitmap > 0xFFFFFFFF)
	{
		bitmap >>= 32;
		result = 32;
	}
	if (bitmap > 0xFFFF)
	{
		bitmap >>= 16;
		result += 16;
	}
	if (bitmap > 0xFF)
	{
		bitmap >>= 8;
		result += 8;
	}
	return result + MS1BTABLE[bitmap];
#elif !defined(_WIN64)
	unsigned long idx, b = (bitmap >> 32);
	if (b)
	{
		_BitScanReverse(&idx, b);
		return idx + 32;
	} else
	{
		_BitScanReverse(&idx, unsigned (bitmap));
		return idx;
	}
#else
	unsigned long idx;
	_BitScanReverse64(&idx, bitmap);
	return idx;
#endif
}
 
void displayBitmap(BitMap in)
{
	int i, rank, file;
	char boardc[64];
 
	for (i = 0 ; i < 64 ; i++)
	{
		if (in & BITSET[i]) boardc[i] = '1';
		else boardc[i] = '.';
	}
 
	std::cout << std::endl << "as binary integer:" << std::endl;
      
	for (i = 63 ; i >= 0 ; i--)  std::cout << boardc[i];
	std::cout << std::endl << "  firstOne = " << firstOne(in) << ", lastOne = " << lastOne(in) << ", bitCnt = " << bitCnt(in) << std::endl;
	std::cout << std::endl << std::endl;
 
	if (board.viewRotated)
	{
		std::cout << "   hgfedcba" << std::endl << std::endl;
		for (rank = 1 ; rank <= 8; rank++)
		{
		std::cout << "   ";
		for (file = 8 ; file >= 1; file--)
		{
			std::cout << boardc[BOARDINDEX[file][rank]];
		}
		std::cout << " " << rank << std::endl;
		}
	}
	else
	{
		for (rank = 8 ; rank >= 1; rank--)
		{
			std::cout << " " << rank << " ";
			for (file = 1 ; file <= 8; file++)
			{
				std::cout << boardc[BOARDINDEX[file][rank]];
			}
			std::cout << std::endl;
		}
		std::cout << std::endl << "   abcdefgh" << std::endl;
	}
	std::cout << std::endl;
	return;
}
