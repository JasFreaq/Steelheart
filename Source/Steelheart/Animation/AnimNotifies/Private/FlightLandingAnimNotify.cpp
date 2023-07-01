#include "Steelheart/Animation/AnimNotifies/Public/FlightLandingAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Steelheart/Characters/Public/SteelheartCharacter.h"

void UFlightLandingAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	// Cast the owner of the skeletal mesh component to ACharacter
	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());

	// Check if the cast was successful
	if (ensure(OwnerCharacter != nullptr))
	{
		// Enable input for the owner character
		OwnerCharacter->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}
}