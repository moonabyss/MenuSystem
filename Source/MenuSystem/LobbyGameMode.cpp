// MenuSystem. All rights reserved

#include "LobbyGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (GameState)
    {
        int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1, 60.0f, FColor::Yellow, FString::Printf(TEXT("Players in game: %d"), NumOfPlayers));

            auto* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
            if (PlayerState)
            {
                FString PlayerName = PlayerState->GetPlayerName();
                GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Cyan, FString::Printf(TEXT("%s has joined the game"), *PlayerName));
            }
        }
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (GEngine)
    {
        int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
        GEngine->AddOnScreenDebugMessage(1, 60.0f, FColor::Yellow, FString::Printf(TEXT("Players in game: %d"), NumOfPlayers--));

        auto* PlayerState = Exiting->GetPlayerState<APlayerState>();
        if (PlayerState)
        {
            FString PlayerName = PlayerState->GetPlayerName();
            GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Cyan, FString::Printf(TEXT("%s has exited the game"), *PlayerName));
        }
    }
}
