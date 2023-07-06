// Multiplayer Sessions plugin. All rights reserved

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"

bool UMenu::Initialize()
{
    if (!Super::Initialize()) return false;

    HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);

    return true;
}

void UMenu::NativeDestruct()
{
    MenuTeardown();
    Super::NativeDestruct();
}

void UMenu::MenuSetup(const int32 NumberOfPublicConnections, const FString& TypeOfMatch, const FString& LobyMapPath)
{
    check(GetGameInstance());

    MultiplayerSessionsSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSessionsSubsystem>();
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->SetupSession(NumberOfPublicConnections, TypeOfMatch, LobyMapPath);
        MenuShow();
    }
}

void UMenu::HostButtonClicked()
{
    if (!MultiplayerSessionsSubsystem) return;

    MultiplayerSessionsSubsystem->MultiplayerCreateSessionCompleteDelegate.AddUObject(this, &ThisClass::OnCreateSession);
    MultiplayerSessionsSubsystem->CreateSession();
}

void UMenu::JoinButtonClicked()
{
    if (!MultiplayerSessionsSubsystem) return;

    MultiplayerSessionsSubsystem->MultiplayerFindSessionsCompleteDelegate.AddUObject(this, &ThisClass::OnFindSessions);
    MultiplayerSessionsSubsystem->FindSessions();
}

void UMenu::MenuShow()
{
    check(GetWorld());

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    // Set input mode
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeUIOnly InputModeData;
        InputModeData.SetWidgetToFocus(TakeWidget());
        InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputModeData);
        PC->SetShowMouseCursor(true);
    }
}

void UMenu::MenuTeardown()
{
    check(GetWorld());

    RemoveFromParent();

    // Set input mode
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputModeData;
        PC->SetInputMode(InputModeData);
        PC->SetShowMouseCursor(false);
    }
}

void UMenu::OnCreateSession(bool bWasSussessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerCreateSessionCompleteDelegate.RemoveAll(this);
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerFindSessionsCompleteDelegate.RemoveAll(this);

    // Join session
    MultiplayerSessionsSubsystem->MultiplayerJoinSessionCompleteDelegate.AddUObject(this, &ThisClass::OnJoinSession);
    if (bWasSuccessful && SessionResults.Num() == 1)
    {
        MultiplayerSessionsSubsystem->JoinSession(SessionResults[0]);
    }
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result, const FString& Address)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerJoinSessionCompleteDelegate.RemoveAll(this);

    MultiplayerSessionsSubsystem->MultiplayerStartSessionCompleteDelegate.AddUObject(this, &ThisClass::OnStartSession);

    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (PC)
    {
        PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
    }
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerStartSessionCompleteDelegate.RemoveAll(this);

    MultiplayerSessionsSubsystem->MultiplayerStartSessionCompleteDelegate.AddUObject(this, &ThisClass::OnDestroySession);
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerDestroySessionCompleteDelegate.RemoveAll(this);
}
