#include "PMGameMode.h"
#include "PMGameState.h"
#include "Character/Player/PMPlayer.h"
#include "Character/Player/PMPlayerController.h"
#include "Character/Player/PMPlayerState.h"
#include "UI/PMHUD.h"

APMGameMode::APMGameMode()
{
	DefaultPawnClass = APMPlayer::StaticClass();
	PlayerControllerClass = APMPlayerController::StaticClass();
	GameStateClass = APMGameState::StaticClass();
	PlayerStateClass = APMPlayerState::StaticClass();
	HUDClass = APMHUD::StaticClass();
}
