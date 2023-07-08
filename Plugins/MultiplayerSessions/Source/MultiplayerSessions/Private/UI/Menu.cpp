// Multiplayer Sessions plugin. All rights reserved

#include "UI/Menu.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "OnlineSessionSettings.h"

#include "UI/ServerRow.h"

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

    SelectedIndex = SearchResults.Num() > 0 ? 0 : -1;
    FillServerList(SearchResults);
    SetButtonsEnabled(true);
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

void UMenu::FillServerList(const TArray<FServerData>& SearchResults)
{
    ServerList->ClearChildren();
    int32 Index{0};
    for (const auto& Result : SearchResults)
    {
        auto Row = CreateWidget<UServerRow>(GetWorld(), ServerRowClass);
        if (!Row) continue;

        Row->Setup(Index, Result.ServerName);
        Row->SetHighlightVisible(SelectedIndex == Index);
        Row->ServerSelectedDelegate.AddUObject(this, &ThisClass::SelectIndex);
        ServerList->AddChild(Row);
        Index++;
    }
}
