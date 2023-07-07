// Multiplayer Sessions plugin. All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void MenuSetup(                                          //
        const int32 NumberOfPublicConnections = 4,           //
        const FString& TypeOfMatch = FString("FreeForAll"),  //
        const FString& LobyMapPath = FString("/Game/ThirdPerson/Maps/Lobby"));

protected:
    bool Initialize() override;
    void NativeDestruct() override;

    /**
     * Callbacks for the custom delegates on the MultiplayerSessionSubsystem
     */
    virtual void OnCreateSession(bool bWasSussessful);
    virtual void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
    virtual void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
    virtual void OnStartSession(bool bWasSuccessful);
    virtual void OnDestroySession(bool bWasSuccessful);

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

    void MenuShow();
    void MenuTeardown();
};
