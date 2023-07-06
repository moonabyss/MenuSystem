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

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, const FString& MatchType)
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
    const bool IsSubsystemNull = *IOnlineSubsystem::Get()->GetSubsystemName().ToString().ToLower() == FString("null");
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
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults) {}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult) {}

void UMultiplayerSessionsSubsystem::DestroySession() {}

void UMultiplayerSessionsSubsystem::StartSession() {}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    GetWorld()->ServerTravel("/Game/ThirdPerson/Maps/Lobby?listen");
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful) {}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful) {}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful) {}
