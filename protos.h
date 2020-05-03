#ifndef WINGLET_PROTOS_H
#define WINGLET_PROTOS_H

#include "board.h"
#include "move.h"

#ifndef __GNUC__
unsigned int  bitCnt(BitMap);
#else
#define bitCnt __builtin_popcountll
#endif
int           captgen(int);
void          dataInit();
void          displayBitmap(BitMap);
void          displayMove(Move &);
void		  displayPV();
void		  commands();
BOOLTYPE      doIniCommand(const char *);
inline int  firstOne(const U64&);
void          info();
BOOLTYPE      isAttacked(BitMap &, const unsigned char &);
BOOLTYPE isSqAttacked(const int sq, bool fromSide=true);
BOOLTYPE      isOtherKingAttacked();
BOOLTYPE      isOwnKingAttacked();
BOOLTYPE      isValidTextMove(char *, Move &);
inline unsigned int  lastOne(BitMap);
void          makeBlackPromotion(unsigned int, unsigned int &);
void          makeCapture(unsigned int &, unsigned int &);
void          makeMove(Move &);
void          makeWhitePromotion(unsigned int, unsigned int &);
int           movegen(int);
void		  mstostring(U64 dt, char *);
U64           perft(int, int);
BOOLTYPE      readFen(char *, int);
void		  readIniFile();
void          setup();
void          setupFen(char *, char *, char *, char *, int , int );
void		  test(char *);
void		  timeControl();
BOOLTYPE	  toSan(Move &, char *);
void          unmakeBlackPromotion(unsigned int, unsigned int &);
void          unmakeCapture(unsigned int &, unsigned int &);
void          unmakeMove(Move &);
void          unmakeWhitePromotion(unsigned int, unsigned int &);

#endif 
