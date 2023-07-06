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

void UMenu::MenuSetup(const int32 NumberOfPublicConnections, const FString& TypeOfMatch)
{
    check(GetWorld());
    check(GetGameInstance());

    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;

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

    MultiplayerSessionsSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSessionsSubsystem>();
}

void UMenu::HostButtonClicked()
{
    if (!MultiplayerSessionsSubsystem) return;

    MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
}

void UMenu::JoinButtonClicked() {}

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
