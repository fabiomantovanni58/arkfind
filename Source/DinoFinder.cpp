#include "DinoFinder.h"
#include "API/ARK/ArkApi.h"
#include <sstream>

// Variáveis estáticas
std::unordered_map<uint64, AActor*> DinoFinder::trackingTargets;
std::unordered_map<uint64, std::vector<AActor*>> DinoFinder::lastSearchResults;
int DinoFinder::timerHandle = -1;

void DinoFinder::OnEnable()
{
    // Registrar comandos
    ArkApi::GetCommands().AddChatCommand("/finddino", &CmdFindDino);
    ArkApi::GetCommands().AddChatCommand("/fd", &CmdFd);
    
    // Iniciar timer para atualização do rastreamento (a cada 1 segundo)
    timerHandle = FTimerHandle();
    ASHooterGameMode* gameMode = static_cast<ASHooterGameMode*>(UGameplayStatics::GetGameMode(nullptr));
    if (gameMode)
    {
        gameMode->GetWorldTimerManager().SetTimer(
            timerHandle,
            []() { UpdateTracking(); },
            1.0f,
            true
        );
    }
}

void DinoFinder::OnDisable()
{
    // Remover comandos
    ArkApi::GetCommands().RemoveChatCommand("/finddino");
    ArkApi::GetCommands().RemoveChatCommand("/fd");
    
    // Parar timer
    ASHooterGameMode* gameMode = static_cast<ASHooterGameMode*>(UGameplayStatics::GetGameMode(nullptr));
    if (gameMode && timerHandle != -1)
    {
        gameMode->GetWorldTimerManager().ClearTimer(timerHandle);
    }
    
    trackingTargets.clear();
    lastSearchResults.clear();
}

bool DinoFinder::CmdFindDino(AShooterPlayerController* player, FString message)
{
    if (!player) return false;
    
    FString playerName;
    player->GetPlayerName(playerName);
    
    // Extrair nome do dino após o comando
    FString dinoName = message.RightChop(10); // Remove "/finddino "
    dinoName = dinoName.TrimStartAndEnd();
    
    if (dinoName.IsEmpty())
    {
        SendChatMessage(player, "{FFFFFF}Uso: /finddino <nome_do_dino>");
        SendChatMessage(player, "{FFFFFF}Exemplo: /finddino Rex");
        return false;
    }
    
    // Buscar dinossauros
    std::vector<AActor*> dinos = FindDinosByName(dinoName);
    
    if (dinos.empty())
    {
        SendChatMessage(player, "{FF0000}Nenhum dinossauro encontrado com o nome: " + dinoName.ToString());
        return false;
    }
    
    // Armazenar resultados para rastreamento posterior
    uint64 playerId = player->GetUniqueNetId().GetValue();
    lastSearchResults[playerId] = dinos;
    
    // Mostrar lista numerada
    ShowDinoList(player, dinos);
    
    // Se encontrou apenas um, iniciar rastreamento automaticamente
    if (dinos.size() == 1)
    {
        StartTracking(player, dinos[0]);
        SendChatMessage(player, "{00FF00}Rastreamento iniciado automaticamente!");
    }
    else
    {
        SendChatMessage(player, "{FFFF00}Digite /fd <número> para rastrear um dinossauro específico.");
    }
    
    return false;
}

bool DinoFinder::CmdFd(AShooterPlayerController* player, FString message)
{
    if (!player) return false;
    
    uint64 playerId = player->GetUniqueNetId().GetValue();
    
    // Verificar se há argumento (número para selecionar da última busca)
    FString arg = message.RightChop(3); // Remove "/fd"
    arg = arg.TrimStartAndEnd();
    
    if (!arg.IsEmpty())
    {
        // Tentar parsear número
        int selectedIndex = FCString::Atoi(*arg);
        
        auto it = lastSearchResults.find(playerId);
        if (it != lastSearchResults.end() && !it->second.empty())
        {
            if (selectedIndex > 0 && selectedIndex <= static_cast<int>(it->second.size()))
            {
                StartTracking(player, it->second[selectedIndex - 1]);
                SendChatMessage(player, "{00FF00}Rastreamento iniciado para: " + GetDinoName(it->second[selectedIndex - 1]).ToString());
                return false;
            }
            else
            {
                SendChatMessage(player, "{FF0000}Número inválido. Use um número entre 1 e " + FString::FromInt(it->second.size()));
                return false;
            }
        }
        else
        {
            SendChatMessage(player, "{FF0000}Nenhuma busca anterior encontrada. Use /finddino primeiro.");
            return false;
        }
    }
    else
    {
        // Apenas /fd - mostrar status do rastreamento atual ou iniciar do último resultado
        auto trackIt = trackingTargets.find(playerId);
        if (trackIt != trackingTargets.end() && trackIt->second)
        {
            // Já está rastreando, mostrar informação imediata
            float dist = CalculateDistance(player->GetPawn(), trackIt->second);
            FString name = GetDinoName(trackIt->second);
            SendNotify(player, name + " | " + FString::SanitizeFloat(dist, 0) + "m");
        }
        else
        {
            // Tentar iniciar do primeiro resultado da última busca
            auto searchIt = lastSearchResults.find(playerId);
            if (searchIt != lastSearchResults.end() && !searchIt->second.empty())
            {
                StartTracking(player, searchIt->second[0]);
                SendChatMessage(player, "{00FF00}Rastreamento iniciado para: " + GetDinoName(searchIt->second[0]).ToString());
            }
            else
            {
                SendChatMessage(player, "{FFFFFF}Uso: /fd <número> para rastrear um dinossauro da sua última busca.");
                SendChatMessage(player, "{FFFFFF}Ou use /finddino <nome> para buscar novos dinossauros.");
            }
        }
    }
    
    return false;
}

std::vector<AActor*> DinoFinder::FindDinosByName(const FString& nameFilter)
{
    std::vector<AActor*> results;
    
    // Obter todos os atores do tipo PrimalDinoCharacter
    TArray<AActor*> foundActors;
    UGameplayStatics::GetAllActorsOfClass(nullptr, APrimalDinoCharacter::StaticClass(), foundActors);
    
    FString filterLower = nameFilter.ToLower();
    
    for (AActor* actor : foundActors)
    {
        if (!actor) continue;
        
        FString dinoName = GetDinoName(actor);
        FString dinoNameLower = dinoName.ToLower();
        
        // Busca parcial (case insensitive)
        if (dinoNameLower.Contains(filterLower))
        {
            results.push_back(actor);
        }
    }
    
    return results;
}

void DinoFinder::ShowDinoList(AShooterPlayerController* player, const std::vector<AActor*>& dinos)
{
    SendChatMessage(player, "{00FFFF}=== Dinossauros Encontrados ===");
    
    int index = 1;
    for (AActor* dino : dinos)
    {
        if (!dino) continue;
        
        FString name = GetDinoName(dino);
        float dist = CalculateDistance(player->GetPawn(), dino);
        
        FString msg = "{" + FString::FromInt(index) + "} " + name + " - " + FString::SanitizeFloat(dist, 0) + "m";
        SendChatMessage(player, msg);
        
        index++;
        if (index > 20) // Limitar a 20 resultados
        {
            SendChatMessage(player, "{FFFF00}... e mais " + FString::FromInt(dinos.size() - 20) + " dinossauros.");
            break;
        }
    }
}

void DinoFinder::StartTracking(AShooterPlayerController* player, AActor* target)
{
    if (!player || !target) return;
    
    uint64 playerId = player->GetUniqueNetId().GetValue();
    trackingTargets[playerId] = target;
    
    SendChatMessage(player, "{00FF00}Rastreamento iniciado! Digite /fd para ver a direção.");
}

void DinoFinder::StopTracking(AShooterPlayerController* player)
{
    if (!player) return;
    
    uint64 playerId = player->GetUniqueNetId().GetValue();
    trackingTargets.erase(playerId);
    
    SendChatMessage(player, "{FF0000}Rastreamento parado.");
}

void DinoFinder::UpdateTracking()
{
    // Iterar sobre todos os jogadores que estão rastreando
    for (auto it = trackingTargets.begin(); it != trackingTargets.end(); )
    {
        uint64 playerId = it->first;
        AActor* target = it->second;
        
        // Verificar se o alvo ainda existe
        if (!target || target->IsPendingKillOrUnreachable())
        {
            it = trackingTargets.erase(it);
            continue;
        }
        
        // Obter o jogador
        AShooterPlayerController* player = static_cast<AShooterPlayerController*>(
            UGameplayStatics::GetPlayerController(nullptr, 0)
        );
        
        // Encontrar o controller correto pelo PlayerID
        for (FConstPlayerControllerIterator controllerIt = UGameplayStatics::GetGameMode(nullptr)->GetWorld()->GetPlayerControllerIterator(); controllerIt; ++controllerIt)
        {
            AShooterPlayerController* pc = static_cast<AShooterPlayerController*>(controllerIt->Get());
            if (pc && pc->GetUniqueNetId().GetValue() == playerId)
            {
                player = pc;
                break;
            }
        }
        
        if (!player || !player->GetPawn())
        {
            it = trackingTargets.erase(it);
            continue;
        }
        
        // Calcular distância e direção
        float dist = CalculateDistance(player->GetPawn(), target);
        FString name = GetDinoName(target);
        
        // Calcular ângulo
        FVector playerLoc = player->GetPawn()->K2_GetActorLocation();
        FVector targetLoc = target->K2_GetActorLocation();
        FVector direction = (targetLoc - playerLoc).GetSafeNormal();
        
        FRotator playerRot = player->GetControlRotation();
        FVector forward = playerRot.Vector();
        
        float dot = FVector::DotProduct(forward, direction);
        float angle = FMath::Acos(dot) * (180.0f / PI);
        
        // Determinar se está à esquerda ou direita
        FVector right = playerRot.GetRightVector();
        float cross = FVector::DotProduct(right, direction);
        if (cross < 0) angle = -angle;
        
        FString arrow = GetDirectionArrow(angle);
        
        // Enviar notificação
        FString notifyMsg = name + " | " + arrow + " | " + FString::SanitizeFloat(dist, 0) + "m";
        SendNotify(player, notifyMsg);
        
        ++it;
    }
}

FString DinoFinder::GetDinoName(AActor* dino)
{
    if (!dino) return TEXT("Desconhecido");
    
    APrimalDinoCharacter* dinoChar = static_cast<APrimalDinoCharacter*>(dino);
    if (!dinoChar) return TEXT("Desconhecido");
    
    FString displayName;
    dinoChar->GetDisplayName(displayName);
    
    if (displayName.IsEmpty())
    {
        // Fallback para o nome da classe
        return dino->GetClass()->GetName();
    }
    
    return displayName;
}

float DinoFinder::CalculateDistance(AActor* from, AActor* to)
{
    if (!from || !to) return 0.0f;
    
    FVector locFrom = from->K2_GetActorLocation();
    FVector locTo = to->K2_GetActorLocation();
    
    return FVector::Dist(locFrom, locTo);
}

FString DinoFinder::GetDirectionArrow(float angle)
{
    // Converter ângulo para seta
    // -180 a -157: ⬅️
    // -157 a -112: ↙️
    // -112 a -67: ⬇️
    // -67 a -22: ↘️
    // -22 a 22: ⬆️
    // 22 a 67: ↗️
    // 67 a 112: ➡️
    // 112 a 157: ↖️
    // 157 a 180: ⬅️
    
    if (angle >= -22 && angle <= 22) return TEXT("⬆️");
    if (angle > 22 && angle <= 67) return TEXT("↗️");
    if (angle > 67 && angle <= 112) return TEXT("➡️");
    if (angle > 112 && angle <= 157) return TEXT("↘️");
    if (angle > 157 || angle <= -157) return TEXT("⬇️");
    if (angle > -157 && angle <= -112) return TEXT("↙️");
    if (angle > -112 && angle <= -67) return TEXT("⬅️");
    if (angle > -67 && angle <= -22) return TEXT("↖️");
    
    return TEXT("⬆️");
}

void DinoFinder::SendNotify(AShooterPlayerController* player, const FString& message)
{
    if (!player) return;
    
    // Usar o sistema de notificação do ARK
    player->ClientStatusMessage(message, EStatusMessageType::Notice);
}

void DinoFinder::SendChatMessage(AShooterPlayerController* player, const FString& message)
{
    if (!player) return;
    
    // Adicionar cor e enviar para o chat
    FString coloredMsg = message;
    player->ClientMessage(coloredMsg);
}

// Exportar plugin
DECLARE_PLUGIN(DinoFinder)
IMPLEMENT_PLUGIN(DinoFinder)
