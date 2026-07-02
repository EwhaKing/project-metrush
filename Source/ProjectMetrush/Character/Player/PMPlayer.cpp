#include "ProjectMetrush/Character/Player/PMPlayer.h"
#include "ProjectMetrush/Component/PMCombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APMPlayer::APMPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.f, 900.f, 0.f);

	// 기획서 4.6 기본 이동 수치
	MoveComp->MaxWalkSpeed = 500.f;
	MoveComp->MaxAcceleration = 6000.f;
	MoveComp->BrakingDecelerationWalking = 8000.f;

	// 기획서 4.8 경사/단차 수치
	MoveComp->SetWalkableFloorAngle(45.f);
	MoveComp->MaxStepHeight = 35.f;

	// 쿼터뷰 카메라 (각도, 거리는 BP에서 튜닝)
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->SetRelativeRotation(FRotator(-55.f, -45.f, 0.f));
	SpringArmComponent->TargetArmLength = 1200.f;
	SpringArmComponent->bDoCollisionTest = false;
	SpringArmComponent->bUsePawnControlRotation = false;
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritYaw = false;
	SpringArmComponent->bInheritRoll = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	CombatComponent = CreateDefaultSubobject<UPMCombatComponent>(TEXT("CombatComponent"));
}

void APMPlayer::BeginPlay()
{
	Super::BeginPlay();

	LastMoveDirection = GetActorForwardVector();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APMPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APMPlayer::Input_Move);
	EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Started, this, &APMPlayer::Input_Dodge);
	EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &APMPlayer::Input_Attack);
}

void APMPlayer::Input_Move(const FInputActionValue& InputActionValue)
{
	FVector2D Input = InputActionValue.Get<FVector2D>();
	if (Input.SizeSquared() > 1.f)
	{
		Input.Normalize();
	}

	// 카메라 기준 방향 계산 (기획서 4.4)
	const FRotator YawRotation(0.f, CameraComponent->GetComponentRotation().Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 방향 의사는 상태와 무관하게 항상 기록 (공격 중 캔슬 회피의 방향으로 사용)
	const FVector MoveDir = ForwardDir * Input.Y + RightDir * Input.X;
	if (!MoveDir.IsNearlyZero())
	{
		LastMoveDirection = MoveDir.GetSafeNormal();
	}

	// 실제 이동은 자유 상태에서만 (기획서: 공격/회피 중 이동 없음)
	if (!CombatComponent->CanMove())
	{
		return;
	}

	AddMovementInput(ForwardDir, Input.Y);
	AddMovementInput(RightDir, Input.X);
}

void APMPlayer::Input_Dodge(const FInputActionValue& InputActionValue)
{
	CombatComponent->TryDodge(LastMoveDirection);
}

void APMPlayer::Input_Attack(const FInputActionValue& InputActionValue)
{
	CombatComponent->TryAttack(LastMoveDirection);
}