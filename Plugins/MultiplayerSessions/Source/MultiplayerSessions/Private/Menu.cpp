// Multiplayer Sessions plugin. All rights reserved

#include "Menu.h"

void UMenu::MenuSetup() 
{
    check(GetWorld());

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController()){
        FInputModeUIOnly InputModeData;
        InputModeData.SetWidgetToFocus(TakeWidget());
        InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputModeData);
        PC->SetShowMouseCursor(true);
    }
}
