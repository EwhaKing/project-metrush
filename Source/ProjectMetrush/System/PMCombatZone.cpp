#include "PMCombatZone.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Actor/PMSpawnPoint.h"
#include "Character/Enemy/PMEnemy.h"

APMCombatZone::APMCombatZone()
{
    PrimaryActorTick.bCanEverTick = false;

    // 전투방 : 50 x 50 기준
    ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
    SetRootComponent(ZoneBox);
    ZoneBox->SetBoxExtent(FVector(2500.f, 2500.f, 300.f));
    ZoneBox->SetCollisionProfileName(TEXT("Trigger"));
}

void APMCombatZone::BeginPlay()
{
    Super::BeginPlay();

    ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &APMCombatZone::OnZoneBeginOverlap);
}

void APMCombatZone::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 전투는 한 번만 시작
    if (bCombatStarted)
    {
        return;
    }

    // 들어온 것이 플레이어일 때만
    if (OtherActor == nullptr || OtherActor != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        return;
    }

    StartCombat();
}

void APMCombatZone::StartCombat()
{
    bCombatStarted = true;
    CurrentWaveIndex = -1;

    UE_LOG(LogTemp, Warning, TEXT("Combat Started"));

    // 추후 조정필요 : 전투 시작 시 진입 열차 출발 또는 문 잠금

    GetWorldTimerManager().SetTimer(UpdateTimerHandle, this, &APMCombatZone::UpdateCombat, UpdateInterval, true);
    StartNextWave();
}

void APMCombatZone::StartNextWave()
{
    bWaitingNextWave = false;
    ++CurrentWaveIndex;

    if (Waves.IsValidIndex(CurrentWaveIndex) == false)
    {
        return;
    }

    // 이번 웨이브 몬스터를 전부 대기열에 넣고 스폰은 한 마리씩 나눠서 처리
    CurrentWaveTotalCount = 0;
    CurrentWaveEnemies.Reset();

    for (const FPMWaveEntry& Entry : Waves[CurrentWaveIndex].Entries)
    {
        if (Entry.EnemyClass == nullptr)
        {
            continue;
        }

        for (int32 Index = 0; Index < Entry.Count; ++Index)
        {
            PendingSpawns.Add(Entry.EnemyClass);
            ++CurrentWaveTotalCount;
        }
    }

    CurrentWaveStartTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Warning, TEXT("Wave %d Start : %d Enemies"), CurrentWaveIndex + 1, CurrentWaveTotalCount);
}

void APMCombatZone::UpdateCombat()
{
    CleanUpEnemyList();
    ProcessPendingSpawns();

    const bool bNoMoreWaves = (CurrentWaveIndex >= Waves.Num() - 1);

    // 전투 종료 : 활성 몬스터 없음 + 스폰 예정 없음 + 남은 웨이브 없음
    if (ActiveEnemies.Num() == 0 && PendingSpawns.Num() == 0 && bNoMoreWaves && bWaitingNextWave == false)
    {
        EndCombat();
        return;
    }

    // 웨이브 전환
    if (bWaitingNextWave == false && bNoMoreWaves == false && ShouldStartNextWave())
    {
        bWaitingNextWave = true;

        // 마지막 웨이브는 예고 시간을 더 길게
        const bool bNextIsFinalWave = (CurrentWaveIndex + 1 == Waves.Num() - 1);
        const float TransitionDelay = bNextIsFinalWave ? FinalWaveNoticeTime : WaveTransitionTime;

        GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &APMCombatZone::StartNextWave, TransitionDelay, false);
    }
}

bool APMCombatZone::ShouldStartNextWave() const
{
    // 아직 스폰하지 못한 몬스터가 남아 있으면 전환하지 않음
    if (PendingSpawns.Num() > 0)
    {
        return false;
    }

    // 조건 1 : 현재 웨이브 몬스터가 2마리 이하
    if (CurrentWaveEnemies.Num() <= NextWaveRemainCount)
    {
        return true;
    }

    // 조건 2 : 현재 웨이브의 75% 이상 처치
    if (CurrentWaveTotalCount > 0)
    {
        const float KilledRatio = 1.f - ((float)CurrentWaveEnemies.Num() / (float)CurrentWaveTotalCount);
        if (KilledRatio >= NextWaveKillRatio)
        {
            return true;
        }
    }

    // 조건 3 : 제한시간 경과
    const float TimeLimit = Waves[CurrentWaveIndex].TimeLimit;
    if (TimeLimit > 0.f && GetWorld()->GetTimeSeconds() - CurrentWaveStartTime >= TimeLimit)
    {
        return true;
    }

    return false;
}

void APMCombatZone::ProcessPendingSpawns()
{
    while (PendingSpawns.Num() > 0 && ActiveEnemies.Num() < MaxActiveEnemies)
    {
        TSubclassOf<APMEnemy> EnemyClass = PendingSpawns[0];
        const APMEnemy* EnemyCDO = EnemyClass ? EnemyClass.GetDefaultObject() : nullptr;
        if (EnemyCDO == nullptr)
        {
            PendingSpawns.RemoveAt(0);
            continue;
        }

        // 이전 웨이브 힐러가 살아 있으면 새 힐러는 추가하지 않음
        if (EnemyCDO->GetEnemyType() == EPMEnemyType::Healer && HasAliveHealer())
        {
            UE_LOG(LogTemp, Warning, TEXT("Healer Spawn Skipped : 이전 힐러 생존"));

            PendingSpawns.RemoveAt(0);
            continue;
        }

        APMSpawnPoint* SpawnPoint = PickSpawnPoint(EnemyClass);
        if (SpawnPoint == nullptr)
        {
            // 지금은 쓸 수 있는 위치가 없음 (플레이어가 너무 가까움) -> 다음 갱신 때 재시도
            break;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APMEnemy* SpawnedEnemy = GetWorld()->SpawnActor<APMEnemy>(
            EnemyClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), SpawnParams);
        if (SpawnedEnemy == nullptr)
        {
            break;
        }

        // 등장 직후 0.5초 동안 공격 금지
        SpawnedEnemy->SetSpawnAttackBlock(SpawnAttackBlockTime);

        ActiveEnemies.Add(SpawnedEnemy);
        CurrentWaveEnemies.Add(SpawnedEnemy);

        UE_LOG(LogTemp, Log, TEXT("Spawned %s (Active %d / %d, Pending %d)"),
            *SpawnedEnemy->GetName(), ActiveEnemies.Num(), MaxActiveEnemies, PendingSpawns.Num());
        PendingSpawns.RemoveAt(0);
    }
}

APMSpawnPoint* APMCombatZone::PickSpawnPoint(TSubclassOf<APMEnemy> EnemyClass) const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    const APMEnemy* EnemyCDO = EnemyClass ? EnemyClass.GetDefaultObject() : nullptr;
    if (PlayerPawn == nullptr || EnemyCDO == nullptr)
    {
        return nullptr;
    }

    const EPMEnemyType WantType = EnemyCDO->GetEnemyType();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const float MinDistSq = MinSpawnDistanceFromPlayer * MinSpawnDistanceFromPlayer;

    TArray<APMSpawnPoint*> Candidates;
    for (APMSpawnPoint* SpawnPoint : SpawnPoints)
    {
        if (SpawnPoint == nullptr || SpawnPoint->CanSpawnType(WantType) == false)
        {
            continue;
        }

        // 플레이어 근처에는 등장하지 않음
        if (FVector::DistSquared(SpawnPoint->GetActorLocation(), PlayerLocation) < MinDistSq)
        {
            continue;
        }

        Candidates.Add(SpawnPoint);
    }

    if (Candidates.Num() == 0)
    {
        return nullptr;
    }

    // 한곳에 몰리지 않도록 무작위 선택
    return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

bool APMCombatZone::HasAliveHealer() const
{
    for (const TWeakObjectPtr<APMEnemy>& WeakEnemy : ActiveEnemies)
    {
        const APMEnemy* Enemy = WeakEnemy.Get();
        if (Enemy != nullptr && Enemy->IsDead() == false && Enemy->GetEnemyType() == EPMEnemyType::Healer)
        {
            return true;
        }
    }

    return false;
}

void APMCombatZone::CleanUpEnemyList()
{
    // 사망하면 Destroy되므로 약참조 자동으로 무효
    auto IsGone = [](const TWeakObjectPtr<APMEnemy>& WeakEnemy)
    {
        const APMEnemy* Enemy = WeakEnemy.Get();
        return Enemy == nullptr || Enemy->IsDead();
    };

    ActiveEnemies.RemoveAll(IsGone);
    CurrentWaveEnemies.RemoveAll(IsGone);
}

void APMCombatZone::EndCombat()
{
    // 클리어한 구역은 다시 시작하지 않음 (bCombatStarted를 true로 유지)
    GetWorldTimerManager().ClearTimer(UpdateTimerHandle);
    GetWorldTimerManager().ClearTimer(WaveTimerHandle);

    UE_LOG(LogTemp, Warning, TEXT("Combat Cleared"));

    // 추후 조정필요 : 클리어 시 다음 지하철 도착 / 문 열기
}