// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TempPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STEELHEART_API ATempPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UProceduralMeshComponent* CreateProceduralMeshComponent(AActor* Owning, const FName& AttachSocket = NAME_None);
};
