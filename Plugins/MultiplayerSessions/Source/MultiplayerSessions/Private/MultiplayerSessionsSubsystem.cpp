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
    MultiplayerSessionSettings.NumPublicConnections = NumberOfPublicConnections;
    MultiplayerSessionSettings.MatchType = TypeOfMatch;
    MultiplayerSessionSettings.LobbyMap = LobyMapPath;
}

void UMultiplayerSessionsSubsystem::CreateSession()
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerCreateSessionCompleteDelegate.Broadcast(false);
        return;
    }

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
    LastSessionSettings->NumPublicConnections = MultiplayerSessionSettings.NumPublicConnections;
    LastSessionSettings->bAllowJoinInProgress = true;
    LastSessionSettings->bAllowJoinViaPresence = true;
    LastSessionSettings->bUsesPresence = true;
    LastSessionSettings->bShouldAdvertise = true;
    LastSessionSettings->bUseLobbiesIfAvailable = true;
    LastSessionSettings->Set(
        FName("MatchType"), MultiplayerSessionSettings.MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
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
    if (!SessionInterface.IsValid())
    {
        MultiplayerFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }

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

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if (!SessionInterface.IsValid())
    {
        MultiplayerJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError, FString());
        return;
    }

    // Store the delegate
    JoinSessionCompleteDelegate_Handle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

    // try to join session
    const auto LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), MenuSessionName, SessionResult))
    {
        // if not successful, remove from stored delegates list
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate_Handle);

        // broadcast custom delegate
        MultiplayerJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError, FString());
    }
}

void UMultiplayerSessionsSubsystem::DestroySession() {}

void UMultiplayerSessionsSubsystem::StartSession() {}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    // broadcast custom delegate
    MultiplayerCreateSessionCompleteDelegate.Broadcast(bWasSuccessful);

    if (!SessionInterface) return;

    // remove from stored delegates list
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);

    if (bWasSuccessful && GetWorld())
    {
        // Travel to Lobby Map
        GetWorld()->ServerTravel(MultiplayerSessionSettings.LobbyMap.Append("?listen"));
    }
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    // Filter results by MatchType
    TArray<FOnlineSessionSearchResult> FilteredResults = LastSessionSearch->SearchResults.FilterByPredicate(
        [&](const FOnlineSessionSearchResult& Result)
        {
            FString MatchType;
            Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
            return MatchType == MultiplayerSessionSettings.MatchType;
        });

    // broadcast custom delegate
    MultiplayerFindSessionsCompleteDelegate.Broadcast(FilteredResults, bWasSuccessful);

    if (SessionInterface)
    {
        // remove from stored delegates list
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);
    }
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface) return;

    // remove from stored delegates list
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate_Handle);

    FString Address;
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        SessionInterface->GetResolvedConnectString(SessionName, Address);
    }

    // broadcast custom delegate
    MultiplayerJoinSessionCompleteDelegate.Broadcast(Result, Address);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful) {}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful) {}
