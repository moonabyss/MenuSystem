// Multiplayer Sessions plugin. All rights reserved

#include "Menu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "OnlineSessionSettings.h"

#include "ServerRow.h"

bool UMenu::Initialize()
{
    if (!Super::Initialize()) return false;

    if (!ensure(HostButton)) return false;
    if (!ensure(JoinButton)) return false;
    if (!ensure(RefreshButton)) return false;

    ServerList->ClearChildren();

    HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
    RefreshButton->OnClicked.AddDynamic(this, &ThisClass::RefreshButtonClicked);

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

void UMenu::SelectIndex(int32 Index)
{
    if (Index < 0) return;

    SelectedIndex = Index;
    UpdateChildren();
    GEngine->AddOnScreenDebugMessage(0, 10.0f, FColor::Yellow, FString::Printf(TEXT("%d"), Index));
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

    // Join session
    if (SelectedIndex >= 0)
    {
        SetButtonsEnabled(false);
        MultiplayerSessionsSubsystem->JoinSession(SelectedIndex);
    }
}

void UMenu::RefreshButtonClicked()
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

    if (SearchResults.Num() > 0)
    {
        SelectedIndex = 0;
    }
    ServerList->ClearChildren();
    int32 Index{0};
    for (const auto& Result : SearchResults)
    {
        auto Row = CreateWidget<UServerRow>(GetWorld(), ServerRowClass);
        if (!Row) continue;

        Row->Setup(this, Index);
        Row->ServerName->SetText(FText::FromString(Result.ServerName));
        Row->SetHighlightVisible(SelectedIndex == Index);
        ServerList->AddChild(Row);
        Index++;
    }
    SetButtonsEnabled(true);

    return;
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
    RefreshButton->SetIsEnabled(bIsButtonEnabled);
    JoinButton->SetIsEnabled(bIsButtonEnabled);
}

void UMenu::UpdateChildren()
{
    for (const auto& Child : ServerList->GetAllChildren())
    {
        const auto& Row = Cast<UServerRow>(Child);
        Row->SetHighlightVisible(SelectedIndex == Row->GetIndex());
    }
}
