#include "ia.h"

#include <limits.h>
#include <stdlib.h>

static int valorPeca(char tipo) {
    switch(tipo) {
        case 'P': return 1;
        case 'C':
        case 'B': return 3;
        case 'T': return 5;
        case 'R': return 4;
        case 'D': return 9;
        default: return 0;
    }
}

static int material(const Combate *j,int lado) {
    int total=0;
    for(int u=0;u<40;u++)
        if(j->unidades[u].vivo && j->unidades[u].lado==lado)
            total+=valorPeca(j->unidades[u].tipo);
    return total;
}

static int distanciaAliado(const Combate *j,int u) {
    Casa origem=posicao(j,u);
    int menor=INT_MAX;
    for(int v=0;v<40;v++) {
        if(!j->unidades[v].vivo || j->unidades[v].lado!=0) continue;
        Casa alvo=posicao(j,v);
        int distancia=abs(origem.l-alvo.l)+abs(origem.c-alvo.c);
        if(distancia<menor) menor=distancia;
    }
    return menor==INT_MAX?0:menor;
}

static int avaliarAcao(const Combate *j,Acao a) {
    Combate depois=*j;
    if(!executarAcaoGerada(&depois,a)) return INT_MIN;

    int balanco=(material(j,0)-material(&depois,0))
        -(material(j,1)-material(&depois,1));
    int pontuacao=balanco*100;
    if(!depois.unidades[a.ator].vivo) return pontuacao;

    int distanciaAntes=distanciaAliado(j,a.ator);
    int distanciaDepois=distanciaAliado(&depois,a.ator);
    pontuacao+=(distanciaAntes-distanciaDepois)*10-distanciaDepois;

    int ameacadoAntes=ameacado(j,a.ator);
    int ameacadoDepois=ameacado(&depois,a.ator);
    int valor=valorPeca(depois.unidades[a.ator].tipo);
    if(ameacadoDepois && !ameacadoAntes) pontuacao-=valor*100;
    else if(ameacadoAntes && !ameacadoDepois) pontuacao+=valor*50;
    return pontuacao;
}

int acaoInimiga(Combate *j) {
    if(j->lado!=1 || promocaoPendente(j)>=0) return 0;
    ListaAcoes lista;
    Acao melhor={0};
    int valor=INT_MIN;
    for(int u=0;u<40;u++) {
        gerarAcoes(j,u,&lista);
        for(int i=0;i<lista.n;i++) {
            Acao a=lista.itens[i];
            int nota=avaliarAcao(j,a);
            if(nota>valor) {valor=nota;melhor=a;}
        }
    }
    if(valor!=INT_MIN) return executarAcao(j,melhor);
    encerrarAcao(j,-1);
    return 0;
}
