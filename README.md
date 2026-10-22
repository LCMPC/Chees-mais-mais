# Dependências do Chees mais mais

Verificação em 22/10/2026, no Windows x86_64 deste projeto.

## Ambiente local verificado

As dependências já estavam instaladas; não foi necessário instalar ou atualizar pacotes.

| Componente | Versão / localização |
| --- | --- |
| GCC | 16.1.0; `C:/msys64/mingw64/bin/gcc.exe` |
| GNU Make | 4.4.1; `mingw32-make.exe` |
| raylib | 5.5; pkg-config reporta 5.5.0 |
| pkg-config | Disponível; encontra cabeçalhos e bibliotecas da raylib |
| Git | Disponível |

Compilação do menu existente, a partir da raiz do projeto, no PowerShell:

```powershell
gcc -std=c99 -Wall -Wextra src/main.c -o tmp/pdfs/menu_verificado.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

Resultado: compilou sem avisos. Também foi compilado e executado um programa mínimo que chama `ColorToInt` e confirma o resultado esperado. Isso verifica cabeçalho, vínculo e carregamento da biblioteca; a janela gráfica não foi aberta. Os executáveis de verificação ficam em `tmp/pdfs/`.

## Windows: preparar outra máquina

Usar o terminal **MSYS2 MinGW64** com pacotes da mesma arquitetura. Em uma instalação de MSYS2 atualizada conforme sua documentação oficial:

```sh
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-raylib mingw-w64-x86_64-pkgconf
gcc --version
mingw32-make --version
pkg-config --modversion raylib
```

O nome do Make no Windows é `mingw32-make`; comandos futuros do plano como `make run` poderão ser executados como `mingw32-make run`. O Makefile ainda será desenvolvido pelo grupo.

No PowerShell, é preciso ter `C:/msys64/mingw64/bin` no PATH da sessão para localizar as ferramentas e a DLL. No terminal MinGW64, o ambiente já fornece esses caminhos. Ao distribuir o jogo, decidir entre vínculo estático ou distribuição das DLLs requeridas.

## Linux: referência Ubuntu/Debian

Instruções para os colegas executarem nas próprias máquinas; não foram executadas neste Windows. Instalar ferramentas e bibliotecas de desenvolvimento para a raylib:

```sh
sudo apt update
sudo apt install build-essential git pkg-config libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

Se a distribuição não oferecer a versão de raylib combinada pelo grupo, uma opção para reproduzir a base local 5.5 é compilar a versão marcada no repositório oficial. Executar em uma pasta de ferramentas, fora dos fontes do jogo:

```sh
git clone --branch 5.5 --depth 1 https://github.com/raysan5/raylib.git raylib-5.5
make -C raylib-5.5/src PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC
sudo make -C raylib-5.5/src install RAYLIB_LIBTYPE=STATIC
pkg-config --modversion raylib
```

Confirmar o diretório de instalação e os parâmetros para a distribuição usada. Uma instalação via pacote da distribuição pode ser mais conveniente. O Makefile do jogo deverá obter opções da raylib com `pkg-config`, incluindo dependências estáticas quando necessário.

## macOS

Também precisa ser validado em um Mac. Preparar Command Line Tools:

```sh
xcode-select --install
```

Com Homebrew já instalado:

```sh
brew install raylib pkg-config
clang --version
make --version
pkg-config --modversion raylib
```

O Homebrew pode instalar uma versão diferente da 5.5. Combinar uma versão comum antes de integrar novas funcionalidades e testar a compatibilidade. Não usar bibliotecas Windows no Linux/macOS.

## Escopo das dependências

O jogo usa C, a biblioteca padrão e raylib. Python, ReportLab, Pillow, pypdf e pypdfium2 foram usados na produção do PDF e não são necessários para compilar ou jogar. O catálogo de melhorias, os sprites e o Makefile continuam sendo trabalho das etapas do plano.

## Fontes oficiais

- [MSYS2: gerenciamento de pacotes](https://www.msys2.org/docs/package-management/)
- [raylib: GNU/Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)
- [raylib: macOS](https://github.com/raysan5/raylib/wiki/Working-on-macOS)
- [raylib 5.5: Makefile da biblioteca](https://github.com/raysan5/raylib/blob/5.5/src/Makefile)

Os comandos de instalação são para preparar máquinas que ainda não tenham as dependências. Este trabalho apenas verificou o ambiente Windows existente e compilou os testes descritos acima.
