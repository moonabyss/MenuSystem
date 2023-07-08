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

void UServerRow::OnClicked()
{
    Parent->SelectIndex(Index);
}
