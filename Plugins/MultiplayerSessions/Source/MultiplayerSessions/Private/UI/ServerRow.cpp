// Multiplayer Sessions plugin. All rights reserved

#include "UI/ServerRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UServerRow::Setup(int32 InIndex, const FString& ServerNameText)
{
    Index = InIndex;
    ServerName->SetText(FText::FromString(ServerNameText));
    RowButton->OnClicked.AddDynamic(this, &ThisClass::OnClicked);
}

void UServerRow::SetHighlightVisible(bool bIsVisible)
{
    RowButton->SetRenderOpacity(bIsVisible ? 1 : 0);
}

int32 UServerRow::GetIndex()
{
    return Index;
}

void UServerRow::OnClicked()
{
    ServerSelectedDelegate.Broadcast(Index);
}
