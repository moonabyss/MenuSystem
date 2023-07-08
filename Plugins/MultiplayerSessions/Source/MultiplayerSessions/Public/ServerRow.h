// Multiplayer Sessions plugin. All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "ServerRow.generated.h"

class UButton;
class UTextBlock;
class UMenu;

UCLASS()
class MULTIPLAYERSESSIONS_API UServerRow : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ServerName;

    UPROPERTY(BlueprintReadOnly)
    bool Selected{false};

    void Setup(UMenu* InParent, int32 InIndex);
    void SetHighlightVisible(bool bIsVisible);
    int32 GetIndex();

private:
    UFUNCTION()
    void OnClicked();

    UPROPERTY()
    UMenu* Parent;

    UPROPERTY(meta = (BindWidget))
    UButton* RowButton;

    int32 Index;
};
