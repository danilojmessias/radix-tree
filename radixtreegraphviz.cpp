#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CHILDREN 256 
#define MAX_INPUT 100   

typedef struct RadixNo {
    char *chave;                          
    struct RadixNo *filhos[MAX_CHILDREN]; 
    int num_filhos;                 
    bool is_terminal;                  
} RadixNo;

typedef struct {
    RadixNo *root;
    int size;
} RadixTree;


RadixTree* cria_radix();
RadixNo* cria_radix_no(const char *chave);
void radix_no_free(RadixNo *no);
void radix_free(RadixTree *tree);
int insere_radix(RadixTree *tree, const char *chave);
bool busca_radix(RadixTree *tree, const char *chave);
void radix_print(RadixTree *tree);
void radix_exporta_graphviz(RadixTree *tree, const char *filename);
void menu_interativo(RadixTree *tree);
void limpar_buffer();

static int busca_prefixo_comum_tamanho(const char *str1, const char *str2);
static RadixNo* insere_radix_recursivo(RadixNo *no, const char *chave, int *inserido);
static bool busca_radix_recursivo(RadixNo *no, const char *chave);
static void radix_graphviz_recursivo(RadixNo *no, char *prefixo, int prefix_tam, FILE *file, int *no_id);

RadixTree* cria_radix() {
    RadixTree *tree = (RadixTree*)malloc(sizeof(RadixTree));
    if (!tree) return NULL;
    
    tree->root = cria_radix_no("");
    tree->size = 0;
    return tree;
}

RadixNo* cria_radix_no(const char *chave) {
    RadixNo *no = (RadixNo*)malloc(sizeof(RadixNo));
    if (!no) return NULL;
    
    no->chave = strdup(chave);
    no->num_filhos = 0;
    no->is_terminal = false;
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        no->filhos[i] = NULL;
    }
    
    return no;
}

void radix_no_free(RadixNo *no) {
    if (!no) return;
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (no->filhos[i]) {
            radix_no_free(no->filhos[i]);
        }
    }
    
    free(no->chave);
    free(no);
}

void radix_free(RadixTree *tree) {
    if (!tree) return;
    
    radix_no_free(tree->root);
    free(tree);
}

void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


static int busca_prefixo_comum_tamanho(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] && str2[i] && str1[i] == str2[i]) {
        i++;
    }
    return i;
}

int insere_radix(RadixTree *tree, const char *chave) {
    if (!tree || !chave || strlen(chave) == 0) return 0;
    
    int inserido = 0;
    tree->root = insere_radix_recursivo(tree->root, chave, &inserido);
    
    if (inserido) {
        tree->size++;
    }
    
    return inserido;
}

static RadixNo* insere_radix_recursivo(RadixNo *no, const char *chave, int *inserido) {
    if (!no) { //se nó for nulo cria uma chave inteira
        no = cria_radix_no(chave);
        no->is_terminal = true;
        *inserido = 1;
        return no;
    }
    
    int prefixo_comum_tam = busca_prefixo_comum_tamanho(no->chave, chave);
    int no_chave_tam = strlen(no->chave);
    int chave_tam = strlen(chave);
    
    if (prefixo_comum_tam == no_chave_tam) {
        if (prefixo_comum_tam == chave_tam) {
        
            if (!no->is_terminal) {
                no->is_terminal = true;
                *inserido = 1;
            }
            return no;
        } else { //nao é identico
        
            const char *chave_restante = chave + prefixo_comum_tam;
            unsigned char primeiro_char = (unsigned char)chave_restante[0];
            
            no->filhos[primeiro_char] = insere_radix_recursivo(
                no->filhos[primeiro_char], chave_restante, inserido
            );
            
            if (no->filhos[primeiro_char] && *inserido) {
                no->num_filhos++;
            }
            
            return no;
        }
    } else { //existe o prefixo mas o nó precisa ser dividido
       
        RadixNo *novo_no = cria_radix_no(no->chave + prefixo_comum_tam);
        novo_no->is_terminal = no->is_terminal;
        novo_no->num_filhos = no->num_filhos;
        

        for (int i = 0; i < MAX_CHILDREN; i++) {
            novo_no->filhos[i] = no->filhos[i];
            no->filhos[i] = NULL;
        }
        
        //atualiza o novo para ter apenas o prefixo comum
        char *old_chave = no->chave;
        no->chave = (char*)malloc(prefixo_comum_tam + 1);
        strncpy(no->chave, old_chave, prefixo_comum_tam);
        no->chave[prefixo_comum_tam] = '\0';
        free(old_chave);
        
        //nó agora intermediario
        no->is_terminal = false;
        no->num_filhos = 1;
        
        //conecta o novo nó como filho do nó com o prefixo comum
        unsigned char primeiro_char = (unsigned char)novo_no->chave[0];
        no->filhos[primeiro_char] = novo_no;
        
   
        if (prefixo_comum_tam == chave_tam) { //nova chave termina no prefixo comum
            no->is_terminal = true;
            *inserido = 1;
        } else { //ainda tem um sufixo, então insere de forma recursiva o novo filho
            const char *chave_restante = chave + prefixo_comum_tam;
            unsigned char new_primeiro_char = (unsigned char)chave_restante[0];
            
            no->filhos[new_primeiro_char] = insere_radix_recursivo(
                NULL, chave_restante, inserido
            );
            no->num_filhos++;
        }
        
        return no;
    }
}

void radix_exporta_graphviz(RadixTree *tree, const char *filename) {
    if (!tree || !filename) return;
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not open file %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "digraph RadixTree {\n");
    fprintf(file, "    rankdir=TB;\n");
    fprintf(file, "    node [shape=record, fontname=\"Arial\", fontsize=10];\n");
    fprintf(file, "    edge [fontname=\"Arial\", fontsize=8];\n");
    fprintf(file, "    \n");
    
    int no_id = 0;
    char prefixo[1000];
    radix_graphviz_recursivo(tree->root, prefixo, 0, file, &no_id);
    
    fprintf(file, "}\n");
    fclose(file);
    
    printf("Arquivo Graphviz exportado para: %s\n", filename);

}


static void radix_graphviz_recursivo(RadixNo *no, char *prefixo, int prefix_tam, FILE *file, int *no_id) {
    if (!no) return;
    
    int id_atual = (*no_id)++; //incrementa o id do proximo no
    
    int chave_tam = strlen(no->chave);
    strcpy(prefixo + prefix_tam, no->chave);
    int new_prefix_tam = prefix_tam + chave_tam;
    prefixo[new_prefix_tam] = '\0';
    
    char escaped_chave[1000];
    char escaped_prefixo[1000];
    int j = 0;
    
    for (int i = 0; no->chave[i] && j < 998; i++) { //escape para nao quebrar o .dot
        if (no->chave[i] == '"' || no->chave[i] == '\\' || no->chave[i] == '\n') {
            escaped_chave[j++] = '\\';
        }
        escaped_chave[j++] = no->chave[i];
    }
    escaped_chave[j] = '\0';
    
    j = 0;
    for (int i = 0; prefixo[i] && j < 998; i++) {//mesmo pro prefixo
        if (prefixo[i] == '"' || prefixo[i] == '\\' || prefixo[i] == '\n') {
            escaped_prefixo[j++] = '\\';
        }
        escaped_prefixo[j++] = prefixo[i];
    }
    escaped_prefixo[j] = '\0';
    
   
    if (strlen(no->chave) == 0) { //para raiz
        
        if (no->is_terminal) {
            fprintf(file, "    node%d [label=\"{ROOT|PALAVRA}\", style=filled, fillcolor=lightblue];\n", 
                   id_atual);
        } else {
            fprintf(file, "    node%d [label=\"ROOT\", style=filled, fillcolor=lightgray];\n", id_atual);
        }
    } else {
    
        if (no->is_terminal) { //define palavra inteira com verde
            fprintf(file, "    node%d [label=\"{%s|palavra: %s}\", style=filled, fillcolor=lightgreen];\n", 
                   id_atual, escaped_chave, escaped_prefixo);
        } else { //prefixo com amarelo
            fprintf(file, "    node%d [label=\"{%s|prefixo: %s}\", style=filled, fillcolor=lightyellow];\n", 
                   id_atual, escaped_chave, escaped_prefixo);
        }
    }
    

    for (int i = 0; i < MAX_CHILDREN; i++) { 
        if (no->filhos[i]) {
            int child_id = *no_id;
            radix_graphviz_recursivo(no->filhos[i], prefixo, new_prefix_tam, file, no_id);
            
            //forma de nao quebrar o .dot
            char aresta_label[10];
            if (i >= 32 && i <= 126) { 
                if (i == '"' || i == '\\') {
                    snprintf(aresta_label, sizeof(aresta_label), "'%c'", (char)i);
                } else {
                    snprintf(aresta_label, sizeof(aresta_label), "%c", (char)i);
                }
            } else {
                snprintf(aresta_label, sizeof(aresta_label), "\\\\x%02X", i);
            }
            //aqui escreve a aresta
            fprintf(file, "    node%d -> node%d [label=\"%s\"];\n", 
                   id_atual, child_id, aresta_label);
        }
    }
}

bool busca_radix(RadixTree *tree, const char *chave) {
    if (!tree || !chave || strlen(chave) == 0) return false;
    
    return busca_radix_recursivo(tree->root, chave);
}

static bool busca_radix_recursivo(RadixNo *no, const char *chave) {
    if (!no) return false;
    
    int prefixo_comum_tam = busca_prefixo_comum_tamanho(no->chave, chave);
    int no_chave_tam = strlen(no->chave);
    int chave_tam = strlen(chave);
    
    if (prefixo_comum_tam == no_chave_tam) {
        if (prefixo_comum_tam == chave_tam) {
            return no->is_terminal;
        } else {
            const char *chave_restante = chave + prefixo_comum_tam;
            unsigned char primeiro_char = (unsigned char)chave_restante[0];
            return busca_radix_recursivo(no->filhos[primeiro_char], chave_restante);
        }
    }
    
    return false;
}

void menu_interativo(RadixTree *tree) {
    int opcao;
    char palavra[MAX_INPUT];
    bool resultado;
    
    do {
        printf("\n========================================\n");
        printf("    RADIX TREE\n");
        printf("========================================\n");
        printf("1. Adicionar palavra\n");
        printf("2. Buscar palavra\n");
        printf("3. Exportar para Graphviz (.dot)\n");
        printf("0. Sair\n");
        printf("========================================\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf(" Entrada Invalida! Digite um numero.\n");
            limpar_buffer();
            continue;
        }
        limpar_buffer();
        
        switch (opcao) {
            case 1:
                printf("\n--- ADICIONAR PALAVRA ---\n");
                printf("Digite a palavra: ");
                if (fgets(palavra, sizeof(palavra), stdin) != NULL) {
                    palavra[strcspn(palavra, "\n")] = 0;
                    
                    if (strlen(palavra) == 0) {
                        printf(" Palavra nao pode estar vazia!\n");
                        break;
                    }
                    
                    int resultado = insere_radix(tree, palavra);
                    if (resultado) {
                        printf("Palavra '%s' adicionada com sucesso!\n", palavra);
                        printf("Total de palavras: %d\n", tree->size);
                    } else {
                        printf("Palavra '%s' ja existe\n", palavra);
                    }
                } else {
                    printf(" Erro ao ler a palavra!\n");
                }
                break;
                
            case 2:
                printf("\n--- BUSCAR PALAVRA ---\n");
                printf("Digite a palavra para buscar: ");
                if (fgets(palavra, sizeof(palavra), stdin) != NULL) {
                    palavra[strcspn(palavra, "\n")] = 0;
                    
                    if (strlen(palavra) == 0) {
                        printf(" Palavra nao pode estar vazia!\n");
                        break;
                    }
                    
                    resultado = busca_radix(tree, palavra);
                    if (resultado) {
                        printf("Palavra '%s' encontrada!\n", palavra);
                    } else {
                        printf("Palavra '%s' nao encontrada.\n", palavra);
                    }
                } else {
                    printf(" Erro ao ler a palavra!\n");
                }
                break;
                
            case 3:
                printf("\n--- EXPORTAR PARA GRAPHVIZ ---\n");
                if (tree->size == 0) {
                    printf("O dicionario esta vazio.\n");
                } else {
                    radix_exporta_graphviz(tree, "radix_tree_teste.dot");
                }
                break;
                
            case 0:
                printf("\n Finalizando programa...\n");
                break;
                
            default:
                printf(" Opcao invalida\n");
                break;
        }
    } while (opcao != 0);
}

// Main function with interactive menu
int main() {
    printf("=== INICIALIZANDO RADIX TREE ===\n");
    
    RadixTree *tree = cria_radix();
    if (!tree) {
        printf(" Erro ao criar a arvore\n");
        return 1;
    }
    
    
    // Start interactive menu
    menu_interativo(tree);
    
    // Cleanup
    radix_free(tree);
    
    return 0;
}