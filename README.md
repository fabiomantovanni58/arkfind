# arkfind

Plugin para ARK: Survival Evolved (ArkAPI) que permite encontrar e rastrear dinossauros no mapa.

## Funcionalidades

- **Busca por nome**: Use `/finddino <nome>` ou `/fd <nome>` para buscar dinossauros por nome parcial ou completo
- **Lista numerada**: Resultados exibidos no chat com números `[1]Rex`, `[2]Rex Alpha`, etc.
- **Rastreamento em tempo real**: Digite `/fd` para ver direção (seta), distância e nome do dinossauro
- **Sem lag**: A busca só é executada quando o comando é usado
- **Notificações**: Informações exibidas no topo da tela com formato: `REX | ⬆️ | 850m`

## Comandos

| Comando | Descrição |
|---------|-----------|
| `/finddino <nome>` | Busca dinossauros pelo nome |
| `/fd <nome>` | Atalho para `/finddino` |
| `/fd <número>` | Seleciona um dinossauro da última busca para rastrear |
| `/fd` | Mostra informação do rastreamento atual |

## Exemplo de Uso

```
Player: /finddino Rex
Bot: === Dinossauros Encontrados ===
Bot: [1] Rex - 450m
Bot: [2] Rex Alpha - 1200m
Bot: Digite /fd <número> para rastrear um dinossauro específico.

Player: /fd 1
Bot: Rastreamento iniciado para: Rex

[Notificação no topo]: Rex | ⬆️ | 450m
[Notificação no topo]: Rex | ↗️ | 445m
[Notificação no topo]: Rex | ⬆️ | 440m
```

## Instalação

### Pré-requisitos

- ARK: Survival Evolved dedicado server
- ArkAPI instalado
- Compilador C++ (MSVC para Windows, GCC para Linux)

### Compilação

1. Clone este repositório:
```bash
git clone https://github.com/fabiomantovanni58/arkfind.git
cd arkfind
```

2. Clone o ArkApi na pasta pai:
```bash
git clone https://github.com/ArkApi/ArkApi.git ../ArkApi
```

3. Compile:

**Windows:**
```bash
cmake -B build -S .
cmake --build build --config Release
```

**Linux:**
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

4. Copie o arquivo gerado para a pasta de plugins do ArkApi:
- Windows: `build/bin/DinoFinder.dll` → `ArkApi/Plugins/`
- Linux: `build/bin/libDinoFinder.so` → `ArkApi/Plugins/`

### Usando GitHub Actions

O plugin será compilado automaticamente quando você fizer push para o repositório:

1. Faça push do código para o GitHub
2. Vá até a aba **Actions** no GitHub
3. Selecione o workflow mais recente
4. Baixe o artefato correspondente ao seu sistema operacional
5. Extraia e coloque na pasta de plugins do ArkApi

## Configuração

Edite o arquivo `Plugins.ini` do ArkApi para carregar o plugin:

```ini
[Plugins]
Plugin=DinoFinder
```

## Direções das Setas

| Seta | Direção | Ângulo |
|------|---------|--------|
| ⬆️ | Frente | -22° a 22° |
| ↗️ | Frente-Direita | 22° a 67° |
| ➡️ | Direita | 67° a 112° |
| ↘️ | Trás-Direita | 112° a 157° |
| ⬇️ | Trás | 157° a 180° ou -180° a -157° |
| ↙️ | Trás-Esquerda | -157° a -112° |
| ⬅️ | Esquerda | -112° a -67° |
| ↖️ | Frente-Esquerda | -67° a -22° |

## Notas

- Funciona apenas em servidores privados
- Requer permissões de administrador para usar os comandos
- A busca é limitada a 20 resultados para evitar spam no chat
- O rastreamento atualiza a cada 1 segundo

## Licença

MIT License

## Contribuição

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues e pull requests.