#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int RA;
    char nome[50];
    int idade;
    float notas[4];
} Aluno;

typedef struct stNo {
    Aluno aluno;
    struct stNo *esquerda;
    struct stNo *direita;
} tNo;

tNo* inserirAluno(tNo *raiz, tNo *no, Aluno aluno) {
    if (!no) {
        no = (tNo *) malloc(sizeof(tNo));
        if (!no) {
            printf("Sem memoria\n");
            exit(1);
        }
        no->aluno = aluno;
        no->esquerda = NULL;
        no->direita = NULL;

        if (!raiz)
            return no;

        if (aluno.RA < raiz->aluno.RA)
            raiz->esquerda = no;
        else
            raiz->direita = no;

        return no;
    }

    if (aluno.RA < no->aluno.RA)
        return inserirAluno(no, no->esquerda, aluno);
    else
        return inserirAluno(no, no->direita, aluno);
}

void print_arvore(tNo *no, int espaco) {
    if (!no) return;

    print_arvore(no->direita, espaco + 1);

    for (int i = 0; i < espaco; i++)
        printf("   ");
    printf("%d\n", no->aluno.RA);

    print_arvore(no->esquerda, espaco + 1);
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
    int opcao;

    do {
        printf("\n1 - Inserir aluno\n2 - Exibir arvore (RAs)\n0 - Sair\nEscolha: ");
        scanf("%d", &opcao);

        if (opcao == 1) {
            Aluno aluno;
            printf("RA: ");
            scanf("%d", &aluno.RA);
            getchar();
            printf("Nome: ");
            fgets(aluno.nome, sizeof(aluno.nome), stdin);
            aluno.nome[strcspn(aluno.nome, "\n")] = '\0';
            printf("Idade: ");
            scanf("%d", &aluno.idade);
            for (int i = 0; i < 4; i++) {
                printf("Nota %d: ", i + 1);
                scanf("%f", &aluno.notas[i]);
            }

            if (!raiz)
                raiz = inserirAluno(raiz, raiz, aluno);
            else
                inserirAluno(raiz, raiz, aluno);

        } else if (opcao == 2) {
            printf("\nArvore de RAs:\n");
            print_arvore(raiz, 0);
        }

    } while (opcao != 0);

    desalocar_arvore(raiz);
    return 0;
}
