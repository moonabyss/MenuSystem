// Multiplayer Sessions plugin. All rights reserved

#include "Menu.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"

bool UMenu::Initialize()
{
    if (!Super::Initialize()) return false;

    if (!ensure(HostButton)) return false;
    if (!ensure(JoinButton)) return false;

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

    SetButtonsEnabled(false);

    MultiplayerSessionsSubsystem->MultiplayerCreateSessionCompleteDelegate.AddUObject(this, &ThisClass::OnCreateSession);
    MultiplayerSessionsSubsystem->CreateSession();
}

void UMenu::JoinButtonClicked()
{
    if (!MultiplayerSessionsSubsystem) return;

    SetButtonsEnabled(false);

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

    if (!bWasSussessful)
    {
        SetButtonsEnabled(true);
    }
}

void UMenu::OnFindSessions(const TArray<FServerData>& SearchResults, bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerFindSessionsCompleteDelegate.RemoveAll(this);

    for (const auto& Result : SearchResults)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("%s"), *Result.ServerName));
    }

    // Join session
    MultiplayerSessionsSubsystem->MultiplayerJoinSessionCompleteDelegate.AddUObject(this, &ThisClass::OnJoinSession);
    if (bWasSuccessful && SearchResults.Num() == 1)
    {
        MultiplayerSessionsSubsystem->JoinSession(0);
    }
    else
    {
        SetButtonsEnabled(true);
    }
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerJoinSessionCompleteDelegate.RemoveAll(this);

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        SetButtonsEnabled(true);
    }
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerStartSessionCompleteDelegate.RemoveAll(this);
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    // Clear bindings
    MultiplayerSessionsSubsystem->MultiplayerDestroySessionCompleteDelegate.RemoveAll(this);
}

void UMenu::SetButtonsEnabled(bool bIsButtonEnabled)
{
    HostButton->SetIsEnabled(bIsButtonEnabled);
    JoinButton->SetIsEnabled(bIsButtonEnabled);
}
