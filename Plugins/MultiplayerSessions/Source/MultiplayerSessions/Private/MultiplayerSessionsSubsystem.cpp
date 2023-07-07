// Multiplayer Sessions plugin. All rights reserved

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogMultiplayerSubsystem, All, All);

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
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("SetupSession()"));
#endif

    MultiplayerSessionSettings.NumPublicConnections = NumberOfPublicConnections;
    MultiplayerSessionSettings.MatchType = TypeOfMatch;
    MultiplayerSessionSettings.LobbyMap = LobyMapPath;
}

void UMultiplayerSessionsSubsystem::CreateSession()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("CreateSession()"));
#endif

    if (!SessionInterface.IsValid())
    {
        MultiplayerCreateSessionCompleteDelegate.Broadcast(false);
        return;
    }

    // Destroy existing session
    auto ExistingSession = SessionInterface->GetNamedSession(MenuSessionName);
    if (ExistingSession)
    {
        DestroySession(MenuSessionName);
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
    // LastSessionSettings->Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

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
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("FindSessions()"));
#endif

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
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("JoinSession()"));
#endif

    if (!SessionInterface.IsValid())
    {
        MultiplayerJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
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
        MultiplayerJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    }
}

void UMultiplayerSessionsSubsystem::DestroySession(FName SessionName)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("DestroySession()"));
#endif

    if (!SessionInterface.IsValid())
    {
        MultiplayerDestroySessionCompleteDelegate.Broadcast(false);
        return;
    }

    // Store the delegate
    DestroySessionCompleteDelegate_Handle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

    if (!SessionInterface->DestroySession(SessionName))
    {
        // if not successful, remove from stored delegates list
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate_Handle);

        // broadcast custom delegate
        MultiplayerDestroySessionCompleteDelegate.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::StartSession()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("StartSession()"));
#endif

    if (!SessionInterface.IsValid())
    {
        MultiplayerStartSessionCompleteDelegate.Broadcast(false);
        return;
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("OnCreateSessionComplete()"));
#endif

    if (!SessionInterface)
    {
        // broadcast custom delegate
        MultiplayerCreateSessionCompleteDelegate.Broadcast(false);
        return;
    }

    // broadcast custom delegate
    MultiplayerCreateSessionCompleteDelegate.Broadcast(bWasSuccessful);

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
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("OnFindSessionsComplete()"));
#endif

    if (!SessionInterface)
    {
        // broadcast custom delegate
        MultiplayerFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }

    // remove from stored delegates list
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);

    // Filter results by MatchType
    TArray<FOnlineSessionSearchResult> FilteredResults = LastSessionSearch->SearchResults.FilterByPredicate(
        [&](const FOnlineSessionSearchResult& Result)
        {
            FString MatchType;
            Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
            return MatchType == MultiplayerSessionSettings.MatchType;
        });

    /*
    FServerData Data;
    Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
    Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
    Data.HostUserName = SearchResult.Session.OwningUserName;
    FString ServerName;
    if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName) && !ServerName.Equals(""))
    {
        Data.Name = ServerName;
    }
    else
    {
        Data.Name = SearchResult.GetSessionIdStr();
    }
    ServerNames.Add(Data);
    */

    // broadcast custom delegate
    MultiplayerFindSessionsCompleteDelegate.Broadcast(FilteredResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("OnJoinSessionComplete()"));
#endif

    if (!SessionInterface)
    {
        // broadcast custom delegate
        MultiplayerJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    // remove from stored delegates list
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate_Handle);

    // broadcast custom delegate
    MultiplayerJoinSessionCompleteDelegate.Broadcast(Result);

    FString Address;
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        SessionInterface->GetResolvedConnectString(SessionName, Address);
    }

    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (PC)
    {
        PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
    }
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("OnDestroySessionComplete()"));
#endif

    if (!SessionInterface.IsValid())
    {
        // broadcast custom delegate
        MultiplayerDestroySessionCompleteDelegate.Broadcast(false);
        return;
    }

    // broadcast custom delegate
    MultiplayerDestroySessionCompleteDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogMultiplayerSubsystem, Display, TEXT("OnStartSessionComplete()"));
#endif

    if (!SessionInterface.IsValid())
    {
        // broadcast custom delegate
        MultiplayerStartSessionCompleteDelegate.Broadcast(false);
        return;
    }

    // broadcast custom delegate
    MultiplayerStartSessionCompleteDelegate.Broadcast(bWasSuccessful);
}
