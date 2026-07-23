#include "DinoFinder.h"
#include "ApiUtils.h"
#include <cmath>
#include <algorithm>

// Singleton implementation
DinoFinderPlugin& DinoFinderPlugin::GetInstance()
{
    static DinoFinderPlugin instance;
    return instance;
}

void DinoFinderPlugin::Init()
{
    // Registrar comando de chat
    ArkApi::GetCommands().AddChatCommand("/finddino", &DinoFinderPlugin::HandleChatMessage, this);
    ArkApi::GetCommands().AddChatCommand("/fd", &DinoFinderPlugin::HandleChatMessage, this);

    // Timer para atualização do rastreamento (1 segundo)
    timer_handle_ = FArkApi::Get().SetTimer(1.0f, true, [this]() { UpdateTracking(); });
}

void DinoFinderPlugin::Unload()
{
    ArkApi::GetCommands().RemoveChatCommand("/finddino");
    ArkApi::GetCommands().RemoveChatCommand("/fd");
    
    if (timer_handle_ != 0)
    {
        FArkApi::Get().ClearTimer(timer_handle_);
    }
    
    player_data_.clear();
}

void DinoFinderPlugin::HandleChatMessage(AShooterPlayerController* player_controller, FString* message, EChatSendMode::Type mode)
{
    if (!player_controller || !message)
        return;

    uint64 player_id = ApiUtils::GetSteamIdFromController(player_controller);
    std::string msg = message->ToString();

    // Remover o comando e espaços extras
    size_t first_space = msg.find(' ');
    std::string search_term = "";
    
    if (first_space != std::string::npos && first_space + 1 < msg.length())
    {
        search_term = msg.substr(first_space + 1);
        // Trim spaces
        size_t start = search_term.find_first_not_of(" \t");
        size_t end = search_term.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos)
            search_term = search_term.substr(start, end - start + 1);
        else
            search_term = "";
    }

    if (!search_term.empty())
    {
        // Busca por dinos
        FindDinos(player_controller, search_term);
    }
    else
    {
        // Sem termo: mostra lista ou inicia rastreamento
        auto it = player_data_.find(player_id);
        if (it != player_data_.end() && !it->second.found_dinos.empty())
        {
            if (it->second.is_tracking)
            {
                StopTracking(player_controller);
                SendChatMessage(player_controller, "Rastreamento parado.");
            }
            else
            {
                StartTracking(player_controller);
            }
        }
        else
        {
            SendChatMessage(player_controller, "Uso: /finddino <nome_do_dino> ou /fd <nome_do_dino>");
            SendChatMessage(player_controller, "Depois digite apenas /fd para rastrear o ultimo dino encontrado.");
        }
    }
}

void DinoFinderPlugin::FindDinos(AShooterPlayerController* player_controller, const std::string& search_term)
{
    if (!player_controller)
        return;

    uint64 player_id = ApiUtils::GetSteamIdFromController(player_controller);
    auto& data = player_data_[player_id];
    data.found_dinos.clear();
    data.tracking_index = -1;
    data.is_tracking = false;

    TArray<AActor*> found_actors;
    UGameplayStatics::GetAllActorsOfClass(player_controller->GetWorld(), APrimalDinoCharacter::StaticClass(), found_actors);

    std::string search_lower = search_term;
    std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);

    for (AActor* actor : found_actors)
    {
        APrimalDinoCharacter* dino = Cast<APrimalDinoCharacter>(actor);
        if (!dino || dino->IsDead())
            continue;

        std::string dino_name = GetDinoName(dino);
        std::string dino_name_lower = dino_name;
        std::transform(dino_name_lower.begin(), dino_name_lower.end(), dino_name_lower.begin(), ::tolower);

        if (dino_name_lower.find(search_lower) != std::string::npos)
        {
            data.found_dinos.push_back(dino);
        }
    }

    // Ordenar por distância
    FVector player_loc = player_controller->GetPawn()->GetActorLocation();
    std::sort(data.found_dinos.begin(), data.found_dinos.end(), 
        [player_loc](APrimalDinoCharacter* a, APrimalDinoCharacter* b)
        {
            float dist_a = FVector::Dist(player_loc, a->GetActorLocation());
            float dist_b = FVector::Dist(player_loc, b->GetActorLocation());
            return dist_a < dist_b;
        });

    if (data.found_dinos.empty())
    {
        SendChatMessage(player_controller, "Nenhum dinossauro encontrado com o nome: " + search_term);
        return;
    }

    // Enviar lista (max 10 para não spamar)
    SendChatMessage(player_controller, "=== Dinossauros Encontrados ===");
    int count = std::min(static_cast<int>(data.found_dinos.size()), 10);
    for (int i = 0; i < count; i++)
    {
        std::string name = GetDinoName(data.found_dinos[i]);
        float dist = CalculateDistance(player_controller->GetPawn(), data.found_dinos[i]);
        std::string msg = "[" + std::to_string(i + 1) + "] " + name + " - " + std::to_string(static_cast<int>(dist)) + "m";
        SendChatMessage(player_controller, msg);
    }

    if (data.found_dinos.size() > 10)
    {
        SendChatMessage(player_controller, "... e mais " + std::to_string(data.found_dinos.size() - 10) + " dinossauros.");
    }

    SendChatMessage(player_controller, "Digite /fd para começar a rastrear o mais próximo!");
}

void DinoFinderPlugin::ListDinos(AShooterPlayerController* player_controller)
{
    // Implementado dentro de HandleChatMessage quando sem argumento
}

void DinoFinderPlugin::StartTracking(AShooterPlayerController* player_controller)
{
    if (!player_controller)
        return;

    uint64 player_id = ApiUtils::GetSteamIdFromController(player_controller);
    auto it = player_data_.find(player_id);
    
    if (it == player_data_.end() || it->second.found_dinos.empty())
        return;

    it->second.is_tracking = true;
    if (it->second.tracking_index < 0 || it->second.tracking_index >= static_cast<int>(it->second.found_dinos.size()))
    {
        it->second.tracking_index = 0; // Começa pelo mais próximo
    }

    SendNotify(player_controller, "Rastreamento iniciado!");
}

void DinoFinderPlugin::StopTracking(AShooterPlayerController* player_controller)
{
    if (!player_controller)
        return;

    uint64 player_id = ApiUtils::GetSteamIdFromController(player_controller);
    auto it = player_data_.find(player_id);
    
    if (it != player_data_.end())
    {
        it->second.is_tracking = false;
    }
}

void DinoFinderPlugin::UpdateTracking()
{
    for (auto& [player_id, data] : player_data_)
    {
        if (!data.is_tracking || data.tracking_index < 0 || data.tracking_index >= static_cast<int>(data.found_dinos.size()))
            continue;

        APrimalDinoCharacter* target = data.found_dinos[data.tracking_index];
        if (!target || target->IsDead())
        {
            data.is_tracking = false;
            continue;
        }

        AShooterPlayerController* pc = ArkApi::GetApiUtils().FindControllerFromSteamId(player_id);
        if (!pc || !pc->GetPawn())
            continue;

        std::string dino_name = GetDinoName(target);
        float distance = CalculateDistance(pc->GetPawn(), target);
        float angle = CalculateAngle(pc->GetPawn(), target);
        std::string arrow = GetDirectionArrow(angle);

        std::string notify_msg = dino_name + " | " + arrow + " | " + std::to_string(static_cast<int>(distance)) + "m";
        SendNotify(pc, notify_msg);
    }
}

std::string DinoFinderPlugin::GetDinoName(APrimalDinoCharacter* dino)
{
    if (!dino)
        return "Desconhecido";

    // Tenta pegar o nome customizado primeiro
    FString dino_name_fstr = dino->GetCustomName();
    if (!dino_name_fstr.IsEmpty())
    {
        return dino_name_fstr.ToString();
    }

    // Se não tiver nome customizado, usa o nome da classe
    FString class_name = dino->GetClass()->GetName();
    std::string name = class_name.ToString();
    
    // Remove prefixos comuns como "BP_Study_" ou "Dino_"
    size_t pos = name.find("BP_");
    if (pos != std::string::npos)
        name = name.substr(pos + 3);
    
    pos = name.find("Dino_");
    if (pos != std::string::npos)
        name = name.substr(pos + 5);

    // Formatar nome (ex: Rex_Character_BP_C -> Rex)
    size_t underscore = name.find('_');
    if (underscore != std::string::npos)
        name = name.substr(0, underscore);

    return name;
}

float DinoFinderPlugin::CalculateDistance(AActor* actor1, AActor* actor2)
{
    if (!actor1 || !actor2)
        return 0.0f;

    FVector loc1 = actor1->GetActorLocation();
    FVector loc2 = actor2->GetActorLocation();

    return FVector::Dist(loc1, loc2);
}

float DinoFinderPlugin::CalculateAngle(AActor* from, AActor* to)
{
    if (!from || !to)
        return 0.0f;

    FVector from_loc = from->GetActorLocation();
    FVector to_loc = to->GetActorLocation();
    FRotator from_rot = from->GetActorRotation();

    // Vetor direção para o alvo
    FVector direction = (to_loc - from_loc).GetSafeNormal();
    
    // Vetor para onde o jogador está olhando (apenas yaw no plano horizontal)
    FVector forward = from_rot.Vector();
    forward.Z = 0;
    forward.Normalize();

    // Calcular ângulo entre os vetores
    float dot_product = FVector::DotProduct(forward, direction);
    float angle = FMath::Acos(dot_product) * (180.0f / PI);

    // Determinar se é esquerda ou direita usando produto vetorial
    FVector cross = FVector::CrossProduct(forward, direction);
    if (cross.Z < 0)
    {
        angle = -angle;
    }

    return angle;
}

std::string DinoFinderPlugin::GetDirectionArrow(float angle)
{
    // Ângulo: 0 = frente, 180/-180 = trás, 90 = direita, -90 = esquerda
    if (angle >= -22.5f && angle <= 22.5f)
        return "⬆️";  // Frente
    else if (angle > 22.5f && angle <= 67.5f)
        return "↗️";  // Frente-Direita
    else if (angle > 67.5f && angle <= 112.5f)
        return "➡️";  // Direita
    else if (angle > 112.5f && angle <= 157.5f)
        return "↘️";  // Trás-Direita
    else if (angle > 157.5f || angle <= -157.5f)
        return "⬇️";  // Trás
    else if (angle > -157.5f && angle <= -112.5f)
        return "↙️";  // Trás-Esquerda
    else if (angle > -112.5f && angle <= -67.5f)
        return "⬅️";  // Esquerda
    else if (angle > -67.5f && angle <= -22.5f)
        return "↖️";  // Frente-Esquerda
    
    return "❓";
}

void DinoFinderPlugin::SendNotify(AShooterPlayerController* player_controller, const std::string& message)
{
    if (!player_controller)
        return;

    FString msg_fstr(message.c_str());
    
    // Usa o sistema de notificação do ARK (topo da tela)
    // Nota: Em algumas versões pode ser necessário usar ClientMessage
    player_controller->ClientMessage(msg_fstr, FName("Notification"), FLinearColor::White, 2.0f);
}

void DinoFinderPlugin::SendChatMessage(AShooterPlayerController* player_controller, const std::string& message)
{
    if (!player_controller)
        return;

    FString msg_fstr(message.c_str());
    ArkApi::GetApiUtils().SendChatMessage(player_controller, L"DinoFinder", *msg_fstr);
}

// Exportação necessária para o loader do ArkAPI
#ifdef _WIN32
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API
#endif

extern "C" PLUGIN_API void Init()
{
    DinoFinderPlugin::GetInstance().Init();
}

extern "C" PLUGIN_API void Unload()
{
    DinoFinderPlugin::GetInstance().Unload();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        DinoFinderPlugin::GetInstance().Unload();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
