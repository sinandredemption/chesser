#include <iostream>
#include <time.h>
#include "defines.h"
#include "protos.h"
#include "extglobals.h"
#include "board.h"
//#include "globals.h"
#include "magicmoves.h"

#define maskAToGFiles 0x7F7F7F7F7F7F7F7FULL
#define maskBToHFiles 0xFEFEFEFEFEFEFEFEULL
#define maskAToFFiles 0x3F3F3F3F3F3F3F3FULL
#define maskCToHFiles 0xFCFCFCFCFCFCFCFCULL
/*
// Macro's to define sliding attacks:
#define RANKMOVES(a)       (RANK_ATTACKS[(a)][((board.occupiedSquares & RANKMASK[(a)]) >> RANKSHIFT[(a)])] & targetBitmap)
#define FILEMOVES(a)       (FILE_ATTACKS[(a)][((board.occupiedSquares & FILEMASK[(a)]) * FILEMAGIC[(a)]) >> 57] & targetBitmap)
#define SLIDEA8H1MOVES(a)  (DIAGA8H1_ATTACKS[(a)][((board.occupiedSquares & DIAGA8H1MASK[(a)]) * DIAGA8H1MAGIC[(a)]) >> 57] & targetBitmap)
#define SLIDEA1H8MOVES(a)  (DIAGA1H8_ATTACKS[(a)][((board.occupiedSquares & DIAGA1H8MASK[(a)]) * DIAGA1H8MAGIC[(a)]) >> 57] & targetBitmap)
#define ROOKMOVES(a)       (RANKMOVES(a) | FILEMOVES(a))
#define BISHOPMOVES(a)     (SLIDEA8H1MOVES(a) | SLIDEA1H8MOVES(a))
#define QUEENMOVES(a)      (BISHOPMOVES(a) | ROOKMOVES(a))
*/
int Board::eval()
{
	U64 targetBitmap, attacks;
	U64 pawns = board.whitePawns;
    U64 wPawnAttacks = ((pawns & maskBToHFiles) << 7) |
                   ((pawns & maskAToGFiles) << 9);
	pawns = board.blackPawns;
    U64 bPawnAttacks = ((pawns & maskBToHFiles) >> 9) |
                   ((pawns & maskAToGFiles) >> 7);
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// This is Chesser's evaluation function
	// The score is calculated from White's perspective (in centipawns)
	// and returned from the perspective of the side to move.
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	int score, square;
	int whitepawns, whiteknights, whitebishops, whiterooks, whitequeens;
	int blackpawns, blackknights, blackbishops, blackrooks, blackqueens;
	int whitekingsquare, blackkingsquare;
	int whitetotalmat, blacktotalmat;
	int whitetotal, blacktotal;
	BOOLTYPE endgame;
	BitMap temp, whitepassedpawns, blackpassedpawns;

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Material
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	score = board.Material;
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> MATERIAL>                       " << board.Material << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Remember where the kings are
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//whitekingsquare = firstOne(board.whiteKing);
	//whitekingsquare = onlyOne(board.whiteKing);
	whitekingsquare = board.wkingsq;
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> WHITE KING POSITION>            " << SQUARENAME[whitekingsquare] << std::endl;
#endif
	//blackkingsquare = firstOne(board.blackKing);
	//blackkingsquare = onlyOne(board.blackKing);
	blackkingsquare = board.bkingsq;
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> BLACK KING POSITION>            " << SQUARENAME[blackkingsquare] << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Piece counts, note that we could have done this incrementally in (un)makeMove
	// because it's basically the same thing as keeping board.Material up to date..
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	whitepawns = bitCnt(board.whitePawns);
	whiteknights = bitCnt(board.whiteKnights);
	whitebishops = bitCnt(board.whiteBishops);
	whiterooks = bitCnt(board.whiteRooks);
	whitequeens = bitCnt(board.whiteQueens);
	whitetotalmat = 3 * whiteknights + 3 * whitebishops + 5 * whiterooks + 9 * whitequeens;
	whitetotal = whitepawns + whiteknights + whitebishops + whiterooks + whitequeens;
	blackpawns = bitCnt(board.blackPawns);
	blackknights = bitCnt(board.blackKnights);
	blackbishops = bitCnt(board.blackBishops);
	blackrooks = bitCnt(board.blackRooks);
	blackqueens = bitCnt(board.blackQueens);
	blacktotalmat = 3 * blackknights + 3 * blackbishops + 5 * blackrooks + 10 * blackqueens;
	blacktotal = blackpawns + blackknights + blackbishops + blackrooks + blackqueens;
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> WHITE TOTAL MAT>                " << whitetotalmat << std::endl;
	std::cout << "EVAL> BLACK TOTAL MAT>                " << blacktotalmat << std::endl;
	std::cout << "EVAL> WHITE TOTAL>                    " << whitetotal << std::endl;
	std::cout << "EVAL> BLACK TOTAL>                    " << blacktotal << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Check if we are in the endgame
	// Anything less than a queen (=10) + rook (=5) is considered endgame
	// (pawns excluded in this count)
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	endgame = (whitetotalmat < 15 || blacktotalmat < 15);
#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> ENDGAME>                        " << endgame << std::endl;
#endif

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate for draws due to insufficient material:
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (!whitepawns && !blackpawns)
	{
		// king versus king:
		if ((whitetotalmat == 0) && (blacktotalmat == 0)) 
		{
			if (board.nextMove) return -DRAWSCORE;
			else return DRAWSCORE;
		}

		// king and knight versus king:
		else if (((whitetotalmat == 3) && (whiteknights == 1) && (blacktotalmat == 0)) ||
			((blacktotalmat == 3) && (blackknights == 1) && (whitetotalmat == 0))) 
		{
			if (board.nextMove) return -DRAWSCORE;
			else return DRAWSCORE;
		}

		// 2 kings with one or more bishops, and all bishops on the same colour:
		else if ((whitebishops + blackbishops) > 0)
		{
			if ((whiteknights == 0) && (whiterooks == 0) && (whitequeens == 0) &&
				(blackknights == 0) && (blackrooks == 0) && (blackqueens == 0))
			{
				if (!((board.whiteBishops | board.blackBishops) & WHITE_SQUARES) ||
					!((board.whiteBishops | board.blackBishops) & BLACK_SQUARES))
				{
					if (board.nextMove) return -DRAWSCORE;
					else return DRAWSCORE;
				}
			}
		}
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate MATERIAL
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Have the winning side prefer to exchange pieces
	// Every exchange with unequal material adds 3 centipawns to the score
	// Loosing a piece (from balanced material) becomes more
	// severe in the endgame
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WINGLET_VERBOSE_EVAL
	int iexch = 0;
	int iwhitepos = 0;
	int iwhitepawns = 0;
	int iwhiteking = 0;
	int iblackpos = 0;
	int iblackpawns = 0;
	int iblackking = 0;
#endif

	if (whitetotalmat + whitepawns > blacktotalmat + blackpawns)
	{
		score += 45 + 3 * whitetotal - 6 * blacktotal;
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> EXCHANGE WHITE>                 " << (45 + 3 * whitetotal - 6 * blacktotal) << std::endl;
		iexch = (45 + 3 * whitetotal - 6 * blacktotal);
#endif
	}
	else
	{
		if (whitetotalmat + whitepawns < blacktotalmat + blackpawns)
		{
			score += -45 - 3 * blacktotal + 6 * whitetotal;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL> EXCHANGE BLACK>                 " << (-45 - 3 * blacktotal + 6 * whitetotal) << std::endl;
			iexch = (-45 - 3 * blacktotal + 6 * whitetotal);
#endif
		}
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate WHITE PIECES
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	targetBitmap = ~(board.whitePieces); // Used in Mobility evaluation

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white pawns
	// - position on the board
	// - distance from opponent king
	// - distance from own king
	// - passed, doubled, isolated or backward pawns
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	whitepassedpawns = 0;
	temp = board.whitePawns;
	while (temp)
	{
		square = firstOne(temp);

#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE PAWN ON                   " << SQUARENAME[square] << std::endl;
#endif
		// Original code
		//		score += PAWNPOS_W[square];
		if(endgame)
		{
			score += PAWNPOS_W_EG[square]+PAWN_OWN_DISTANCE[DISTANCE[square][whitekingsquare]];
		}
		else
			score += PAWNPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << PAWNPOS_W[square] << std::endl;
		iwhitepos += PAWNPOS_W[square];
		iwhitepawns += PAWNPOS_W[square];
#endif

		score += PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]];
		iwhitepawns += PAWN_OPPONENT_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		if (!(PASSED_WHITE[square] & board.blackPawns))
		{
			score += BONUS_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS PASSED, BONUS IS            " << BONUS_PASSED_PAWN << std::endl;
			iwhitepos += BONUS_PASSED_PAWN;
			iwhitepawns += BONUS_PASSED_PAWN;
#endif

			// remember its location, we need it later when evaluating the white rooks:
		}

		if ((board.whitePawns ^ BITSET[square]) & FILEMASK[square])
		{
			if(!endgame)
				score -= PENALTY_DOUBLED_PAWN;
			else
				score -= PENALTY_DOUBLED_PAWN_EG;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS DOUBLED, PENALTY IS         " << PENALTY_DOUBLED_PAWN << std::endl;
			iwhitepos -= PENALTY_DOUBLED_PAWN;
			iwhitepawns -= PENALTY_DOUBLED_PAWN;
#endif
		}

		if (!(ISOLATED_WHITE[square] & board.whitePawns))
		{
			if(!endgame)
				score -= PENALTY_ISOLATED_PAWN;
			else
				score -= PENALTY_ISOLATED_PAWN_EG;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS ISOLATED, PENALTY IS        " << PENALTY_ISOLATED_PAWN << std::endl;
			iwhitepos -= PENALTY_ISOLATED_PAWN;
			iwhitepawns -= PENALTY_ISOLATED_PAWN;
#endif
		}
		else
		{
			// If it is not isolated, then it might be backward. Two conditions must be true:
			//  1) if the next square is controlled by an enemy pawn - we use the PAWN_ATTACKS bitmaps to check this
			//  2) if there are no pawns left that could defend this pawn
			if ((WHITE_PAWN_ATTACKS[square + 8] & board.blackPawns))
			{
				if (!(BACKWARD_WHITE[square] & board.whitePawns))
				{
					score -= PENALTY_BACKWARD_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
					std::cout << "EVAL>  IS BACKWARD, PENALTY IS        " << PENALTY_BACKWARD_PAWN << std::endl;
					iwhitepos -= PENALTY_BACKWARD_PAWN;
					iwhitepawns -= PENALTY_BACKWARD_PAWN;
#endif
				}
			}
		}

		temp &= (temp - 1);
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white knights
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.whiteKnights;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE KNIGHT ON                 " << SQUARENAME[square] << std::endl;
#endif
		// Original Code:
		//		score += KNIGHTPOS_W[square];
		if(endgame)
			score += KNIGHTPOS_W_EG[square];
		else 
			score += KNIGHTPOS_W[square];

#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KNIGHTPOS_W[square] << std::endl;
		iwhitepos += KNIGHTPOS_W[square];
#endif

		score += KNIGHT_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << KNIGHT_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += KNIGHT_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		temp &= (temp - 1);
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white bishops
	// - having the pair
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (board.whiteBishops)
	{
		if (whitebishops > 1)
			score += BONUS_BISHOP_PAIR;

		temp = board.whiteBishops;
		while (temp)
		{
			square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL> WHITE BISHOP ON                 " << SQUARENAME[square] << std::endl;
#endif
			if(endgame)
				score += BISHOPPOS_W_EG[square];
			else
				score += BISHOPPOS_W[square];
			// Added 16-09-2014 by Syed Fahad
			//
			// Evaluate Mobility:
//			attacks = BISHOPMOVES(square);
			attacks = Bmagic(square, board.occupiedSquares) & targetBitmap;
			score += bishopMobScore[bitCnt(attacks & ~(board.whitePieces | bPawnAttacks))];

#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  POSITION SCORE IS              " << BISHOPPOS_W[square] << std::endl;
			iwhitepos += BISHOPPOS_W[square];
#endif

			score += BISHOP_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << BISHOP_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
			iwhitepos += BISHOP_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

			temp &= (temp - 1);
		}
	}



	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white rooks
	// - position on the board
	// - distance from opponent king
	// - on the same file as a passed pawn
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.whiteRooks;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE ROOK ON                   " << SQUARENAME[square] << std::endl;
#endif

		score += ROOKPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << ROOKPOS_W[square] << std::endl;
		iwhitepos += ROOKPOS_W[square];
#endif

		score += ROOK_DISTANCE[DISTANCE[square][blackkingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << ROOK_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += ROOK_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif
			// Added 16-09-2014 by Syed Fahad
			//
			// Evaluate Mobility:
//			attacks = ROOKMOVES(square);
		attacks = Rmagic(square, board.occupiedSquares) & targetBitmap;
			score += rookMobScore[bitCnt(attacks & ~(board.whitePieces | bPawnAttacks))];

		if (FILEMASK[square] & whitepassedpawns)
		{
			if ((unsigned int) square < lastOne(FILEMASK[square] & whitepassedpawns))
			{
				score += BONUS_ROOK_BEHIND_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
				std::cout << "EVAL>  BEHIND PASSED PAWN BONUS       " << BONUS_ROOK_BEHIND_PASSED_PAWN << std::endl;
				iwhitepos += BONUS_ROOK_BEHIND_PASSED_PAWN;
#endif
			}
		}
		temp &= (temp - 1);
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate white queens
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.whiteQueens;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE QUEEN ON                  " << SQUARENAME[square] << std::endl;
#endif

		score += QUEENPOS_W[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << QUEENPOS_W[square] << std::endl;
		iwhitepos += QUEENPOS_W[square];
#endif

		score += QUEEN_DISTANCE[DISTANCE[square][blackkingsquare]];

		// Added 16-09-2014 by Syed Fahad
		//
		// Evaluate Mobility:
//		attacks = QUEENMOVES(square);
		attacks = Qmagic(square, board.occupiedSquares) & targetBitmap;
		score += queenMobScore[bitCnt(attacks & ~(board.whitePieces | bPawnAttacks))];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << QUEEN_DISTANCE[DISTANCE[square][blackkingsquare]] << std::endl;
		iwhitepos += QUEEN_DISTANCE[DISTANCE[square][blackkingsquare]];
#endif

		temp &= (temp - 1);
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate the white king
	// - position on the board
	// - proximity to the pawns
	// - pawn shield (not in the endgame)
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (endgame)
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE KING ON (endgame)         " << SQUARENAME[whitekingsquare] << std::endl;
#endif

		score += KINGPOS_ENDGAME_W[whitekingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_ENDGAME_W[whitekingsquare] << std::endl;
		iwhitepos += KINGPOS_ENDGAME_W[whitekingsquare];
		iwhiteking += KINGPOS_ENDGAME_W[whitekingsquare];
#endif

	}
	else
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> WHITE KING ON (no endgame)      " << SQUARENAME[whitekingsquare] << std::endl;
#endif

		score += KINGPOS_W[whitekingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_W[whitekingsquare] << std::endl;
		iwhitepos += KINGPOS_W[whitekingsquare];
		iwhiteking += KINGPOS_W[whitekingsquare];
#endif

		// add pawn shield bonus if we're not in the endgame:
		// strong pawn shield bonus if the pawns are near the king:
		score += BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  STRONG PAWN SHIELD SCORE IS    " << BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns) << std::endl;
		iwhitepos += BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns);
		iwhiteking += BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_W[whitekingsquare] & board.whitePawns);
#endif

		// weaker pawn shield bonus if the pawns are not so near the king:
		score += BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  WEAK PAWN SHIELD SCORE IS      " << BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns) << std::endl;
		iwhitepos += BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns);
		iwhiteking += BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_W[whitekingsquare] & board.whitePawns);
#endif

	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate BLACK PIECES
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	targetBitmap = ~(board.blackPieces);

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black pawns
	// - position on the board
	// - distance from opponent king
	// - distance from own king
	// - passed, doubled, isolated or backward pawns
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	blackpassedpawns = 0;
	temp = board.blackPawns;
	while (temp)
	{
		square = firstOne(temp);

#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK PAWN ON                   " << SQUARENAME[square] << std::endl;
#endif
		if(endgame)
		{
			score -= (PAWNPOS_B_EG[square]+PAWN_OWN_DISTANCE[DISTANCE[square][blackkingsquare]]);
		}
		else
			score -= PAWNPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << PAWNPOS_B[square] << std::endl;
		iblackpos -= PAWNPOS_B[square];
		iblackpawns -= PAWNPOS_B[square];
#endif

		score -= PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]];
		iblackpawns -= PAWN_OPPONENT_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif


		if (!(PASSED_BLACK[square] & board.whitePawns))
		{
			score -= BONUS_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS PASSED, BONUS IS            " << BONUS_PASSED_PAWN << std::endl;
			iblackpos -= BONUS_PASSED_PAWN;
			iblackpawns -= BONUS_PASSED_PAWN;
#endif

			// remember its location, we need it later when evaluating the black rooks:
			blackpassedpawns &= (temp - 1);

		}

		if ((board.blackPawns ^ BITSET[square]) & FILEMASK[square])
		{
			score += PENALTY_DOUBLED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS DOUBLED, PENALTY IS         " << PENALTY_DOUBLED_PAWN << std::endl;
			iblackpos += PENALTY_DOUBLED_PAWN;
			iblackpawns += PENALTY_DOUBLED_PAWN;
#endif
		}

		if (!(ISOLATED_BLACK[square] & board.blackPawns))
		{
			score += PENALTY_ISOLATED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
			std::cout << "EVAL>  IS ISOLATED, PENALTY IS        " << PENALTY_ISOLATED_PAWN << std::endl;
			iblackpos += PENALTY_ISOLATED_PAWN;
			iblackpawns += PENALTY_ISOLATED_PAWN;
#endif
		}
		else
		{
			// If it is not isolated, then it might be backward. Two conditions must be true:
			//  1) if the next square is controlled by an enemy pawn - we use the PAWN_ATTACKS bitmaps to check this
			//  2) if there are no pawns left that could defend this pawn
			if ((BLACK_PAWN_ATTACKS[square - 8] & board.whitePawns))
			{
				if (!(BACKWARD_BLACK[square] & board.blackPawns))
				{
					score += PENALTY_BACKWARD_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
					std::cout << "EVAL>  IS BACKWARD, PENALTY IS        " << PENALTY_BACKWARD_PAWN << std::endl;
					iwhitepos += PENALTY_BACKWARD_PAWN;
					iwhitepawns += PENALTY_BACKWARD_PAWN;
#endif
				}
			}
		}

		temp &= (temp - 1);

	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black knights
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.blackKnights;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK KNIGHT ON                 " << SQUARENAME[square] << std::endl;
#endif
		if(endgame)
			score -= KNIGHTPOS_B_EG[square];
		else
			score -= KNIGHTPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KNIGHTPOS_B[square] << std::endl;
		iblackpos -= KNIGHTPOS_B[square];
#endif

		score -= KNIGHT_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << KNIGHT_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= KNIGHT_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		temp &= (temp - 1);
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black bishops
	// - having the pair
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (board.blackBishops)
	{
		if (blackbishops > 1)
			score -= BONUS_BISHOP_PAIR;

		temp = board.blackBishops;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK BISHOP ON                 " << SQUARENAME[square] << std::endl;
#endif
		if(endgame)
			score -= BISHOPPOS_B_EG[square];
		else
			score -= BISHOPPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << BISHOPPOS_B[square] << std::endl;
		iblackpos -= BISHOPPOS_B[square];
#endif

		// Added 16-09-2014 by Syed Fahad
		//
		// Evaluate Mobility:
//		attacks = BISHOPMOVES(square);
		attacks = Bmagic(square, board.occupiedSquares) & targetBitmap;
		score -= bishopMobScore[bitCnt(attacks & ~(board.blackPieces | wPawnAttacks))];

		score -= BISHOP_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << BISHOP_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= BISHOP_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		temp &= (temp - 1);
	}
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black rooks
	// - position on the board
	// - distance from opponent king
	// - on the same file as a passed pawn
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.blackRooks;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK ROOK ON                   " << SQUARENAME[square] << std::endl;
#endif

		score -= ROOKPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << ROOKPOS_B[square] << std::endl;
		iblackpos -= ROOKPOS_B[square];
#endif

		score -= ROOK_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << ROOK_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= ROOK_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		// Added 16-09-2014 by Syed Fahad
		//
		// Evaluate Mobility:
//		attacks = ROOKMOVES(square);
		attacks = Rmagic(square, board.occupiedSquares) & targetBitmap;
		score -= rookMobScore[bitCnt(attacks & ~(board.blackPieces | wPawnAttacks))];

		if (FILEMASK[square] & blackpassedpawns)
		{
			if (square > firstOne(FILEMASK[square] & blackpassedpawns))
			{
				score -= BONUS_ROOK_BEHIND_PASSED_PAWN;
#ifdef WINGLET_VERBOSE_EVAL
				std::cout << "EVAL>  BEHIND PASSED PAWN BONUS       " << BONUS_ROOK_BEHIND_PASSED_PAWN << std::endl;
				iblackpos -= BONUS_ROOK_BEHIND_PASSED_PAWN;
#endif
			}
		}
		temp &= (temp - 1);
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate black queens
	// - position on the board
	// - distance from opponent king
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	temp = board.blackQueens;
	while (temp)
	{
		square = firstOne(temp);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK QUEEN ON                  " << SQUARENAME[square] << std::endl;
#endif

		score -= QUEENPOS_B[square];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << QUEENPOS_B[square] << std::endl;
		iblackpos -= QUEENPOS_B[square];
#endif

		// Added 16-09-2014 by Syed Fahad
		//
		// Evaluate Mobility:
//		attacks = QUEENMOVES(square);
		attacks = Qmagic(square, board.occupiedSquares) & targetBitmap;
		score -= queenMobScore[bitCnt(attacks & ~(board.blackPieces | wPawnAttacks))];

		score -= QUEEN_DISTANCE[DISTANCE[square][whitekingsquare]];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  DISTANCE TO OPPONNT KING SCORE " << QUEEN_DISTANCE[DISTANCE[square][whitekingsquare]] << std::endl;
		iblackpos -= QUEEN_DISTANCE[DISTANCE[square][whitekingsquare]];
#endif

		temp &= (temp - 1);
	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Evaluate the black king
	// - position on the board
	// - proximity to the pawns
	// - pawn shield (not in the endgame)
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (endgame)
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK KING ON (endgame)         " << SQUARENAME[blackkingsquare] << std::endl;
#endif

		score -= KINGPOS_ENDGAME_B[blackkingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_ENDGAME_B[blackkingsquare] << std::endl;
		iblackpos -= KINGPOS_ENDGAME_B[blackkingsquare];
		iblackking -= KINGPOS_ENDGAME_B[blackkingsquare];
#endif
	}
	else
	{
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL> BLACK KING ON (no endgame)      " << SQUARENAME[blackkingsquare] << std::endl;
#endif

		score -= KINGPOS_B[blackkingsquare];
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  POSITION SCORE IS              " << KINGPOS_B[blackkingsquare] << std::endl;
		iblackpos -= KINGPOS_B[blackkingsquare];
		iblackking -= KINGPOS_B[blackkingsquare];
#endif

		// add pawn shield bonus if we're not in the endgame:
		// strong pawn shield bonus if the pawns are near the king:
		score -= BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  STRONG PAWN SHIELD SCORE IS    " << BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns) << std::endl;
		iblackpos -= BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns);
		iblackking -= BONUS_PAWN_SHIELD_STRONG * bitCnt(KINGSHIELD_STRONG_B[blackkingsquare] & board.blackPawns);
#endif

		// weaker pawn shield bonus if the pawns are not so near the king:
		score -= BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns);
#ifdef WINGLET_VERBOSE_EVAL
		std::cout << "EVAL>  WEAK PAWN SHIELD SCORE IS      " << BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns) << std::endl;
		iblackpos -= BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns);
		iblackking -= BONUS_PAWN_SHIELD_WEAK * bitCnt(KINGSHIELD_WEAK_B[blackkingsquare] & board.blackPawns);
#endif

	}

	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Return the score
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WINGLET_VERBOSE_EVAL
	std::cout << "EVAL> SUMMARY:" << std::endl;
	std::cout << "EVAL>  MATERIAL                :" << board.Material << std::endl;
	std::cout << "EVAL>  UNEVEN MATERIAL BONUS   :" << iexch << std::endl;
	std::cout << "EVAL>  WHITE POSITIONAL BONUS  :" << iwhitepos << std::endl;
	std::cout << "EVAL>     WHITE PAWNS BONUS    :" << iwhitepawns << std::endl;
	std::cout << "EVAL>     WHITE KING POS/SAF   :" << iwhiteking << std::endl;
	std::cout << "EVAL>  BLACK POSITIONAL BONUS  :" << iblackpos << std::endl;
	std::cout << "EVAL>     BLACK PAWNS BONUS    :" << iblackpawns << std::endl;
	std::cout << "EVAL>     BLACK KING POS/SAF   :" << iblackking << std::endl;
#endif

	if (board.nextMove) return -score;
	else return score;

}