#pragma once

#include "CoreMinimal.h"
#include "Common/E_ObjectType.h"

// Plain chess rules on an 8x8 board, used by the server to generate and validate moves.
// Columns are the letters A..H (1..8) and rows are the numbers 1..8, like the slot names ("E2").
// HOME (IsAway == false) starts on rows 1-2 and moves up; AWAY starts on rows 7-8 and moves down.

enum class EMCMoveKind : uint8
{
	Normal,
	DoublePawnStep,
	EnPassant,
	CastleKingSide,
	CastleQueenSide
};

struct FMCMove
{
	int32 FromCol = 0;
	int32 FromRow = 0;
	int32 ToCol = 0;
	int32 ToRow = 0;
	EMCMoveKind Kind = EMCMoveKind::Normal;
	bool bCapture = false;
	bool bPromotion = false;
};

struct FMCBoardPiece
{
	bool bValid = false;
	E_ObjectType Type = E_ObjectType::Pawn;
	bool bAway = false;
	bool bMoved = false;
};

class MULTIPLAYERCHESS_API FMCChessRules
{
public:
	// Board[Col][Row], both 1..8 (index 0 unused).
	FMCBoardPiece Board[9][9];

	// Square a pawn can capture onto en passant (the square it skipped), or 0 when none.
	int32 EnPassantCol = 0;
	int32 EnPassantRow = 0;

	static bool ParseSquare(FName SquareName, int32& OutCol, int32& OutRow);
	static FName MakeSquareName(int32 Col, int32 Row);
	static bool IsOnBoard(int32 Col, int32 Row) { return Col >= 1 && Col <= 8 && Row >= 1 && Row <= 8; }

	void SetPiece(int32 Col, int32 Row, E_ObjectType Type, bool bAway, bool bMoved);

	// Moves of the piece on the square that do not leave its own king in check.
	void GenerateLegalMoves(int32 Col, int32 Row, TArray<FMCMove>& OutMoves) const;

	// The legal move from one square to another, if there is one.
	bool FindLegalMove(int32 FromCol, int32 FromRow, int32 ToCol, int32 ToRow, FMCMove& OutMove) const;

	bool IsSquareAttacked(int32 Col, int32 Row, bool bByAway) const;
	bool IsInCheck(bool bAway) const;
	bool HasAnyLegalMove(bool bAway) const;

	// Applies the move to this board (used to test moves for check).
	void ApplyMove(const FMCMove& Move);

private:
	void GeneratePseudoMoves(int32 Col, int32 Row, TArray<FMCMove>& OutMoves) const;
	void AddSlidingMoves(int32 Col, int32 Row, const int32 (*Directions)[2], int32 NumDirections, int32 MaxSteps, TArray<FMCMove>& OutMoves) const;
	void AddMoveIfPossible(int32 Col, int32 Row, int32 ToCol, int32 ToRow, TArray<FMCMove>& OutMoves) const;
	bool FindKing(bool bAway, int32& OutCol, int32& OutRow) const;
};
