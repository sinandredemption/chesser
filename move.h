#ifndef WINGLET_MOVE_H_
#define WINGLET_MOVE_H_
#include <cassert>
#include "defines.h"

//  There are at least 3 different ways to store a move in max 32 bits
//  1) using shift & rank in an unsigned int                
//  2) using 4 unsigned chars, union-ed with an unsigned int
//  3) using C++ bitfields, union-ed with an unsigned int   

//  this is 1) using shift & rank in an unsigned int (32 bit):
struct Move                                  
{
	// from (6 bits)
	// tosq (6 bits)
	// piec (4 bits)
	// capt (4 bits)
	// prom (4 bits)
	int moveInt;
	void make_move(const int from, const int to, const int piece, const int capt = 0, const int prom = 0)
	{
		setFrom(from);
		setTosq(to);
		setPiec(piece);
		setCapt(capt);
		setProm(prom);
	}
	inline void clear()
	{
		moveInt = 0;
	}

	inline void setFrom(unsigned int from)  
	{   // bits  0.. 5
    assert(from <= 0x0000003f);
//		moveInt &= 0xffffffc0; moveInt |= (from & 0x0000003f);
    moveInt &= 0xffffffc0; moveInt |= (from);
	}

	inline void setTosq(unsigned int to)  
	{   // bits  6..11
    
    assert(to <= 0x0000003f);
		moveInt &= 0xfffff03f; moveInt |= (to) << 6;
	}

	inline void setPiec(unsigned int piec)  
	{   // bits 12..15
    assert(piec <= 0xf);
		moveInt &= 0xffff0fff; moveInt |= (piec) << 12;
	} 

	inline void setCapt(unsigned int capt)  
	{   // bits 16..19
    assert(capt <= 0xf);
		moveInt &= 0xfff0ffff; moveInt |= (capt) << 16;
	} 

	inline void setProm(unsigned int prom)  
	{   // bits 20..23
    assert(prom <= 0xf);
		moveInt &= 0xff0fffff; moveInt |= (prom) << 20;
	} 

	// read move information:
	// first shift right, then mask to get the info

	inline unsigned int getFrom()  
	{   // 6 bits (value 0..63), position  0.. 5
		return (moveInt        & 0x0000003f);
	}  

	inline unsigned int getTosq()  
	{   // 6 bits (value 0..63), position  6..11
		return (moveInt >>  6) & 0x0000003f; 
	}   

	inline unsigned int getPiec()  
	{   // 4 bits (value 0..15), position 12..15
		return (moveInt >> 12) & 0x0000000f; 
	}   

	inline unsigned int getCapt()  
	{   // 4 bits (value 0..15), position 16..19
		return (moveInt >> 16) & 0x0000000f; 
	}   

	inline unsigned int getProm()  
	{   // 4 bits (value 0..15), position 20..23
		return (moveInt >> 20) & 0x0000000f; 
	}   

	// boolean checks for some types of moves.
	// first mask, then compare
	// Note that we are using the bit-wise properties of piece identifiers, so we cannot just change them anymore !

	inline BOOLTYPE isWhitemove()  
	{   // piec is white: bit 15 must be 0
		return (~moveInt & 0x00008000) == 0x00008000;
	} 

	inline BOOLTYPE isBlackmove()  
	{   // piec is black: bit 15 must be 1
		return ( moveInt & 0x00008000) == 0x00008000;
	} 

	inline BOOLTYPE isCapture()    
	{   // capt is nonzero, bits 16 to 19 must be nonzero
		return ( moveInt & 0x000f0000) != 0x00000000;
	} 

	inline BOOLTYPE isKingcaptured()
	{   // bits 17 to 19 must be 010
		return ( moveInt & 0x00070000) == 0x00020000;
	} 

	inline BOOLTYPE isRookmove()
	{   // bits 13 to 15 must be 110
		return ( moveInt & 0x00007000) == 0x00006000;
	} 

	inline BOOLTYPE isRookcaptured()
	{   // bits 17 to 19 must be 110
		return ( moveInt & 0x00070000) == 0x00060000;
	} 

	inline BOOLTYPE isKingmove()
	{   // bits 13 to 15 must be 010
		return ( moveInt & 0x00007000) == 0x00002000;
	} 

	inline BOOLTYPE isPawnmove()
	{   // bits 13 to 15 must be 001
		return ( moveInt & 0x00007000) == 0x00001000;
	} 

	inline BOOLTYPE isPawnDoublemove()
	{   // bits 13 to 15 must be 001 &
		//     bits 4 to 6 must be 001 (from rank 2) & bits 10 to 12 must be 011 (to rank 4)
		// OR: bits 4 to 6 must be 110 (from rank 7) & bits 10 to 12 must be 100 (to rank 5)

		return ((( moveInt & 0x00007000) == 0x00001000) && (((( moveInt & 0x00000038) == 0x00000008) && ((( moveInt & 0x00000e00) == 0x00000600))) || 
			((( moveInt & 0x00000038) == 0x00000030) && ((( moveInt & 0x00000e00) == 0x00000800)))));
	} 

	inline BOOLTYPE isEnpassant()  
	{   // prom is a pawn, bits 21 to 23 must be 001
		return ( moveInt & 0x00700000) == 0x00100000;
	} 

	inline BOOLTYPE isPromotion()  
	{   // prom (with color bit removed), .xxx > 2 (not king or pawn)
		return ( moveInt & 0x00700000) >  0x00200000;
	} 

	inline BOOLTYPE isCastle()     
	{   // prom is a king, bits 21 to 23 must be 010
		return ( moveInt & 0x00700000) == 0x00200000;
	} 

	inline BOOLTYPE isCastleOO()   
	{   // prom is a king and tosq is on the g-file
		return ( moveInt & 0x007001c0) == 0x00200180;
	} 

	inline BOOLTYPE isCastleOOO()  
	{   // prom is a king and tosq is on the c-file
		return ( moveInt & 0x007001c0) == 0x00200080;
	} 

};

#endif
