#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMultiplayerChess, Log, All);

// Blueprint graphs keep running after "Accessed None" (they only log it). The C++ port keeps that
// behaviour: it logs the same kind of warning and skips the statement instead of crashing.
#define MC_ACCESSED_NONE(What) UE_LOG(LogMultiplayerChess, Warning, TEXT("Accessed None trying to read %s (%s)"), TEXT(What), ANSI_TO_TCHAR(__FUNCTION__))
