// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"



void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage > 0.f)
    {
        CurrentHealth -= ActualDamage;

        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Ouch! HP: %f"), CurrentHealth));

        if (CurrentHealth <= 0.f)
        {
            Destroy();
        }
    }

    return ActualDamage;
}