// Multiplayer Sessions plugin. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MultiplayerSessionsSubsystem.generated.h"

struct FMultiplayerSessionSettings
{
    int32 NumPublicConnections;
    FString MatchType;
    FString LobbyMap;
};

/**
 * Declaring our own custom delegates for the Menu class to bind callbacks to
 */
// clang-format off
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMultiplayerCreateSessionCompleteDelegate, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMultiplayerFindSessionsCompleteDelegate, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMultiplayerJoinSessionCompleteDelegate, EOnJoinSessionCompleteResult::Type Result, const FString& Address);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMultiplayerStartSessionCompleteDelegate, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMultiplayerDestroySessionCompleteDelegate, bool bWasSuccessful);
// clang-format on

class FOnlineSessionSettings;

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMultiplayerSessionsSubsystem();

    /**
     * To handle session functionality. The Menu class will call this.
     */
    void SetupSession(const int32 NumberOfPublicConnections, const FString& TypeOfMatch, const FString& LobyMapPath);
    void CreateSession();
    void FindSessions(int32 MaxSearchResults = 10);
    void JoinSession(const FOnlineSessionSearchResult& SessionResult);
    void DestroySession();
    void StartSession();

    /**
     * Our own custom delegates for the Menu class to bind callbacks to
     */
    FOnMultiplayerCreateSessionCompleteDelegate MultiplayerCreateSessionCompleteDelegate;
    FOnMultiplayerFindSessionsCompleteDelegate MultiplayerFindSessionsCompleteDelegate;
    FOnMultiplayerJoinSessionCompleteDelegate MultiplayerJoinSessionCompleteDelegate;
    FOnMultiplayerDestroySessionCompleteDelegate MultiplayerDestroySessionCompleteDelegate;
    FOnMultiplayerStartSessionCompleteDelegate MultiplayerStartSessionCompleteDelegate;

protected:
    /**
     * Internal callbacks for the delegates we'll add to the Online Session Interface delegate list.
     * These don't need to be called outside this class.
     */
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
    TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

    const FName MenuSessionName = FName("MoonabyssGameSession");
    const FString SearchServerKeyword = FString("MoonabyssGame");

    FMultiplayerSessionSettings MultiplayerSessionSettings{};

    /**
     * To add to the Online Session Interface delegate list.
     * We'll bind our MultiplayerSessionSubsystem internal callbacks to these.
     */
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegate_Handle;

    FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
    FDelegateHandle FindSessionsCompleteDelegate_Handle;

    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegate_Handle;

    FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
    FDelegateHandle DestroySessionCompleteDelegate_Handle;

    FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
    FDelegateHandle StartSessionCompleteDelegate_Handle;
};
