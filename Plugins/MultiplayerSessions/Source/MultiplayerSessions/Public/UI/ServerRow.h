// Multiplayer Sessions plugin. All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "ServerRow.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnServerSelectedDelegate, int32 Index);

class UButton;
class UTextBlock;

UCLASS()
class MULTIPLAYERSESSIONS_API UServerRow : public UUserWidget
{
    GENERATED_BODY()

public:
    void Setup(int32 InIndex, const FString& ServerNameText);
    void SetHighlightVisible(bool bIsVisible);
    int32 GetIndex();

    FOnServerSelectedDelegate ServerSelectedDelegate;

private:
    UFUNCTION()
    void OnClicked();

    UPROPERTY(meta = (BindWidget))
    UButton* RowButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ServerName;

    int32 Index;
};
