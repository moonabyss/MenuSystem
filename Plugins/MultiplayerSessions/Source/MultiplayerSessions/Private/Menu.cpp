// Multiplayer Sessions plugin. All rights reserved

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

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

    MultiplayerSessionsSubsystem->CreateSession();
}

void UMenu::JoinButtonClicked() {}

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

void UMenu::OnCreateSession(bool bWasSussessful) {}
