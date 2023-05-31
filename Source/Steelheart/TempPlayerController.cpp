// Fill out your copyright notice in the Description page of Project Settings.


#include "TempPlayerController.h"
#include "ProceduralMeshComponent.h"

UProceduralMeshComponent* ATempPlayerController::CreateProceduralMeshComponent(AActor* Owning, const FName& AttachSocket)
{
	FName YourObjectName("Proc");

	//CompClass can be a BP
	UProceduralMeshComponent* ProcComp = NewObject<UProceduralMeshComponent>(Owning, UProceduralMeshComponent::StaticClass(), YourObjectName);
	//UPrimitiveComponent* NewComp = ConstructObject<UPrimitiveComponent>(CompClass, this, YourObjectName);
	if (!ProcComp)
	{
		return NULL;
	}
	//~~~~~~~~~~~~~

	ProcComp->RegisterComponent();        //You must ConstructObject with a valid Outer that has world, see above	 
	
	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	ProcComp->AttachToComponent(Owning->GetDefaultAttachComponent(), AttachmentTransformRules, AttachSocket);
	Owning->AddInstanceComponent(ProcComp);
	//could use different than Root Comp

	return ProcComp;
}
