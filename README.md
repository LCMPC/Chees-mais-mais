# Chees mais mais

Roguelike tático de xadrez desenvolvido em **C com raylib**, como projeto em grupo.

## Estado atual

O projeto já possui uma partida jogável em raylib, com combate por turnos, cartas de melhorias, recrutamento, controle manual dos dois lados e expansão do tabuleiro.

| Arquivo | Responsabilidade / estado |
| --- | --- |
| `src/main.c` | Janela raylib e loop do jogo |
| `src/ui/screens.c` e `.h` | Menu, escolha inicial, cartas, preparação, combate e derrota |
| `src/ui/theme.c` e `.h` | Paleta, componentes e peças desenhadas quando sprites faltam |
| `src/campo/tabuleiro.c` e `.h` | Matriz de casas com ponteiros; criação, expansão e liberação |
| `src/pecas/peca.c` e `.h` | Estrutura, criação e lista de peças aliadas |
| `src/pecas/pecaInimiga.c` e `.h` | Geração de inimigos por fase, posicionamento e distribuição de IDs de melhorias |
| `src/campo/movimento.c` e `.h` | Validação geral de movimento, captura e toque do rei |
| `src/pecas/unidades/*.c` | Regras de movimento de cada tipo de peça |
| `src/campo/combate.c` e `.h` | Estado da partida, geração e execução de ações, efeitos temporários, capturas e transição de rodadas |
| `src/campo/ia.c` e `.h` | Escolha de jogadas inimigas, priorizando capturas, segurança e aproximação |
| `src/pecas/melhorias.c` | Catálogo de 60 cartas, sorteio, aquisição e limites de uso |
| `src/pecas/unidades/melhorias_*.c` | Regras específicas das melhorias, divididas em peão/cavalo, torre/bispo e rei/rainha |

## Proposta do jogo

O jogador começa com peão, torre, bispo, cavalo ou rei. A cada rodada, escolhe uma melhoria entre três cartas sorteadas para tipos de peça que possui. Melhorias aliadas afetam todas as unidades daquele tipo e não se repetem.

Nas rodadas pares, uma opção permite recrutar outra peça, respeitando a capacidade de colocação. Após escolher a recompensa, o jogador posiciona as novas unidades em casas livres das duas primeiras fileiras do seu lado. A origem escolhida permanece registrada durante a run. O combate usa movimento normal; todas as peças aliadas, inclusive as capturadas, retornam às suas origens ao iniciar a próxima rodada após uma vitória.

O inimigo começa com três peças. Da segunda rodada em diante, um sorteio aumenta a quantidade de unidades ou o orçamento de melhorias; a composição é sorteada novamente. A cada cinco rodadas, uma unidade vira um rei sem melhorias, que só pode ser capturado por último. Nessas rodadas, o jogador recebe cinco opções de cartas. Rainhas inimigas não recebem melhorias.

Rainhas aliadas permanentes são obtidas pelas cartas P20 (Coroação) e P21 (Ascensão real). Peões também podem escolher rainha como promoção temporária durante o combate. As cartas de rainha entram no sorteio quando houver uma rainha aliada viva na escolha de recompensas. Reis comuns e rainhas inimigas ficam disponíveis no sorteio de inimigos a partir da fase 10. A Ascensão real é exclusiva do jogador. Capturar um rei comum não encerra a rodada; apenas o rei identificado como chefe tem a regra de ser capturado por último.

### Tabuleiro expansível

Dimensões na ordem **largura × altura**:

| Rodadas | Tamanho |
| --- | --- |
| 1 a 5 | 5 × 5 |
| 6 a 10 | 6 × 5 |
| 11 a 15 | 7 × 5 |
| 16 em diante | 8 × 5 |

A expansão ocorre após vencer o chefe, para a próxima rodada. A altura permanece em cinco linhas, e a largura cresce automaticamente de cinco até oito colunas.

### Interface e arte

Paleta preto e branco com detalhes vermelhos. Tabuleiro no centro, aliados à esquerda, inimigos à direita, aliados capturados acima e inimigos capturados abaixo. O catálogo selecionado está no plano v0.5.

## Requisitos técnicos

- C e raylib.
- Matriz para representar o tabuleiro.
- Listas encadeadas para peças e registros de capturas.
- Arquivo para o recorde, com `podio.txt` como nome proposto.
- Makefile para Windows, Linux e macOS.

## Dependências

Compilador C, raylib, GNU Make e pkg-config (Linux/macOS). O ambiente Windows verificado anteriormente usa MSYS2/MinGW64, GCC e raylib 5.5. O protótipo visual compilou neste macOS com raylib 6.2.0.

Consulte o [guia de dependências](docs/DEPENDENCIAS.md) para preparar as máquinas do grupo. Python e bibliotecas usadas para gerar o PDF não são dependências do jogo.

## Executar o jogo

Na raiz do projeto, em Linux/macOS:

```sh
make
make run
```

No Windows, abra o terminal **MSYS2 UCRT64**. Se ainda não tiver as dependências, instale-as uma vez:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-make
```

Nesse terminal, entre na pasta do projeto e compile/inicie o jogo:

```sh
cd "/c/caminho/para/Chees-mais-mais"
mingw32-make
mingw32-make run
```

`make clean` (ou `mingw32-make clean` no Windows) limpa a pasta `build`. Execute a partir da raiz para que os recursos opcionais sejam encontrados.

O fluxo atual inclui escolha da peça, posicionamento inicial, escolha da recompensa, combate, próxima rodada e derrota. Use mouse, Enter e setas na seleção inicial, e clique ou pressione o número da carta para adquirir uma melhoria. Durante o combate, arraste uma peça ou clique nela e em seu destino. Os inimigos agem automaticamente: priorizam capturas, evitam ameaças imediatas e se aproximam das peças aliadas quando não podem capturar. Quando houver alternativas, escolha a ação e seus alvos adicionais no painel. Clique novamente na própria peça para ativar melhorias estacionárias. Escape cancela a seleção ou retorna ao menu. O pódio permanece visualmente reservado para a persistência futura.

### Regras da implementação atual

- Cada equipe executa uma ação por vez. O jogador controla os aliados e a IA controla os inimigos. O toque do rei concede uma ação adicional consecutiva ao alvo quando ele agir.
- Ao alcançar a última fileira, um peão se promove à escolha do jogador em rainha, torre, bispo ou cavalo até o fim da rodada. Na rodada seguinte, volta a ser peão. As cartas P20 (Coroação) e P21 (Ascensão real) continuam concedendo promoções permanentes.
- As cartas aliadas pertencem ao tipo de peça. Recrutas herdam as aquisições. Limites por rodada são compartilhados entre aliados do mesmo tipo.
- Efeitos opcionais de movimento são selecionáveis. Cada melhoria aparece uma vez no menu de ações; quando ela possui múltiplas variantes, os destinos ou alvos são escolhidos em seguida. As reações opcionais podem ser ligadas ou desligadas no painel aliado e escolhem o primeiro alvo elegível quando existem vários.
- Uma rodada termina com a eliminação de uma equipe. Quando ambas ficam sem ações, a partida termina em derrota.
- O estado de combate usa uma matriz de índices e listas encadeadas por índices para as unidades ativas e registros de captura da rodada. Os totais acumulados de capturas permanecem durante toda a run. A lista encadeada original de inimigos continua sendo usada para gerar as rodadas.

Os 12 sprites principais das peças estão em `sprites/`, com versões brancas e pretas para cada tipo. São carregados ao iniciar o jogo. Se um sprite estiver ausente ou não puder ser carregado, o jogo usa o recurso alternativo em `assets/pieces/white_pawn.png`, `black_pawn.png` etc., quando disponível; caso contrário, mantém a peça desenhada por código. Os tipos aceitos nesse caminho alternativo são `pawn`, `rook`, `bishop`, `knight`, `king` e `queen`, com prefixo `white_` ou `black_`. A fonte opcional em `assets/fonts/ui.ttf` também possui a fonte padrão da raylib como reserva.

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
    combate.c / combate.h
    ia.c / ia.h
  pecas/
    melhorias.c
    peca.c / peca.h
    pecaInimiga.c / pecaInimiga.h
    unidades/
      peao.c, torre.c, bispo.c, cavalo.c, rei.c, rainha.c
      melhorias_pc.c, melhorias_tb.c, melhorias_rd.c
tests/
  testar_melhorias.c
```

`peca.h` e `pecaInimiga.h` definem as unidades e não dependem do tabuleiro. `tabuleiro.h` usa esses tipos para guardar os ponteiros nas casas. `movimento.h` apresenta as funções de validação e usa o tabuleiro. `ia.h` expõe a decisão da IA, enquanto `ia.c` escolhe e avalia ações usando as regras do combate. As regras individuais continuam nos arquivos de cada peça, com um único cabeçalho de movimentos.

Incluir cabeçalhos `.h`, nunca arquivos `.c`. Os caminhos dos includes são relativos ao arquivo que está incluindo: em `pecas/unidades/`, `../../campo/movimento.h` sobe duas pastas até `src/` e entra em `campo/`.

## Verificações do motor

Na raiz, compilar e executar os testes sem abrir a interface:

```sh
make test
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

O [plano atualizado em PDF](PlanoChess++.pdf) (v0.5) inclui as decisões de interface e o catálogo com 60 melhorias escolhidas, distribuídas entre os seis tipos de peça (páginas 15 a 20).

A geração inimiga utiliza os contadores de dificuldade e as cartas elegíveis do catálogo, respeitando a proteção do chefe.
