#include "Game/MCChessRules.h"

namespace
{
	const int32 RookDirections[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	const int32 BishopDirections[4][2] = { { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
	const int32 QueenDirections[8][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
	const int32 KnightJumps[8][2] = { { 1, 2 }, { 2, 1 }, { 2, -1 }, { 1, -2 }, { -1, -2 }, { -2, -1 }, { -2, 1 }, { -1, 2 } };

	int32 PawnDirection(bool bAway) { return bAway ? -1 : 1; }
	int32 PawnStartRow(bool bAway) { return bAway ? 7 : 2; }
	int32 PromotionRow(bool bAway) { return bAway ? 1 : 8; }
	int32 HomeRow(bool bAway) { return bAway ? 8 : 1; }
}

bool FMCChessRules::ParseSquare(FName SquareName, int32& OutCol, int32& OutRow)
{
	const FString Name = SquareName.ToString();
	if (Name.Len() != 2)
	{
		return false;
	}
	OutCol = FChar::ToUpper(Name[0]) - TEXT('A') + 1;
	OutRow = Name[1] - TEXT('0');
	return IsOnBoard(OutCol, OutRow);
}

FName FMCChessRules::MakeSquareName(int32 Col, int32 Row)
{
	if (!IsOnBoard(Col, Row))
	{
		return NAME_None;
	}
	const TCHAR Chars[3] = { static_cast<TCHAR>(TEXT('A') + Col - 1), static_cast<TCHAR>(TEXT('0') + Row), 0 };
	return FName(Chars);
}

void FMCChessRules::SetPiece(int32 Col, int32 Row, E_ObjectType Type, bool bAway, bool bMoved)
{
	if (IsOnBoard(Col, Row))
	{
		FMCBoardPiece& Piece = Board[Col][Row];
		Piece.bValid = true;
		Piece.Type = Type;
		Piece.bAway = bAway;
		Piece.bMoved = bMoved;
	}
}

void FMCChessRules::AddMoveIfPossible(int32 Col, int32 Row, int32 ToCol, int32 ToRow, TArray<FMCMove>& OutMoves) const
{
	if (!IsOnBoard(ToCol, ToRow))
	{
		return;
	}
	const FMCBoardPiece& Target = Board[ToCol][ToRow];
	if (Target.bValid && Target.bAway == Board[Col][Row].bAway)
	{
		return;
	}
	FMCMove Move;
	Move.FromCol = Col;
	Move.FromRow = Row;
	Move.ToCol = ToCol;
	Move.ToRow = ToRow;
	Move.bCapture = Target.bValid;
	OutMoves.Add(Move);
}

void FMCChessRules::AddSlidingMoves(int32 Col, int32 Row, const int32 (*Directions)[2], int32 NumDirections, int32 MaxSteps, TArray<FMCMove>& OutMoves) const
{
	for (int32 Dir = 0; Dir < NumDirections; ++Dir)
	{
		for (int32 Step = 1; Step <= MaxSteps; ++Step)
		{
			const int32 ToCol = Col + Directions[Dir][0] * Step;
			const int32 ToRow = Row + Directions[Dir][1] * Step;
			if (!IsOnBoard(ToCol, ToRow))
			{
				break;
			}
			AddMoveIfPossible(Col, Row, ToCol, ToRow, OutMoves);
			if (Board[ToCol][ToRow].bValid)
			{
				break;
			}
		}
	}
}

void FMCChessRules::GeneratePseudoMoves(int32 Col, int32 Row, TArray<FMCMove>& OutMoves) const
{
	if (!IsOnBoard(Col, Row) || !Board[Col][Row].bValid)
	{
		return;
	}
	const FMCBoardPiece& Piece = Board[Col][Row];

	switch (Piece.Type)
	{
	case E_ObjectType::Pawn:
	{
		const int32 Dir = PawnDirection(Piece.bAway);
		const int32 OneRow = Row + Dir;
		if (IsOnBoard(Col, OneRow) && !Board[Col][OneRow].bValid)
		{
			FMCMove Move;
			Move.FromCol = Col; Move.FromRow = Row; Move.ToCol = Col; Move.ToRow = OneRow;
			Move.bPromotion = OneRow == PromotionRow(Piece.bAway);
			OutMoves.Add(Move);

			const int32 TwoRow = Row + 2 * Dir;
			if (Row == PawnStartRow(Piece.bAway) && IsOnBoard(Col, TwoRow) && !Board[Col][TwoRow].bValid)
			{
				FMCMove Double;
				Double.FromCol = Col; Double.FromRow = Row; Double.ToCol = Col; Double.ToRow = TwoRow;
				Double.Kind = EMCMoveKind::DoublePawnStep;
				OutMoves.Add(Double);
			}
		}
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			const int32 ToCol = Col + Side;
			if (!IsOnBoard(ToCol, OneRow))
			{
				continue;
			}
			const FMCBoardPiece& Target = Board[ToCol][OneRow];
			if (Target.bValid && Target.bAway != Piece.bAway)
			{
				FMCMove Capture;
				Capture.FromCol = Col; Capture.FromRow = Row; Capture.ToCol = ToCol; Capture.ToRow = OneRow;
				Capture.bCapture = true;
				Capture.bPromotion = OneRow == PromotionRow(Piece.bAway);
				OutMoves.Add(Capture);
			}
			else if (!Target.bValid && ToCol == EnPassantCol && OneRow == EnPassantRow)
			{
				FMCMove EnPassant;
				EnPassant.FromCol = Col; EnPassant.FromRow = Row; EnPassant.ToCol = ToCol; EnPassant.ToRow = OneRow;
				EnPassant.Kind = EMCMoveKind::EnPassant;
				EnPassant.bCapture = true;
				OutMoves.Add(EnPassant);
			}
		}
		break;
	}
	case E_ObjectType::Knight:
		for (const auto& Jump : KnightJumps)
		{
			AddMoveIfPossible(Col, Row, Col + Jump[0], Row + Jump[1], OutMoves);
		}
		break;
	case E_ObjectType::Bishop:
		AddSlidingMoves(Col, Row, BishopDirections, 4, 7, OutMoves);
		break;
	case E_ObjectType::Rook:
		AddSlidingMoves(Col, Row, RookDirections, 4, 7, OutMoves);
		break;
	case E_ObjectType::Queen:
		AddSlidingMoves(Col, Row, QueenDirections, 8, 7, OutMoves);
		break;
	case E_ObjectType::King:
	{
		AddSlidingMoves(Col, Row, QueenDirections, 8, 1, OutMoves);

		// Castling: king and rook unmoved on their home squares, nothing in between, and the king is not in
		// check, does not pass through an attacked square and does not land on one.
		const int32 Home = HomeRow(Piece.bAway);
		if (!Piece.bMoved && Col == 5 && Row == Home && !IsSquareAttacked(5, Home, !Piece.bAway))
		{
			const FMCBoardPiece& KingSideRook = Board[8][Home];
			if (KingSideRook.bValid && KingSideRook.Type == E_ObjectType::Rook && KingSideRook.bAway == Piece.bAway && !KingSideRook.bMoved
				&& !Board[6][Home].bValid && !Board[7][Home].bValid
				&& !IsSquareAttacked(6, Home, !Piece.bAway) && !IsSquareAttacked(7, Home, !Piece.bAway))
			{
				FMCMove Castle;
				Castle.FromCol = 5; Castle.FromRow = Home; Castle.ToCol = 7; Castle.ToRow = Home;
				Castle.Kind = EMCMoveKind::CastleKingSide;
				OutMoves.Add(Castle);
			}
			const FMCBoardPiece& QueenSideRook = Board[1][Home];
			if (QueenSideRook.bValid && QueenSideRook.Type == E_ObjectType::Rook && QueenSideRook.bAway == Piece.bAway && !QueenSideRook.bMoved
				&& !Board[2][Home].bValid && !Board[3][Home].bValid && !Board[4][Home].bValid
				&& !IsSquareAttacked(4, Home, !Piece.bAway) && !IsSquareAttacked(3, Home, !Piece.bAway))
			{
				FMCMove Castle;
				Castle.FromCol = 5; Castle.FromRow = Home; Castle.ToCol = 3; Castle.ToRow = Home;
				Castle.Kind = EMCMoveKind::CastleQueenSide;
				OutMoves.Add(Castle);
			}
		}
		break;
	}
	}
}

bool FMCChessRules::IsSquareAttacked(int32 Col, int32 Row, bool bByAway) const
{
	// Pawns
	const int32 PawnRow = Row - PawnDirection(bByAway);
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const int32 PawnCol = Col + Side;
		if (IsOnBoard(PawnCol, PawnRow))
		{
			const FMCBoardPiece& P = Board[PawnCol][PawnRow];
			if (P.bValid && P.bAway == bByAway && P.Type == E_ObjectType::Pawn)
			{
				return true;
			}
		}
	}
	// Knights
	for (const auto& Jump : KnightJumps)
	{
		const int32 C = Col + Jump[0];
		const int32 R = Row + Jump[1];
		if (IsOnBoard(C, R))
		{
			const FMCBoardPiece& P = Board[C][R];
			if (P.bValid && P.bAway == bByAway && P.Type == E_ObjectType::Knight)
			{
				return true;
			}
		}
	}
	// Sliding pieces and the king
	for (const auto& Dir : QueenDirections)
	{
		const bool bDiagonal = Dir[0] != 0 && Dir[1] != 0;
		for (int32 Step = 1; Step <= 7; ++Step)
		{
			const int32 C = Col + Dir[0] * Step;
			const int32 R = Row + Dir[1] * Step;
			if (!IsOnBoard(C, R))
			{
				break;
			}
			const FMCBoardPiece& P = Board[C][R];
			if (!P.bValid)
			{
				continue;
			}
			if (P.bAway == bByAway)
			{
				if (P.Type == E_ObjectType::Queen
					|| (bDiagonal && P.Type == E_ObjectType::Bishop)
					|| (!bDiagonal && P.Type == E_ObjectType::Rook)
					|| (Step == 1 && P.Type == E_ObjectType::King))
				{
					return true;
				}
			}
			break;
		}
	}
	return false;
}

bool FMCChessRules::FindKing(bool bAway, int32& OutCol, int32& OutRow) const
{
	for (int32 C = 1; C <= 8; ++C)
	{
		for (int32 R = 1; R <= 8; ++R)
		{
			const FMCBoardPiece& P = Board[C][R];
			if (P.bValid && P.bAway == bAway && P.Type == E_ObjectType::King)
			{
				OutCol = C;
				OutRow = R;
				return true;
			}
		}
	}
	return false;
}

bool FMCChessRules::IsInCheck(bool bAway) const
{
	int32 KingCol = 0, KingRow = 0;
	return FindKing(bAway, KingCol, KingRow) && IsSquareAttacked(KingCol, KingRow, !bAway);
}

void FMCChessRules::ApplyMove(const FMCMove& Move)
{
	FMCBoardPiece Moving = Board[Move.FromCol][Move.FromRow];
	Board[Move.FromCol][Move.FromRow] = FMCBoardPiece();

	if (Move.Kind == EMCMoveKind::EnPassant)
	{
		Board[Move.ToCol][Move.FromRow] = FMCBoardPiece();
	}
	else if (Move.Kind == EMCMoveKind::CastleKingSide || Move.Kind == EMCMoveKind::CastleQueenSide)
	{
		const int32 RookFrom = Move.Kind == EMCMoveKind::CastleKingSide ? 8 : 1;
		const int32 RookTo = Move.Kind == EMCMoveKind::CastleKingSide ? 6 : 4;
		FMCBoardPiece Rook = Board[RookFrom][Move.FromRow];
		Rook.bMoved = true;
		Board[RookFrom][Move.FromRow] = FMCBoardPiece();
		Board[RookTo][Move.FromRow] = Rook;
	}

	Moving.bMoved = true;
	if (Move.bPromotion)
	{
		// The player picks the piece afterwards; a queen is the strongest assumption for check tests.
		Moving.Type = E_ObjectType::Queen;
	}
	Board[Move.ToCol][Move.ToRow] = Moving;

	EnPassantCol = 0;
	EnPassantRow = 0;
	if (Move.Kind == EMCMoveKind::DoublePawnStep)
	{
		EnPassantCol = Move.FromCol;
		EnPassantRow = (Move.FromRow + Move.ToRow) / 2;
	}
}

void FMCChessRules::GenerateLegalMoves(int32 Col, int32 Row, TArray<FMCMove>& OutMoves) const
{
	if (!IsOnBoard(Col, Row) || !Board[Col][Row].bValid)
	{
		return;
	}
	const bool bAway = Board[Col][Row].bAway;
	TArray<FMCMove> Pseudo;
	GeneratePseudoMoves(Col, Row, Pseudo);
	for (const FMCMove& Move : Pseudo)
	{
		FMCChessRules After = *this;
		After.ApplyMove(Move);
		if (!After.IsInCheck(bAway))
		{
			OutMoves.Add(Move);
		}
	}
}

bool FMCChessRules::FindLegalMove(int32 FromCol, int32 FromRow, int32 ToCol, int32 ToRow, FMCMove& OutMove) const
{
	TArray<FMCMove> Moves;
	GenerateLegalMoves(FromCol, FromRow, Moves);
	for (const FMCMove& Move : Moves)
	{
		if (Move.ToCol == ToCol && Move.ToRow == ToRow)
		{
			OutMove = Move;
			return true;
		}
	}
	return false;
}

bool FMCChessRules::HasAnyLegalMove(bool bAway) const
{
	for (int32 C = 1; C <= 8; ++C)
	{
		for (int32 R = 1; R <= 8; ++R)
		{
			const FMCBoardPiece& P = Board[C][R];
			if (P.bValid && P.bAway == bAway)
			{
				TArray<FMCMove> Moves;
				GenerateLegalMoves(C, R, Moves);
				if (Moves.Num() > 0)
				{
					return true;
				}
			}
		}
	}
	return false;
}
