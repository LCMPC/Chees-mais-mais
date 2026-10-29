# Chees mais mais

Roguelike tático de xadrez desenvolvido em **C com raylib**, como projeto em grupo.

## Estado atual

O projeto está no início da implementação. Há um protótipo visual navegável em raylib, ainda sem combate funcional.

| Arquivo | Responsabilidade / estado |
| --- | --- |
| `src/main.c` | Janela raylib e loop das quatro telas |
| `src/ui/screens.c` e `.h` | Menu, escolha inicial, prévia de jogo e derrota; navegação por mouse e teclado |
| `src/ui/theme.c` e `.h` | Paleta, componentes e peças desenhadas quando sprites faltam |
| `src/campo/tabuleiro.c` e `.h` | Matriz de casas com ponteiros; criação, expansão e liberação |
| `src/pecas/peca.c` e `.h` | Estrutura, criação e lista de peças aliadas |
| `src/pecas/pecaInimiga.c` e `.h` | Geração de inimigos por fase, posicionamento e distribuição de IDs de melhorias |
| `src/campo/movimento.c` e `.h` | Validação geral de movimento, captura e toque do rei |
| `src/pecas/unidades/*.c` | Regras de movimento de cada tipo de peça |
| `src/pecas/melhorias.c` | Reservado para os aprimoramentos |

## Proposta do jogo

O jogador começa com peão, torre, bispo, cavalo ou rei. A cada rodada, escolhe uma melhoria entre três cartas sorteadas para tipos de peça que possui. Melhorias aliadas afetam todas as unidades daquele tipo e não se repetem.

Nas rodadas pares, uma opção permite recrutar outra peça, respeitando a capacidade de colocação. Após escolher a recompensa, o jogador posiciona as novas unidades em casas livres das duas primeiras fileiras do seu lado. A origem escolhida permanece registrada durante a run. O plano propõe movimento normal no combate e retorno dos sobreviventes às origens entre rodadas; essa interpretação ainda precisa ser confirmada pelo grupo.

O inimigo começa com três peças. Da segunda rodada em diante, um sorteio aumenta a quantidade de unidades ou o orçamento de melhorias; a composição é sorteada novamente. A cada cinco rodadas, uma unidade vira um rei sem melhorias, que só pode ser capturado por último. Nessas rodadas, o jogador recebe cinco opções de cartas. Rainhas inimigas não recebem melhorias.

Rainhas são obtidas por aprimoramento, cuja regra ainda será definida, e não entram no sorteio comum. Na geração atual, reis comuns ficam disponíveis a partir da fase 10 e podem receber melhorias. Capturar um rei comum não encerra a rodada; apenas o rei identificado como chefe tem a regra de ser capturado por último.

### Tabuleiro expansível

Dimensões na ordem **largura × altura**:

| Rodadas | Tamanho |
| --- | --- |
| 1 a 5 | 5 × 5 |
| 6 a 10 | 6 × 5 |
| 11 a 15 | 7 × 5 |
| 16 em diante | 8 × 5 |

A expansão ocorre após vencer o chefe, para a próxima rodada. A altura permanece em cinco linhas. O tabuleiro já permite crescer de cinco até oito colunas; a transição automática entre rodadas ainda será integrada.

### Interface e arte

Paleta preto e branco com detalhes vermelhos. Tabuleiro no centro, aliados à esquerda, inimigos à direita, aliados capturados acima e inimigos capturados abaixo. Sprites e catálogo definitivo de melhorias serão produzidos pelo grupo.

## Requisitos técnicos

- C e raylib.
- Matriz para representar o tabuleiro.
- Listas encadeadas para peças e registros de capturas.
- Arquivo para o recorde, com `podio.txt` como nome proposto.
- Makefile para Windows, Linux e macOS.

## Dependências

Compilador C, raylib, GNU Make e pkg-config (Linux/macOS). O ambiente Windows verificado anteriormente usa MSYS2/MinGW64, GCC e raylib 5.5. O protótipo visual compilou neste macOS com raylib 6.2.0.

Consulte o [guia de dependências](docs/DEPENDENCIAS.md) para preparar as máquinas do grupo. Python e bibliotecas usadas para gerar o PDF não são dependências do jogo.

## Executar o protótipo visual

Na raiz do projeto:

```sh
make
make run
```

No Windows com MSYS2/MinGW64, use `mingw32-make` no lugar de `make`. `make clean` limpa a pasta `build`. Execute a partir da raiz para que os recursos opcionais sejam encontrados.

O fluxo atual é **Início → Seleção de peça → Jogo → Derrota → Início**. Use mouse, Enter e as setas na seleção; na prévia de jogo, clique em **Simular derrota** ou pressione D. Escape retorna ao início a partir da seleção ou do jogo. Rodada, pódio, peças laterais e capturas são dados visuais de demonstração. Combate e persistência ainda não foram conectados.

Para incluir arte depois, coloque uma fonte em `assets/fonts/ui.ttf` e sprites PNG em `assets/pieces/white_pawn.png`, `black_pawn.png` etc. Os tipos aceitos são `pawn`, `rook`, `bishop`, `knight`, `king` e `queen`, com prefixo `white_` ou `black_`. Se um arquivo faltar, a peça é desenhada por código; a fonte padrão da raylib também serve como reserva.

A interface foi compilada e aberta neste macOS. Linux e Windows ainda precisam ser conferidos nas máquinas do grupo.

## Organização dos módulos

```text
src/
  main.c
  ui/
    screens.c / screens.h
    theme.c / theme.h
  campo/
    tabuleiro.c / tabuleiro.h
    movimento.c / movimento.h
  pecas/
    melhorias.c
    peca.c / peca.h
    pecaInimiga.c / pecaInimiga.h
    unidades/
      peao.c, torre.c, bispo.c, cavalo.c, rei.c, rainha.c
tests/
  testar_movimento.c
  testar_inimigos.c
```

`peca.h` e `pecaInimiga.h` definem as unidades e não dependem do tabuleiro. `tabuleiro.h` usa esses tipos para guardar os ponteiros nas casas. `movimento.h` apresenta as funções de validação e usa o tabuleiro. As regras individuais continuam nos arquivos de cada peça, com um único cabeçalho de movimentos.

Incluir cabeçalhos `.h`, nunca arquivos `.c`. Os caminhos dos includes são relativos ao arquivo que está incluindo: em `pecas/unidades/`, `../../campo/movimento.h` sobe duas pastas até `src/` e entra em `campo/`.

## Verificações do motor

Na raiz, depois de criar a pasta `build`, compilar e executar os testes sem abrir a interface:

```powershell
gcc -std=c99 -Wall -Wextra -Werror -pedantic src/campo/movimento.c src/campo/tabuleiro.c src/pecas/unidades/*.c tests/testar_movimento.c -o build/testar_movimento.exe
./build/testar_movimento.exe
gcc -std=c99 -Wall -Wextra -Werror -pedantic src/pecas/pecaInimiga.c tests/testar_inimigos.c -o build/testar_inimigos.exe
./build/testar_inimigos.exe
```

## Ordem de desenvolvimento

1. Fechar regras de turnos, vitória, derrota e transição entre rodadas.
2. Ajustar matriz, largura ativa e gerenciamento de memória.
3. Integrar listas de peças e origens; a posição atual fica na matriz de ponteiros.
4. Desenhar o tabuleiro, selecionar casas e colocar novas peças.
5. Implementar movimentos, capturas e uma rodada com inimigos fixos.
6. Adicionar turnos e IA; depois melhorias, cartas e recrutamento.
7. Integrar geração inimiga, chefes, expansão e recorde.
8. Finalizar arte, interface, balanceamento e compilação nos três sistemas.

O [plano atualizado em PDF](chees_mais_mais_plano_v04.pdf) preserva as 13 páginas anteriores e acrescenta as decisões sobre as quatro telas e os recursos de reserva.

A [documentação da geração inimiga](docs/INIMIGOS.md) explica os contadores, o sorteio, os IDs de melhorias e como chamar as funções. A integração dessa geração ao tabuleiro e ao combate ainda está pendente.
