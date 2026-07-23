#pragma once

#include "ArkApi.h"
#include <unordered_map>
#include <string>
#include <vector>

class DinoFinderPlugin
{
public:
    static DinoFinderPlugin& GetInstance();

    void Init();
    void Unload();

private:
    DinoFinderPlugin() = default;
    ~DinoFinderPlugin() = default;

    // Comandos
    void HandleChatMessage(AShooterPlayerController* player_controller, FString* message, EChatSendMode::Type mode);
    
    // Lógica de busca
    void FindDinos(AShooterPlayerController* player_controller, const std::string& search_term);
    void ListDinos(AShooterPlayerController* player_controller);
    
    // Lógica de rastreamento
    void StartTracking(AShooterPlayerController* player_controller);
    void StopTracking(AShooterPlayerController* player_controller);
    void UpdateTracking();

    // Utilitários
    std::string GetDinoName(APrimalDinoCharacter* dino);
    float CalculateDistance(AActor* actor1, AActor* actor2);
    float CalculateAngle(AActor* from, AActor* to);
    std::string GetDirectionArrow(float angle);
    void SendNotify(AShooterPlayerController* player_controller, const std::string& message);
    void SendChatMessage(AShooterPlayerController* player_controller, const std::string& message);

    // Estado do plugin
    struct PlayerData
    {
        std::vector<APrimalDinoCharacter*> found_dinos;
        int tracking_index = -1;
        bool is_tracking = false;
    };

    std::unordered_map<uint64, PlayerData> player_data_;
    uint64 timer_handle_ = 0;

    // Singleton
    DinoFinderPlugin(const DinoFinderPlugin&) = delete;
    DinoFinderPlugin& operator=(const DinoFinderPlugin&) = delete;
};
