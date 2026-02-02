// Copyright GameDevWare, Denis Zykov 2025

#pragma once
#include "CoreTypes.h"
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"
#include "Math/Vector4.h"
#include "Math/IntPoint.h"
#include "Math/IntVector.h"
#include "FGameDataDocumentReference.h"
#include "EDocumentReferenceGetResult.h"
#include "FLocalizedText.h"
#include "UGameDataDocument.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UGameDataLibrary.generated.h"

/*
 * Additional function for Blueprints related to game data.
 */
UCLASS()
class CHARON_API UGameDataLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	/*
	 * Get UGameDataDocument referenced in specified DocumentReference.
	 */
	UFUNCTION(BlueprintCallable, Meta = (ExpandEnumAsExecs = "Branches"), Category="Game Data")
	static UGameDataDocument* GetDocument(FGameDataDocumentReference DocumentReference, EDocumentReferenceGetResult& Branches)
	{
		UGameDataDocument* Document = DocumentReference.GetReferencedDocument();
		if (Document == nullptr)
		{
			Branches = EDocumentReferenceGetResult::NotFound;
		}
		else
		{
			Branches = EDocumentReferenceGetResult::Found;
		}
		return Document;
	}

	/*
	 * Get current text specified in internal ULanguageSwitcher of selected LocalizedString.
	 */
	UFUNCTION(BlueprintCallable, Category="Game Data Localization")
	static FText GetCurrentText(FLocalizedText LocalizedString)
	{
		return LocalizedString.GetCurrent();
	}
	
	/*
	 * Parse space separated vector components into vector structure. 
	 */
	UFUNCTION(BlueprintCallable, Category="Conversion")
	static FVector2D ParseVector2(FString VectorStr /* space separated vector components */)
	{
		TArray<FString> Components;
		// Split by space, culling empty entries to handle multiple spaces
		VectorStr.ParseIntoArray(Components, TEXT(" "), true);

		double X = (Components.Num() > 0) ? FCString::Atod(*Components[0]) : 0.0;
		double Y = (Components.Num() > 1) ? FCString::Atod(*Components[1]) : 0.0;

		return FVector2D(X, Y);
	}
	
	/*
	 * Parse space separated vector components into vector structure. 
	 */
	UFUNCTION(BlueprintCallable, Category="Conversion")
	static FVector ParseVector3(FString VectorStr /* space separated vector components */)
	{
		TArray<FString> Components;
		VectorStr.ParseIntoArray(Components, TEXT(" "), true);

		double X = (Components.Num() > 0) ? FCString::Atod(*Components[0]) : 0.0;
		double Y = (Components.Num() > 1) ? FCString::Atod(*Components[1]) : 0.0;
		double Z = (Components.Num() > 2) ? FCString::Atod(*Components[2]) : 0.0;

		return FVector(X, Y, Z);
	}
	
	/*
	 * Parse space separated vector components into vector structure. 
	 */
	UFUNCTION(BlueprintCallable, Category="Conversion")
	static FVector4 ParseVector4(FString VectorStr /* space separated vector components */)
	{
		TArray<FString> Components;
		VectorStr.ParseIntoArray(Components, TEXT(" "), true);

		double X = (Components.Num() > 0) ? FCString::Atod(*Components[0]) : 0.0;
		double Y = (Components.Num() > 1) ? FCString::Atod(*Components[1]) : 0.0;
		double Z = (Components.Num() > 2) ? FCString::Atod(*Components[2]) : 0.0;
		double W = (Components.Num() > 3) ? FCString::Atod(*Components[3]) : 0.0;

		return FVector4(X, Y, Z, W);
	}
	
	/*
	 * Parse space separated vector components into vector structure. 
	 */
	UFUNCTION(BlueprintCallable, Category="Conversion")
	static FIntPoint ParseFIntPoint(FString VectorStr /* space separated vector components */)
	{
		TArray<FString> Components;
		VectorStr.ParseIntoArray(Components, TEXT(" "), true);

		double X = (Components.Num() > 0) ? FCString::Atoi(*Components[0]) : 0.0;
		double Y = (Components.Num() > 1) ? FCString::Atoi(*Components[1]) : 0.0;

		return FIntPoint(X, Y);
	}
	
	/*
	 * Parse space separated vector components into vector structure. 
	 */
	UFUNCTION(BlueprintCallable, Category="Conversion")
	static FIntVector ParseIntVector3(FString VectorStr /* space separated vector components */)
	{
		TArray<FString> Components;
		VectorStr.ParseIntoArray(Components, TEXT(" "), true);

		double X = (Components.Num() > 0) ? FCString::Atoi(*Components[0]) : 0.0;
		double Y = (Components.Num() > 1) ? FCString::Atoi(*Components[1]) : 0.0;
		double Z = (Components.Num() > 2) ? FCString::Atoi(*Components[2]) : 0.0;

		return FIntVector3(X, Y, Z);
	}
	
	/*
	 * Parse space separated vector components into vector structure. 
	 */
	UFUNCTION(BlueprintCallable, Category="Conversion")
	static FIntVector4 ParseIntVector4(FString VectorStr /* space separated vector components */)
	{
		TArray<FString> Components;
		VectorStr.ParseIntoArray(Components, TEXT(" "), true);

		double X = (Components.Num() > 0) ? FCString::Atoi(*Components[0]) : 0.0;
		double Y = (Components.Num() > 1) ? FCString::Atoi(*Components[1]) : 0.0;
		double Z = (Components.Num() > 2) ? FCString::Atoi(*Components[2]) : 0.0;
		double W = (Components.Num() > 3) ? FCString::Atoi(*Components[3]) : 0.0;

		return FIntVector4(X, Y, Z, W);
	}
	
		
	/*
	 * Parse space tokens into separate tags. 
	 */
	UFUNCTION(BlueprintCallable, Category="Conversion")
	static TArray<FString> ParseTags(const FString TargsStr /* space separated tags */)
	{
		TArray<FString> Components;
		TargsStr.ParseIntoArray(Components, TEXT(" "), true);
		return Components;
	}
};
