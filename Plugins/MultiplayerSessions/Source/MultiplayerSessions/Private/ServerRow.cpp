// Multiplayer Sessions plugin. All rights reserved

#include "ServerRow.h"
#include "Components/Button.h"
#include "Menu.h"

void UServerRow::Setup(UMenu* InParent, int32 InIndex)
{
    Parent = InParent;
    Index = InIndex;
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
    Parent->SelectIndex(Index);
}
