# DinoFinder Plugin para ARK: Survival Evolved

Plugin desenvolvido com **ArkAPI** que permite encontrar e rastrear dinossauros no mapa através de comandos de chat.

## Funcionalidades

- 🔍 **Busca por nome**: Digite `/finddino <nome>` ou `/fd <nome>` para buscar dinossauros por nome (parcial ou completo)
- 📋 **Lista numerada**: Resultados exibidos no chat com numeração `[1]Rex`, `[2]Rex Alpha`, etc.
- 🧭 **Rastreamento em tempo real**: Digite apenas `/fd` após uma busca para ativar o rastreamento
- 📍 **Direção e distância**: Notificação mostra nome, seta direcional e distância em metros
  - Exemplo: `REX | ⬆️ | 850m`

## Comandos

| Comando | Descrição |
|---------|-----------|
| `/finddino <nome>` | Busca dinossauros pelo nome |
| `/fd <nome>` | Atalho para busca |
| `/fd` | Inicia/para rastreamento do último dino encontrado |

## Setas Direcionais

O plugin usa emojis para indicar a direção relativa ao jogador:

- ⬆️ = Frente
- ↗️ = Frente-Direita
- ➡️ = Direita
- ↘️ = Trás-Direita
- ⬇️ = Trás
- ↙️ = Trás-Esquerda
- ⬅️ = Esquerda
- ↖️ = Frente-Esquerda

## Instalação

### Pré-requisitos

- ARK: Survival Evolved servidor dedicado
- ArkAPI instalado no servidor
- Compilador C++ (MSVC para Windows, GCC para Linux)

### Passos

1. **Clone este repositório**:
   ```bash
   git clone https://github.com/SEU_USUARIO/DinoFinder.git
   cd DinoFinder
   ```

2. **Clone o ArkAPI** (se ainda não tiver):
   ```bash
   git clone https://github.com/ArkApi/ArkApi.git ../ArkApi
   ```

3. **Compile o plugin**:

   **Windows:**
   ```bash
   cmake -B build -S .
   cmake --build build --config Release
   ```

   **Linux:**
   ```bash
   cmake -B build -S .
   cmake --build build --config Release
   ```

4. **Copie o arquivo compilado**:
   - Windows: Copie `build/DinoFinder.dll` para `ShooterGame/Binaries/Win64/Plugins/`
   - Linux: Copie `build/DinoFinder.so` para `ShooterGame/Binaries/Linux/Plugins/`

5. **Ative o plugin**:
   Adicione no seu `GameUserSettings.ini`:
   ```ini
   [/Script/ShooterGame.ShooterGameUserSettings]
   +ActivePlugins=DinoFinder.dll
   ```

## Uso no Servidor

1. Entre no servidor como administrador
2. No chat, digite: `/fd rex` (ou qualquer outro nome de dino)
3. Veja a lista de dinossauros encontrados
4. Digite `/fd` novamente para iniciar o rastreamento
5. As notificações aparecerão no topo da tela a cada segundo

## Build Automático (CI/CD)

Este repositório inclui um workflow do GitHub Actions que compila automaticamente o plugin para Windows e Linux a cada push.

### Para baixar builds automáticas:

1. Vá até a aba **Actions** no GitHub
2. Selecione o workflow mais recente
3. Baixe o artifact correspondente ao seu sistema operacional

### Criar uma release:

```bash
git tag v1.0.0
git push origin v1.0.0
```

Isso triggerará o workflow que criará uma release automática com os binários.

## Estrutura do Projeto

```
DinoFinder/
├── Source/
│   ├── DinoFinder.h      # Header file
│   └── DinoFinder.cpp    # Implementação
├── .github/
│   └── workflows/
│       └── build.yml     # CI/CD configuration
├── CMakeLists.txt        # Configuração de build
└── README.md             # Este arquivo
```

## Notas Importantes

- ⚠️ **Use apenas em servidores privados** onde você tem permissão de administrador
- ⚠️ O uso em servidores públicos pode violar as regras do servidor
- 🎯 A busca é feita apenas quando o comando é executado, evitando lag constante
- 🔄 O rastreamento atualiza a cada 1 segundo para manter performance

## Desenvolvimento

Para desenvolver novas funcionalidades:

1. Edite os arquivos em `Source/`
2. Recompile seguindo os passos acima
3. Teste em um servidor local antes de deploy

## Licença

MIT License - Sinta-se livre para modificar e distribuir.

## Suporte

Para issues ou sugestões, abra uma issue neste repositório.

---

**Divirta-se encontrando seus dinossauros!** 🦕🦖
