// Multiplayer Sessions plugin. All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "Menu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void MenuSetup(const int32 NumberOfPublicConnections = 4, const FString& TypeOfMatch = FString("FreeForAll"));

protected:
    bool Initialize() override;
    void NativeDestruct() override;

private:
    UPROPERTY(meta = (BindWidget))
    UButton* HostButton;

    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton;

    UFUNCTION()
    void HostButtonClicked();

    UFUNCTION()
    void JoinButtonClicked();

    // The subsystem designed to handle all online session functionality
    UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

    void MenuTeardown();

    int32 NumPublicConnections;
    FString MatchType;
};
