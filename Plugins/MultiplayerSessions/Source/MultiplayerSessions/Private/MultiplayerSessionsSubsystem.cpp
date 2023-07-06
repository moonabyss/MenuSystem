// Multiplayer Sessions plugin. All rights reserved

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
    : CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
    , FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete))
    , JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
    , DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
    , StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
    }
}

void UMultiplayerSessionsSubsystem::SetupSession(
    const int32 NumberOfPublicConnections, const FString& TypeOfMatch, const FString& LobyMapPath)
{
    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;
    LobbyMap = LobyMapPath;
}

void UMultiplayerSessionsSubsystem::CreateSession()
{
    if (!SessionInterface.IsValid()) return;

    auto ExistingSession = SessionInterface->GetNamedSession(MenuSessionName);
    if (ExistingSession)
    {
        SessionInterface->DestroySession(MenuSessionName, DestroySessionCompleteDelegate);
    }

    // Store the delegate
    CreateSessionCompleteDelegate_Handle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

    // session settings
    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    const bool IsSubsystemNull = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
    LastSessionSettings->bIsLANMatch = IsSubsystemNull;
    LastSessionSettings->NumPublicConnections = NumPublicConnections;
    LastSessionSettings->bAllowJoinInProgress = true;
    LastSessionSettings->bAllowJoinViaPresence = true;
    LastSessionSettings->bUsesPresence = true;
    LastSessionSettings->bShouldAdvertise = true;
    LastSessionSettings->bUseLobbiesIfAvailable = true;
    LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    LastSessionSettings->Set(SEARCH_KEYWORDS, SearchServerKeyword, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    // TODO: Set server name

    const auto LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    // Try to create session
    if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), MenuSessionName, *LastSessionSettings))
    {
        // if not successful, remove from stored delegates list
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);

        // broadcast custom delegate
        MultiplayerCreateSessionCompleteDelegate.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (!SessionInterface.IsValid()) return;

    // Store the delegate
    FindSessionsCompleteDelegate_Handle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

    LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    LastSessionSearch->MaxSearchResults = MaxSearchResults;
    const bool IsSubsystemNull = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
    LastSessionSearch->bIsLanQuery = IsSubsystemNull;
    LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    LastSessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, SearchServerKeyword, EOnlineComparisonOp::Equals);
    // TODO: Set server name

    // try to find sessions
    const auto LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
    {
        // if not successful, remove from stored delegates list
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);

        // broadcast custom delegate
        MultiplayerFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
    }
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult) {}

void UMultiplayerSessionsSubsystem::DestroySession() {}

void UMultiplayerSessionsSubsystem::StartSession() {}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    // broadcast custom delegate
    MultiplayerCreateSessionCompleteDelegate.Broadcast(bWasSuccessful);

    if (SessionInterface)
    {
        // remove from stored delegates list
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);
    }

    if (bWasSuccessful && GetWorld())
    {
        // Travel to Lobby Map
        GetWorld()->ServerTravel(LobbyMap.Append("?listen"));
    }
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    // broadcast custom delegate
    MultiplayerFindSessionsCompleteDelegate.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);

    if (SessionInterface)
    {
        // remove from stored delegates list
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);
    }
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful) {}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful) {}
