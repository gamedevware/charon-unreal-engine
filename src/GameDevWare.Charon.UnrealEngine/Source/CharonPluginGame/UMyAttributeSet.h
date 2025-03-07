#pragma once

#include "AttributeSet.h"

#include "UMyAttributeSet.generated.h"

UCLASS(BlueprintType)
class UMyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	 
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Health;
};