#pragma once

#include "API/ARK/Ark.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <cmath>

class DinoFinder : public ARK::Plugin
{
public:
    void OnEnable() override;
    void OnDisable() override;

private:
    // Comandos
    static bool CmdFindDino(AShooterPlayerController* player, FString message);
    static bool CmdFd(AShooterPlayerController* player, FString message);

    // Lógica de busca
    static std::vector<AActor*> FindDinosByName(const FString& nameFilter);
    static void ShowDinoList(AShooterPlayerController* player, const std::vector<AActor*>& dinos);
    
    // Rastreamento
    static void StartTracking(AShooterPlayerController* player, AActor* target);
    static void StopTracking(AShooterPlayerController* player);
    static void UpdateTracking();

    // Utilitários
    static FString GetDinoName(AActor* dino);
    static float CalculateDistance(AActor* from, AActor* to);
    static FString GetDirectionArrow(float angle);
    static void SendNotify(AShooterPlayerController* player, const FString& message);
    static void SendChatMessage(AShooterPlayerController* player, const FString& message);

    // Estado do rastreamento
    static std::unordered_map<uint64, AActor*> trackingTargets; // PlayerID -> Dino
    static std::unordered_map<uint64, std::vector<AActor*>> lastSearchResults; // PlayerID -> Lista de dinos
    static int timerHandle;
};
