#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(a, b)                     \
    ({                                \
        __typeof__(a) _a = (a);       \
        __typeof__(b) _b = (b);       \
        _a > _b ? _a : _b;            \
    })

typedef struct stNo {
    int info;
    struct stNo *esquerda;
    struct stNo *direita;
    int fb;
} tNo;

tNo* rotacaoDireita(tNo *p) {
    tNo *q = p->esquerda;
    p->esquerda = q->direita;
    q->direita = p;
    return q;
}

tNo* rotacaoEsquerda(tNo *p) {
    tNo *q = p->direita;
    p->direita = q->esquerda;
    q->esquerda = p;
    return q;
}

tNo* balancear(tNo *p) {
    if (p->fb <= -2) {
        if (p->esquerda->fb > 0)
            p->esquerda = rotacaoEsquerda(p->esquerda);
        p = rotacaoDireita(p);
    } else {
        if (p->direita->fb < 0)
            p->direita = rotacaoDireita(p->direita);
        p = rotacaoEsquerda(p);
    }
    return p;
}

int atualizaFatBal(tNo **no) {
    int he, hd;
    if (*no == NULL)
        return 0;
    he = atualizaFatBal(&(*no)->esquerda);
    hd = atualizaFatBal(&(*no)->direita);
    (*no)->fb = hd - he;
    if (abs((*no)->fb) >= 2) {
        *no = balancear(*no);
        he = atualizaFatBal(&(*no)->esquerda);
        hd = atualizaFatBal(&(*no)->direita);
        (*no)->fb = hd - he;
    }
    return max(hd, he) + 1;
}

tNo* inserirNo(tNo *raiz, int info) {
    if (!raiz) {
        tNo *novo = (tNo *) malloc(sizeof(tNo));
        if (!novo) {
            printf("Sem memoria\n");
            exit(1);
        }
        novo->info = info;
        novo->esquerda = NULL;
        novo->direita = NULL;
        novo->fb = 0;
        return novo;
    }

    if (info == raiz->info) {
        return raiz;
    } 
    else if (info < raiz->info)
        raiz->esquerda = inserirNo(raiz->esquerda, info);
    else
        raiz->direita = inserirNo(raiz->direita, info);

    return raiz;
}
void print_arvore(tNo *no, int espaco) {
    if (!no) return;

    print_arvore(no->direita, espaco + 1);

    for (int i = 0; i < espaco; i++)
        printf("   ");
    printf("%d (fb=%d)\n", no->info, no->fb);

    print_arvore(no->esquerda, espaco + 1);
}

void escreverDot(tNo *no, FILE *arquivo) {
    if (!no) return;

    fprintf(arquivo, "    %d [label=\"%d\\nFB=%d\"];\n", no->info, no->info, no->fb);

    if (no->esquerda) {
        fprintf(arquivo, "    %d -> %d;\n", no->info, no->esquerda->info);
        escreverDot(no->esquerda, arquivo);
    }
    if (no->direita) {
        fprintf(arquivo, "    %d -> %d;\n", no->info, no->direita->info);
        escreverDot(no->direita, arquivo);
    }
}

void exportarParaDot(tNo *raiz, const char *nomeArquivo) {
    if (!raiz) {
        printf("Árvore vazia. Nenhum arquivo gerado.\n");
        return;
    }

    FILE *arquivo = fopen(nomeArquivo, "w");
    if (!arquivo) {
        perror("Erro ao criar o arquivo");
        return;
    }

    fprintf(arquivo, "digraph AVL {\n");
    fprintf(arquivo, "node [shape=ellipse, style=filled, fillcolor=lightblue];\n");
    escreverDot(raiz, arquivo);
    fprintf(arquivo, "}\n");

    fclose(arquivo);
    printf("Arquivo DOT gerado com sucesso: %s\n", nomeArquivo);
}

void desalocar_arvore(tNo *no) {
    if (no) {
        desalocar_arvore(no->esquerda);
        desalocar_arvore(no->direita);
        free(no);
    }
}

int main() {
    tNo *raiz = NULL;
    int opcao, num;

    do {
        printf("\n1 - Inserir numero\n2 - Exibir arvore\n3 - Exportar para DOT\n0 - Sair\nEscolha: ");
        scanf("%d", &opcao);

        if (opcao == 1) {
            do {
                printf("Insira um numero ou 0 para sair: ");
                scanf("%d", &num);
                if (num == 0)
                    break;
                if (!raiz)
                    raiz = inserirNo(raiz, num);
                else
                    inserirNo(raiz, num);
                atualizaFatBal(&raiz);
            } while (num != 0);
        }

        else if (opcao == 2) {
            printf("\nÁrvore:\n");
            print_arvore(raiz, 0);
        }

        else if (opcao == 3) {
            exportarParaDot(raiz, "arvore.dot");
        }

    } while (opcao != 0);

    desalocar_arvore(raiz);
    return 0;
}