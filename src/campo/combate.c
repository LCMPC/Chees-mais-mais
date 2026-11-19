#include "combate.h"
#include "movimento.h"
#include <stdlib.h>
#include <string.h>

static const Casa nenhuma = {-1, -1};
static int reagindo;

int igualCasa(Casa a, Casa b) { return a.l == b.l && a.c == b.c; }
int dentro(const Combate *j, Casa p) {
    return p.l >= 0 && p.l < 5 && p.c >= 0 && p.c < j->largura;
}
int ocupante(const Combate *j, Casa p) {
    return dentro(j, p) ? j->tab[p.l][p.c] - 1 : -1;
}
int vazio(const Combate *j, Casa p) { return dentro(j, p) && ocupante(j,p) < 0; }
Casa posicao(const Combate *j, int u) {
    if (u < 0 || u >= 40 || !j->unidades[u].vivo) return nenhuma;
    for (int l=0; l<5; l++) for (int c=0; c<j->largura; c++)
        if (j->tab[l][c] == u+1) return (Casa){l,c};
    return nenhuma;
}
int aliado(const Combate *j, int u, int v) {
    return u >= 0 && u < 40 && v >= 0 && v < 40 && j->unidades[u].vivo
        && j->unidades[v].vivo && j->unidades[u].lado == j->unidades[v].lado;
}
int inimigo(const Combate *j, int u, int v) {
    return u >= 0 && u < 40 && v >= 0 && v < 40 && j->unidades[u].vivo
        && j->unidades[v].vivo && j->unidades[u].lado != j->unidades[v].lado;
}
int ortogonal(Casa a, Casa b) { return movimentoTorre(a.l,a.c,b.l,b.c); }
int diagonal(Casa a, Casa b) { return movimentoBispo(a.l,a.c,b.l,b.c); }
int adjacente(Casa a, Casa b) { return movimentoRei(a.l,a.c,b.l,b.c); }
int caminho(const Combate *j, Casa a, Casa b) {
    if (!dentro(j,a) || !dentro(j,b) || (!ortogonal(a,b) && !diagonal(a,b))) return 0;
    int dl=(b.l>a.l)-(b.l<a.l), dc=(b.c>a.c)-(b.c<a.c);
    for (Casa p={a.l+dl,a.c+dc}; !igualCasa(p,b); p.l+=dl,p.c+=dc)
        if (!vazio(j,p)) return 0;
    return 1;
}
int contarLado(const Combate *j, int lado) {
    int n=0;
    for(int no=j->primeira[lado];no;no=j->unidades[no-1].proximo) n++;
    return n;
}
int podeCapturar(const Combate *j, int u, int v) {
    return inimigo(j,u,v) && (!j->unidades[v].chefe || contarLado(j,j->unidades[v].lado)==1);
}
int movimentoBase(const Combate *j, int u, Casa b) {
    Casa a=posicao(j,u);
    if (!dentro(j,a) || !dentro(j,b) || igualCasa(a,b)) return 0;
    const Unidade *p=&j->unidades[u];
    int v=ocupante(j,b), ocupado=v>=0;
    if (j->selada[b.l][b.c]==p->lado+1) return 0;
    if (aliado(j,u,v))
        return p->tipo=='R' && j->unidades[v].tipo!='R'
            && !j->unidades[v].duplo && adjacente(a,b) ? MOVIMENTO_BUFF_REI : 0;
    if (ocupado && !podeCapturar(j,u,v)) return 0;
    int legal=0;
    char tipo=p->herdeira && !p->silencio ? 'D' : p->tipo;
    switch(tipo) {
        case 'P': legal=movimentoPeao(a.l,a.c,b.l,b.c,p->lado?1:-1,p->moveu,ocupado); break;
        case 'T': legal=ortogonal(a,b); break;
        case 'B': legal=diagonal(a,b); break;
        case 'R': legal=adjacente(a,b); break;
        case 'C': legal=movimentoCavalo(a.l,a.c,b.l,b.c); break;
        case 'D': legal=ortogonal(a,b)||diagonal(a,b); break;
    }
    if (legal && (tipo=='C' || caminho(j,a,b))) return ocupado?MOVIMENTO_CAPTURA:MOVIMENTO_NORMAL;
    if (p->tipo=='P' && ocupado && j->unidades[v].marca && adjacente(a,b)) return MOVIMENTO_CAPTURA;
    if (!ocupado && dentro(j,p->corredor) && caminho(j,a,b)
        && ((ortogonal(a,p->corredor)&&ortogonal(b,p->corredor))
            || (diagonal(a,p->corredor)&&diagonal(b,p->corredor)))
        && abs(a.l-b.l)+abs(a.c-b.c)+abs(b.l-p->corredor.l)+abs(b.c-p->corredor.c)
            == abs(a.l-p->corredor.l)+abs(a.c-p->corredor.c)) return MOVIMENTO_NORMAL;
    return 0;
}
int ameaca(const Combate *j, int u, Casa destino) {
    if (!dentro(j,destino) || !dentro(j,posicao(j,u))) return 0;
    int v=ocupante(j,destino);
    if (inimigo(j,u,v)) {
        if(movimentoBase(j,u,destino)==MOVIMENTO_CAPTURA) return 1;
        if(!podeCapturar(j,u,v) || j->unidades[u].silencio) return 0;
        ListaAcoes alternativas={0};
        char tipo=j->unidades[u].tipo;
        /* Só estas cartas ampliam o alcance de captura. Gerar ações de
         * resgate do cavalo nesta consulta causaria recursão. */
        if(tipo=='P') gerarPC(j,u,&alternativas);
        else if(tipo=='T'||tipo=='B') gerarTB(j,u,&alternativas);
        else if(tipo=='R') gerarRD(j,u,&alternativas);
        for(int i=0;i<alternativas.n;i++) {
            Acao a=alternativas.itens[i];
            if(!igualCasa(a.destino,destino)) continue;
            if((tipo=='P'&&(a.carta==1||a.carta==16))
                ||(tipo=='T'&&a.carta==20)||(tipo=='B'&&a.carta==2)
                ||(tipo=='R'&&(a.carta==5||a.carta==20))) return 1;
        }
        return 0;
    }
    if (v>=0) return 0;
    Combate copia=*j;
    int livre=-1;
    for(int i=0;i<40;i++) if(!copia.unidades[i].vivo) { livre=i;break; }
    if(livre<0) return 0;
    copia.unidades[livre]=(Unidade){.vivo=1,.lado=1-j->unidades[u].lado,.tipo='P',.corredor=nenhuma};
    int lado=copia.unidades[livre].lado;
    copia.unidades[livre].proximo=copia.primeira[lado];
    copia.primeira[lado]=livre+1;
    copia.tab[destino.l][destino.c]=livre+1;
    return ameaca(&copia,u,destino);
}
int ameacado(const Combate *j, int u) {
    Casa p=posicao(j,u);
    for (int v=0;v<40;v++) if(inimigo(j,u,v) && ameaca(j,v,p)) return 1;
    return 0;
}
static int mesmaAcao(Acao a,Acao b) {
    return a.ator==b.ator&&a.carta==b.carta&&igualCasa(a.destino,b.destino)
        &&igualCasa(a.extra,b.extra)&&igualCasa(a.extra2,b.extra2);
}

void adicionar(ListaAcoes *lista,int u,int carta,Casa destino,Casa extra,Casa extra2) {
    Acao nova={u,carta,destino,extra,extra2};
    for(int i=0;i<lista->n;i++)
        if(mesmaAcao(nova,lista->itens[i])) return;
    if(lista->n<MAX_ACOES) lista->itens[lista->n++]=nova;
}
static void registrarMovimento(Combate *j,int u,Casa origem,Casa destino) {
    TrajetoMovimento *t=&j->trajetos[u];
    if(!t->n) t->casas[t->n++]=origem;
    if(t->n<=MAX_ETAPAS_MOVIMENTO) t->casas[t->n++]=destino;
    else t->casas[MAX_ETAPAS_MOVIMENTO]=destino;
}
int deslocar(Combate *j,int u,Casa p) {
    Casa a=posicao(j,u);
    if(!dentro(j,a)||!vazio(j,p)||j->selada[p.l][p.c]==j->unidades[u].lado+1) return 0;
    j->tab[a.l][a.c]=0;
    j->tab[p.l][p.c]=u+1;
    registrarMovimento(j,u,a,p);
    j->unidades[u].moveu=1;
    return 1;
}
int trocar(Combate *j,int u,int v) {
    if(!aliado(j,u,v)||u==v) return 0;
    Casa a=posicao(j,u),b=posicao(j,v);
    if(!dentro(j,a)||!dentro(j,b)||j->selada[a.l][a.c]==j->unidades[v].lado+1
        ||j->selada[b.l][b.c]==j->unidades[u].lado+1) return 0;
    j->tab[a.l][a.c]=v+1; j->tab[b.l][b.c]=u+1;
    registrarMovimento(j,u,a,b);
    registrarMovimento(j,v,b,a);
    j->unidades[u].moveu=j->unidades[v].moveu=1;
    return 1;
}
void remover(Combate *j,int u) {
    Casa p=posicao(j,u);
    if(!dentro(j,p)) return;
    Unidade *vitima=&j->unidades[u];
    int t=indiceTipo(vitima->tipo);
    if(t>=0) {
        j->capturas[vitima->lado][t]++;
        if(j->quantidadeCapturas<40) {
            int i=j->quantidadeCapturas++;
            j->historicoRodada[i]=(RegistroCaptura){
                .tipo=vitima->tipo, .lado=vitima->lado, .rodada=j->rodada,
                .proximo=j->primeiraCaptura, .casa=p,
                .cartas=vitima->lado?vitima->cartas:j->adquiridas[t]
            };
            j->primeiraCaptura=i+1;
        }
    }
    int *elo=&j->primeira[vitima->lado];
    while(*elo && *elo!=u+1) elo=&j->unidades[*elo-1].proximo;
    if(*elo) *elo=vitima->proximo;
    j->tab[p.l][p.c]=0;
    vitima->vivo=0;
    vitima->proximo=0;
    vitima->promocaoPendente=0;
}
void tocar(Combate *j,int u) {
    if(u>=0 && u<40 && j->unidades[u].vivo && j->unidades[u].tipo!='R')
        j->unidades[u].duplo=1;
}
void promover(Combate *j,int u,char tipo,int permanente) {
    if(u<0||u>=40||!j->unidades[u].vivo) return;
    j->unidades[u].tipo=tipo;
    j->unidades[u].promocaoPendente=0;
    if(permanente) j->unidades[u].original=tipo;
    if(j->unidades[u].lado) j->unidades[u].cartas=0;
}
int promocaoPendente(const Combate *j) {
    for(int u=0;u<40;u++) {
        const Unidade *p=&j->unidades[u];
        if(p->vivo && p->tipo=='P' && p->promocaoPendente) return u;
    }
    return -1;
}
int escolherPromocao(Combate *j,int u,char tipo) {
    if(u<0 || u>=40 || !j->unidades[u].vivo ||
        j->unidades[u].tipo!='P' || !j->unidades[u].promocaoPendente) return 0;
    if(tipo!='D' && tipo!='T' && tipo!='B' && tipo!='C') return 0;
    promover(j,u,tipo,0);
    return 1;
}
int capturar(Combate *j,int u,int v) {
    if(!podeCapturar(j,u,v)) return 0;
    Casa p=posicao(j,v);
    if(!dentro(j,p)||j->selada[p.l][p.c]==j->unidades[u].lado+1) return 0;
    int anterior=reagindo;
    /* Defesas continuam protegendo uma unidade de capturas reativas.
     * Somente as contra-capturas deixam de disparar novas cadeias. */
    reagindo=1;
    int bloqueado=antesPC(j,u,v)||antesTB(j,u,v)||antesRD(j,u,v);
    reagindo=anterior;
    if(bloqueado) return 0;
    remover(j,v);
    deslocar(j,u,p);
    j->unidades[u].capturou=1;
    efeitosMorteRD(j,u,v);
    if(!anterior) {
        reagindo=1;
        aposCapturaPC(j,u,v,p);
        aposCapturaTB(j,u,v,p);
        aposCapturaRD(j,u,v,p);
        for(int g=0;g<40;g++) {
            if(!j->unidades[g].guarda || !inimigo(j,u,g) || j->unidades[g].lado!=j->unidades[v].lado) continue;
            if(ameaca(j,g,posicao(j,u))) {
                j->unidades[g].guarda=0;
                capturar(j,g,u);
                break;
            }
        }
        reagindo=anterior;
    }
    return 1;
}
int moverOuCapturar(Combate *j,int u,Casa p) {
    int v=ocupante(j,p);
    return v>=0?capturar(j,u,v):deslocar(j,u,p);
}
void gerarAcoes(const Combate *j,int u,ListaAcoes *lista) {
    lista->n=0;
    if(promocaoPendente(j)>=0) return;
    if(u<0||u>=40||!j->unidades[u].vivo || j->unidades[u].lado!=j->lado) return;
    if(j->bonus>=0 && j->bonus!=u) return;
    for(int l=0;l<5;l++) for(int c=0;c<j->largura;c++) {
        Casa p={l,c};
        if(movimentoBase(j,u,p)) adicionar(lista,u,0,p,nenhuma,nenhuma);
    }
    if(!j->unidades[u].silencio) {
        gerarPC(j,u,lista); gerarTB(j,u,lista); gerarRD(j,u,lista);
    }
}
void encerrarAcao(Combate *j,int ator) {
    if(ator<0 && promocaoPendente(j)>=0) return;
    int lado=j->lado;
    for(int u=0;u<40;u++) {
        Unidade *p=&j->unidades[u];
        if(p->lado==lado) {
            p->silencio=0;
            if(ator<0) p->guarda=0;
        }
        else { p->marca=0; p->gambito=0; }
    }
    for(int l=0;l<5;l++) for(int c=0;c<j->largura;c++)
        if(j->selada[l][c]==lado+1) j->selada[l][c]=0;
    if(ator>=0 && j->unidades[ator].vivo && j->unidades[ator].duplo && j->bonus!=ator) {
        j->unidades[ator].duplo=0;
        j->bonus=ator;
        return;
    }
    j->bonus=-1;
    j->lado=1-lado;
}
int executarAcaoGerada(Combate *j,Acao a) {
    Combate anterior=*j;
    memset(j->trajetos,0,sizeof j->trajetos);
    int u=a.ator, v=ocupante(j,a.destino), capturou=0, aplicada=0;
    Casa origem=posicao(j,u);
    char tipo=j->unidades[u].tipo;
    int antes[2]={contarLado(j,0),contarLado(j,1)};
    /* Guarda dura até o início da próxima ação de sua equipe.
     * A concessão feita nesta ação permanece para a resposta inimiga. */
    for(int i=0;i<40;i++) if(j->unidades[i].lado==j->lado) j->unidades[i].guarda=0;
    if(a.carta==0) {
        if(movimentoBase(j,u,a.destino)==MOVIMENTO_BUFF_REI) {
            tocar(j,v);
            aplicada=1;
        } else aplicada=moverOuCapturar(j,u,a.destino);
    } else if(tipo=='P'||tipo=='C') aplicada=executarPC(j,a);
    else if(tipo=='T'||tipo=='B') aplicada=executarTB(j,a);
    else aplicada=executarRD(j,a);
    if(!aplicada) {
        /* Interceptações gastam Bastião ou Martírio. Uma falha sem
         * interceptação deve restaurar inclusive deslocamentos parciais. */
        int interceptada=j->quantidadeCapturas>anterior.quantidadeCapturas;
        for(int t=0;t<TIPOS_PECA;t++)
            if(j->usadas[t]!=anterior.usadas[t]) interceptada=1;
        for(int i=0;i<40;i++)
            if(j->unidades[i].usadas!=anterior.unidades[i].usadas) interceptada=1;
        if(!interceptada) {
            *j=anterior;
            return 0;
        }
    }
    capturou=contarLado(j,1-j->lado)<antes[1-j->lado];
    if(j->unidades[u].vivo) {
        j->unidades[u].herdeira=0;
        j->unidades[u].corredor=nenhuma;
        depoisPC(j,u,origem,capturou);
        depoisTB(j,u,origem,capturou);
        reagindo=1;
        depoisRD(j,u,origem,capturou);
        reagindo=0;
    }
    /* Uma promoção também pode ocorrer quando outra peça transporta o peão. */
    for(int i=0;i<40;i++) {
        Casa origemExtra=posicao(&anterior,i);
        if(i==u || !j->unidades[i].vivo || !dentro(j,origemExtra)
            || igualCasa(posicao(j,i),origemExtra)) continue;
        depoisPC(j,i,origemExtra,0);
        if(j->unidades[i].lado==j->lado) {
            reagindo=1;
            depoisRD(j,i,origemExtra,0);
            reagindo=0;
        }
    }
    encerrarAcao(j,u);
    return 1;
}
int executarAcao(Combate *j,Acao a) {
    ListaAcoes lista;
    gerarAcoes(j,a.ator,&lista);
    int ok=0;
    for(int i=0;i<lista.n;i++) if(mesmaAcao(a,lista.itens[i])) {ok=1;break;}
    return ok?executarAcaoGerada(j,a):0;
}
/* Uma unidade aliada capturada conserva seu indice, tipo original e origem
 * ate o fim da run. Os slots de inimigos mortos continuam reutilizaveis. */
static int aliadoRecrutado(const Unidade *p) {
    return !p->lado && indiceTipo(p->original)>=0;
}
static int inserir(Combate *j,char tipo,int lado,Casa p) {
    if(!vazio(j,p)) return -1;
    for(int u=0;u<40;u++) if(!j->unidades[u].vivo && !aliadoRecrutado(&j->unidades[u])) {
        j->unidades[u]=(Unidade){.tipo=tipo,.original=tipo,.lado=lado,.origem=p,.vivo=1,.corredor=nenhuma};
        j->unidades[u].proximo=j->primeira[lado];
        j->primeira[lado]=u+1;
        j->tab[p.l][p.c]=u+1;
        if(u>=j->quantidade) j->quantidade=u+1;
        return u;
    }
    return -1;
}
int recrutar(Combate *j,char tipo,Casa p) {
    if(indiceTipo(tipo)<0||tipo=='D'||p.l<3||!vazio(j,p)) return 0;
    for(int u=0;u<40;u++) if(aliadoRecrutado(&j->unidades[u])&&igualCasa(j->unidades[u].origem,p)) return 0;
    return inserir(j,tipo,0,p)>=0;
}
static int gerarInimigos(Combate *j) {
    PecaInimiga *lista=NULL;
    if(criarRodadaInimiga(&lista,j->rodada,j->melhoriasInimigas,j->pecasInimigas)<0) return 0;
    for(PecaInimiga *p=lista;p;p=p->proximo) {
        int u=inserir(j,p->inicial,1,(Casa){p->posicaoInicial[0],p->posicaoInicial[1]});
        if(u<0) {liberarPecasInimigas(&lista);return 0;}
        j->unidades[u].chefe=p->ehChefe;
        int t=indiceTipo(p->inicial);
        for(int i=0;i<p->quantidadeMelhorias;i++) {
            int id=catalogo[t][p->melhorias[i]-1].id;
            j->unidades[u].cartas|=1u<<id;
        }
    }
    liberarPecasInimigas(&lista);
    return 1;
}
void iniciarCombate(Combate *j,char tipo) {
    memset(j,0,sizeof *j);
    j->largura=5; j->rodada=1; j->bonus=-1; j->pecasInimigas=3;
    inserir(j,tipo,0,(Casa){4,2});
    gerarInimigos(j);
}
int prepararRodada(Combate *j) {
    if(promocaoPendente(j)>=0) return 0;
    Combate anterior=*j;
    memset(j->trajetos,0,sizeof j->trajetos);
    j->rodada++;
    j->largura=5+(j->rodada-1)/5;
    if(j->largura>8) j->largura=8;
    if(rand()%2 && j->pecasInimigas<3*j->largura) j->pecasInimigas++;
    else j->melhoriasInimigas++;
    memset(j->tab,0,sizeof j->tab);
    memset(j->selada,0,sizeof j->selada);
    memset(j->usadas,0,sizeof j->usadas);
    memset(j->historicoRodada,0,sizeof j->historicoRodada);
    j->primeiraCaptura=j->quantidadeCapturas=0;
    j->primeira[0]=j->primeira[1]=0;
    for(int u=0;u<40;u++) {
        Unidade *p=&j->unidades[u];
        p->promocaoPendente=0;
        p->proximo=0;
        if(!aliadoRecrutado(p)) {p->vivo=0; continue;}
        p->vivo=1;
        p->tipo=p->original;
        p->capturou=p->duplo=p->guarda=p->silencio=p->marca=p->herdeira=p->gambito=0;
        p->corredor=nenhuma;
        j->tab[p->origem.l][p->origem.c]=u+1;
        p->proximo=j->primeira[0];
        j->primeira[0]=u+1;
    }
    j->lado=0; j->bonus=-1;
    if(gerarInimigos(j)) return 1;
    *j=anterior;
    return 0;
}
