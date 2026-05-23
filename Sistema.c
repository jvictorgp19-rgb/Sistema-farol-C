#include <stdio.h> //entrada e saida
#include <stdlib.h> // memoria
#include <string.h> //strings e leitura
#include <conio.h> // getch paara leitura de senha e caracteres sem enter

// Declarados aqui pois sao chamados antes de serem definidos no codigo
/* ===== PROTOTIPOS ===== */ 
void menu();
void menuAdm();

// Representa um usuario cadastrado no sistema
/* ===== STRUCTS ===== */
struct Usuario {
    char nome[100];
    char cpf[15]; // 11 digitos + \0 = 12, mas 15 para folga e seguranca
    char senha[20];
    char nascimento[12];
};

// Representa uma ocorrencia registrada por um usuario
struct Ocorrencia {
    int  id;
    char descricao[500];
    char bairro[50];
    char dataHora[20];
    float lat, lon;
    char cpfUsuario[15];
};

struct Usuario usuarioLogado;

/* ===== FUNCOES AUX ===== */
void limpar() { system("cls"); } // Limpa o terminal para melhorar a visualizacao do menu
void pausar() { system("pause"); }

//cabecalho padronizado para cada tela do sistema
void titulo(char *txt) {
    printf("\n========================\n");
    printf("   %s\n", txt);
    printf("========================\n");    // titulo("bsshdh");
}

// Leitura manual caractere a caractere para evitar problemas com buffer do scanf
void ler(char *txt, int t) {
    int i = 0;
    int c;
    while (i < t - 1) {
        c = getch();
        if (c == 0 || c == 0xE0) {
            getch(); // consumir tecla especial (setas, F1, etc.)
            continue;
        }
        if (c == '\r' || c == '\n') break;
        if (c == '\b') {
            if (i > 0) { i--; printf("\b \b"); }
        } else if (c >= 32 && c <= 126) {
            txt[i++] = (char)c;
            putchar(c);
        }
    }
    txt[i] = '\0';
    printf("\n");
}

// Igual ao ler(), mas exibe '*' no lugar do caractere digitado para ocultar a senha
void lerSenha(char *senha, int t) {
    int i = 0;
    char c;
    while (i < t - 1) {
        c = getch();
        if (c == '\r' || c == '\n') break;
        if (c == '\b') {
            if (i > 0) { i--; printf("\b \b"); }
        } else {
            senha[i++] = c;
            printf("*");
        }
    }
    senha[i] = '\0';
    printf("\n");
}

// Retorna 1 se a data for valida, 0 caso contrario
int validarData(char *data) {
    int d, m, a;
    if (strlen(data) != 10) return 0;
    if (data[2] != '/' || data[5] != '/') return 0;
    if (sscanf(data, "%d/%d/%d", &d, &m, &a) != 3) return 0;
    if (m < 1 || m > 12)      return 0;
    if (d < 1 || d > 31)      return 0;
    if (a < 1900 || a > 2100) return 0;
    return 1;
}

// Retorna 1 se a hora for valida no formato HH:MM, 0 caso contrario
int validarHora(char *hora) {
    int h, m;
    if (strlen(hora) != 5) return 0;
    if (hora[2] != ':') return 0;
    if (sscanf(hora, "%d:%d", &h, &m) != 2) return 0;
    if (h < 0 || h > 23) return 0;
    if (m < 0 || m > 59) return 0;
    return 1;
}

//FUNCOES DE ARQUIVO

// Verifica no arquivo se o CPF ja esta cadastrado, evitando duplicatas
int cpfExiste(char *cpf) {
    FILE *f = fopen("usuarios.txt", "r");
    if (f == NULL) return 0;

    char linha[250];
    struct Usuario u;
    int existe = 0;

    rewind(f);
    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%99[^;];%14[^;];%19[^;];%14[^\n]",
                   u.nome, u.cpf, u.senha, u.nascimento) == 4) {
            if (strcmp(cpf, u.cpf) == 0) {
                existe = 1;
                break;
            }
        }
    }

    fclose(f);
    return existe;
}

// Percorre todas as ocorrencias e retorna o maior ID encontrado + 1
int proximoId() {
    FILE *f = fopen("ocorrencias.txt", "a+");
    char linha[750];
    int maior = 0;
    struct Ocorrencia o;

    if (f == NULL) return 1;
    rewind(f);

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%d;%499[^;];%49[^;];%19[^;];%f;%f;%14[^\n]",
                   &o.id, o.descricao, o.bairro,
                   o.dataHora, &o.lat, &o.lon, o.cpfUsuario) == 7) {
            if (o.id > maior) maior = o.id;
        }
    }
    fclose(f);
    return maior + 1;
}

// Autentica o usuario comparando CPF e senha com os dados do arquivo
/* ===== LOGIN USUARIO ===== */
void login() {
    FILE *f = fopen("usuarios.txt", "a+");
    struct Usuario u;
    char cpf[15], senha[20];
    char linha[250];

    if (f == NULL) {
        printf("Erro ao abrir arquivo de usuarios!\n");
        pausar();
        return;
    }

    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) {
        printf("Jj");
        fclose(f); 
        pausar();
        return;
    }
    rewind(f);

    titulo("LOGIN");
    printf("CPF(Sem os sinais!): ");  ler(cpf, 15);
    printf("Senha (Ate 20 caracteres.): "); lerSenha(senha, 20);

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%99[^;];%14[^;];%19[^;];%14[^\n]",
                   u.nome, u.cpf, u.senha, u.nascimento) == 4) {
            if (strcmp(cpf, u.cpf) == 0 &&
                strcmp(senha, u.senha) == 0) {
                usuarioLogado = u;
                fclose(f);
                menu();
                return;
			}
                
        }
    }
    

    fclose(f);
    printf("Usuario ou senha incorretos!\n");
    pausar();
}

// Coleta e valida os dados do novo usuario, depois grava no arquivo
/* ===== CADASTRO ===== */
void cadastrar() {
    FILE *f = fopen("usuarios.txt", "a");

    if (f == NULL) {
        printf("Erro ao abrir arquivo!\n");
        pausar();
        return;
    }

    struct Usuario u;
    char opcaoStr[10];

    titulo("CADASTRO");
    int valido;
    int tentativasNome = 0;
    do {
        tentativasNome++;
        printf("Nome: ");
        ler(u.nome, 100);
        // Verificar se nome é válido: não vazio e não apenas espaços
        valido = 0;
        for (int i = 0; u.nome[i]; i++) {
            if (u.nome[i] != ' ') {
                valido = 1;
                break;
            }
        }
        if (!valido) {
            printf("Nome nao pode ser vazio ou apenas espacos. Tente novamente.\n");
            if (tentativasNome >= 3) {
                char opcao;
                printf("Voce ja errou 3 vezes. Deseja voltar ao menu? (s/n): ");
                ler(opcaoStr, 10);
                opcao = opcaoStr[0];
                if (opcao == 's' || opcao == 'S') {
                    fclose(f);
                    return;
                }
                tentativasNome = 0;
            }
        }
    } while (!valido);

    int cpfRepetido;
    int tentativasCpf = 0;
    do {
        tentativasCpf++;
        printf("CPF (sem os sinais!): ");
        ler(u.cpf, 15);
        if (strlen(u.cpf) == 0) {
            printf("CPF nao pode ser vazio. Tente novamente.\n");
            cpfRepetido = 1; // forçar loop
        } else {
            cpfRepetido = cpfExiste(u.cpf);
            if (cpfRepetido) {
                printf("CPF ja cadastrado! Informe um CPF diferente.\n");
            }
        }
        if (cpfRepetido && tentativasCpf >= 3) {
            char opcao;
            printf("Voce ja errou 3 vezes. Deseja voltar ao menu? (s/n): ");
            ler(opcaoStr, 10);
            opcao = opcaoStr[0];
            if (opcao == 's' || opcao == 'S') {
                fclose(f);
                return;
            }
            tentativasCpf = 0;
        }
    } while (cpfRepetido);

    int tentativasSenha = 0;
    do {
        tentativasSenha++;
        printf("Senha (ate 20 caracteres.): ");
        lerSenha(u.senha, 20);
        if (strlen(u.senha) == 0) {
            printf("Senha nao pode ser vazia. Tente novamente.\n");
            if (tentativasSenha >= 3) {
                char opcao;
                printf("Voce ja errou 3 vezes. Deseja voltar ao menu? (s/n): ");
                ler(opcaoStr, 10);
                opcao = opcaoStr[0];
                if (opcao == 's' || opcao == 'S') {
                    fclose(f);
                    return;
                }
                tentativasSenha = 0;
            }
        }
    } while (strlen(u.senha) == 0);

    int tentativasData = 0;
    do {
        tentativasData++;
        printf("Data de nascimento (DD/MM/AAAA): ");
        ler(u.nascimento, 15);
        if (!validarData(u.nascimento)) {
            printf("Data invalida! Use DD/MM/AAAA.\n");
            if (tentativasData >= 3) {
                char opcao;
                printf("Voce ja errou 3 vezes. Deseja voltar ao menu? (s/n): ");
                ler(opcaoStr, 10);
                opcao = opcaoStr[0];
                if (opcao == 's' || opcao == 'S') {
                    fclose(f);
                    return;
                }
                tentativasData = 0;
            }
        }
    } while (!validarData(u.nascimento));

    fprintf(f, "%s;%s;%s;%s\n", u.nome, u.cpf, u.senha, u.nascimento);
    fclose(f);

    printf("Cadastrado com sucesso!\n");
    pausar();
}

// Registra uma nova ocorrencia vinculada ao usuario logado
// ==========NOVA OCORRENCIA============
void novaOcorrencia() {
    struct Ocorrencia o;
    char dataTemp[15];
    char horaTemp[10];
    char opcaoStr[10];

    FILE *f = fopen("ocorrencias.txt", "a+");
    if (f == NULL) {
        printf("Erro ao abrir arquivo!\n");
        pausar();
        return;
    }

    titulo("NOVA OCORRENCIA");

    o.id = proximoId();

    printf("Descricao(ex: Roubo a mao armada na Rua das Flores): "); ler(o.descricao, 500);
    printf("Bairro: ");    ler(o.bairro, 50);

    int tentativasData = 0;
    do {
        printf("Data da ocorrencia (DD/MM/AAAA): ");
        ler(dataTemp, 15);
        if (!validarData(dataTemp)) {
            tentativasData++;
            printf("Data invalida! Use DD/MM/AAAA.\n");
            if (tentativasData >= 3) {
                char opcao;
                printf("Voce ja errou 3 vezes. Deseja voltar ao menu? (s/n): ");
                ler(opcaoStr, 10);
                opcao = opcaoStr[0];
                if (opcao == 's' || opcao == 'S') {
                    fclose(f);
                    return;
                }
                tentativasData = 0;
            }
        }
    } while (!validarData(dataTemp));

    int tentativasHora = 0;
    do {
        printf("Hora da ocorrencia (HH:MM): ");
        ler(horaTemp, 10);
        if (!validarHora(horaTemp)) {
            tentativasHora++;
            printf("Hora invalida! Use HH:MM (ex: 14:30).\n");
            if (tentativasHora >= 3) {
                char opcao;
                printf("Voce ja errou 3 vezes. Deseja voltar ao menu? (s/n): ");
                ler(opcaoStr, 10);
                opcao = opcaoStr[0];
                if (opcao == 's' || opcao == 'S') {
                    fclose(f);
                    return;
                }
                tentativasHora = 0;
            }
        }
    } while (!validarHora(horaTemp));

    sprintf(o.dataHora, "%s %s", dataTemp, horaTemp);

    printf("Latitude: ");  
    char latStr[20];
    ler(latStr, 20);
    o.lat = atof(latStr);
    printf("Longitude: "); 
    char lonStr[20];
    ler(lonStr, 20);
    o.lon = atof(lonStr);
    // getchar(); // not needed now

    strcpy(o.cpfUsuario, usuarioLogado.cpf);

    fprintf(f, "%d;%s;%s;%s;%.6f;%.6f;%s\n",
            o.id, o.descricao, o.bairro, o.dataHora, o.lat, o.lon, o.cpfUsuario);
    fclose(f);

    printf("Registrado! ID: %d\n", o.id);
    pausar();
}

// Exibe todas as ocorrencias registradas no sistema
/* ===== LISTAR OCORRENCIAS ===== */
void listar() {
    FILE *f = fopen("ocorrencias.txt", "a+");
    if (f == NULL) {
        printf("Nenhuma ocorrencia registrada.\n");
        pausar();
        return;
    }

    struct Ocorrencia o;
    char linha[750];
    int total = 0;
    
    titulo("OCORRENCIAS");

    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) {
        printf("Nenhuma ocorrencia registrada.\n");
        fclose(f);
        pausar();
        return;
    }
    rewind(f);
    
    while (fgets(linha, sizeof(linha), f)) total++;
    rewind(f);

    printf("Total de ocorrencias: %d\n", total);

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%d;%499[^;];%49[^;];%19[^;];%f;%f;%14[^\n]",
                   &o.id, o.descricao, o.bairro,
                   o.dataHora, &o.lat, &o.lon, o.cpfUsuario) == 7) {
            printf("--------------------\n");
            printf("ID   : %d\n",     o.id);
            printf("Desc : %s\n",     o.descricao);
            printf("Bairro: %s\n",    o.bairro);
            printf("Data/Hora: %s\n", o.dataHora);
            printf("Coord: %.6f, %.6f\n\n", o.lat, o.lon);
        }
    }

    fclose(f);
    pausar();
}

// Filtra ocorrencias pelo nome do bairro usando strcmp
/* ===== BUSCAR ===== */
void buscar() {
    FILE *f = fopen("ocorrencias.txt", "a+");
    if (f == NULL) {
        printf("Nenhuma ocorrencia registrada.\n");
        pausar();
        return;
    }

    struct Ocorrencia o;
    char bairro[50];
    char linha[750];
    int encontrou = 0;

    titulo("BUSCAR POR BAIRRO");

    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) {
        printf("Nenhuma ocorrencia registrada.\n");
        fclose(f);
        pausar();
        return;
    }
    rewind(f);

    printf("Digite o bairro: ");
    ler(bairro, 50);

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%d;%499[^;];%49[^;];%19[^;];%f;%f;%14[^\n]",
                   &o.id, o.descricao, o.bairro,
                   o.dataHora, &o.lat, &o.lon, o.cpfUsuario) == 7) {
            if (strcmp(bairro, o.bairro) == 0) {
                printf("ID:%d | %s | %s\n",
                       o.id, o.dataHora, o.descricao);
                encontrou = 1;
            }
        }
    }

    if (!encontrou) printf("Nenhuma ocorrencia encontrada nesse bairro.\n");

    fclose(f);
    pausar();
}

// Remove uma ocorrencia pelo ID, mas so se pertencer ao usuario logado
// Logica: copia todas as linhas para arquivo temporario, exceto a deletada
// Depois substitui o arquivo original pelo temporario
/* ===== DELETAR OCORRENCIA ===== */
void deletar() {
    FILE *f    = fopen("ocorrencias.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (f == NULL || temp == NULL) {
        printf("Erro ao abrir arquivo!\n");
        if (f)    fclose(f);
        if (temp) fclose(temp);
        pausar();
        return;
    }

    struct Ocorrencia o;
    char linha[750];
    char idStr[10];
    int deletado = 0;

    titulo("DELETAR OCORRENCIA");

    printf("ID da ocorrencia: ");
    ler(idStr, 10);
    int id = atoi(idStr);

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%d;%499[^;];%49[^;];%19[^;];%f;%f;%14[^\n]",
                   &o.id, o.descricao, o.bairro,
                   o.dataHora, &o.lat, &o.lon, o.cpfUsuario) == 7) {
            if (o.id != id) {
                fprintf(temp, "%d;%s;%s;%s;%.6f;%.6f;%s\n",
                        o.id, o.descricao, o.bairro,
                        o.dataHora, o.lat, o.lon, o.cpfUsuario);
            } else {
                if (strcmp(o.cpfUsuario, usuarioLogado.cpf) == 0) {
                    deletado = 1;
                } else {
                    printf("Voce so pode deletar suas proprias ocorrencias.\n");
                    fprintf(temp, "%d;%s;%s;%s;%.6f;%.6f;%s\n",
                            o.id, o.descricao, o.bairro,
                            o.dataHora, o.lat, o.lon, o.cpfUsuario);
                }
            }
        }
    }

    fclose(f);
    fclose(temp);
    remove("ocorrencias.txt");
    if (rename("temp.txt", "ocorrencias.txt") != 0) {
        printf("Erro ao renomear arquivo!\n");
        remove("temp.txt"); // Limpar arquivo temporário
    } else {
        if (deletado) printf("Ocorrencia deletada com sucesso!\n");
        else          printf("ID nao encontrado!\n");
    }

    pausar();
}

/* ===== SOS ===== */
void sos() {
    char opStr[10];
    titulo("SOS - EMERGENCIA");
    printf("1 - Policia   (190)\n");
    printf("2 - SAMU      (192)\n");
    printf("3 - Bombeiros (193)\n");
    printf("0 - voltar\n");
    printf("opcao: ");
    ler(opStr, 10);
    int op = atoi(opStr);
    	
	if (op == 1) printf("Chamando Policia - 190\n");
    else if (op == 2) printf("Chamando SAMU    - 192\n");
    else if (op == 3) printf("Chamando Bombeiros - 193\n");
    else if (op == 0) return;
    else printf("Opcao invalida!\n");

    pausar();
 	
}

/* ===== MAPA ===== */
#define LINHAS  3
#define COLUNAS 5

void exibirMapa() {
    char mapa[LINHAS][COLUNAS];
    int i, j;
    char linhaStr[10], colunaStr[10], opcaoStr[10];
    char opcao;

    char *nomesColunas[COLUNAS] = {
        "  1-BoaVista",
        "  2-Madalena",
        "  3-Gracas",
        "  4-Varzea",
        "  5-Curado"  // O bairro do mestre!
    };

    for (i = 0; i < LINHAS; i++)
        for (j = 0; j < COLUNAS; j++)
            mapa[i][j] = '.';

    do {
        printf("\n|========================================|\n");
		printf("|         MAPA DE OCORRENCIAS           |\n");
		printf("|========================================|\n\n");
		
		printf("              "); // alinhar a coluna com a linha
        for (j = 0; j < COLUNAS; j++)
            printf("%-12s", nomesColunas[j]);
        printf("\n");

        for (i = 0; i < LINHAS; i++) {
            printf("Dentro de %dm  ", (i + 1) * 100);
            for (j = 0; j < COLUNAS; j++)
                printf("%-12c", mapa[i][j]);
            printf("\n");
        }
		
		do {
        printf("\nEscolha a faixa de proximidade (1 a 3): ");
        ler(linhaStr, 10);
        if ( linhaStr[0] == '\0') { printf("Essa linha nao existe!"); }
    	} while ( linhaStr[0] == '\0');
		
		int linha = atoi(linhaStr) - 1;
		
		do {
        printf("Escolha a coluna (1 a 5): ");
        ler(colunaStr, 10);
         if ( colunaStr[0] == '\0') { printf("Essa coluna nao existe!"); }
        
    	} while(colunaStr[0] == '\0');
		int coluna = atoi(colunaStr) - 1;

        if (linha >= 0 && linha < LINHAS &&
            coluna >= 0 && coluna < COLUNAS) {
            mapa[linha][coluna] = 'X';
        } else {
            printf("Posicao invalida!\n");
        }

        printf("\nDeseja fazer outra marcacao? (s/n): ");
        ler(opcaoStr, 10);
        opcao = opcaoStr[0];

    } while (opcao == 's' || opcao == 'S');

    printf("\nMapa ATUALIZADO:\n\n     ");
    for (j = 0; j < COLUNAS; j++)
        printf("%-12s", nomesColunas[j]);
    printf("\n");

    for (i = 0; i < LINHAS; i++) {
        printf("Dentro de %dm  ", (i + 1)* 100);
        for (j = 0; j < COLUNAS; j++)
            printf("%-12c", mapa[i][j]);
        printf("\n");
    }

    printf("\nPressione ENTER para voltar...");
    getchar();
    getchar();
}

/* ===== MENU USUARIO ===== */
void menu() {
    char opStr[10];
    while (1) {
        limpar();
        printf("|==========================================================|\n");
        printf("|                        MENU                              |\n");
        printf("|                  Usuario: %-30s                          |\n", usuarioLogado.nome);
        printf("|==========================================================|\n");
        printf("|                                                          |\n");
        printf("|                  1 - Nova Ocorrencia                     |\n");
        printf("|                  2 - SOS                                 |\n");
        printf("|                  3 - Feed de ocorrencias                 |\n");
        printf("|                  4 - Buscar por Bairro                   |\n");
        printf("|                  5 - Deletar Ocorrencia                  |\n");
        printf("|                  6 - Mapa de Ocorrencia                  |\n");
        printf("|                  0 - Sair                                |\n");
        printf("|                                                          |\n");
        printf("|==========================================================|\n");
        printf("Opcao: ");
        ler(opStr, 10);
        int op = atoi(opStr);

		switch (op) {
		
        case 1: novaOcorrencia(); break;
        case 2: sos(); break;
        case 3: listar(); break;
        case 4: buscar(); break;
        case 5: deletar(); break;
        case 6: exibirMapa(); break;
        
        default: printf("Opcao sair/incorreta."); return;
        
    }
}
}

// Lista todos os usuarios cadastrados sem exibir a senha
/* ===== ADM: LISTAR USUARIOS ===== */
void admListarUsuarios() {
    FILE *f = fopen("usuarios.txt", "a+");
    if (f == NULL) {
        printf("Nenhum usuario cadastrado.\n");
        pausar();
        return;
    }

    struct Usuario u;
    char linha[250];
    int total = 0;

    titulo("USUARIOS CADASTRADOS");

    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) {
        printf("Nenhum usuario cadastrado.\n");
        fclose(f);
        pausar();
        return;
    }
    rewind(f);

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%99[^;];%14[^;];%19[^;];%14[^\n]",
                   u.nome, u.cpf, u.senha, u.nascimento) == 4) {
            total++;
            printf("--------------------\n");
            printf("Nome : %s\n", u.nome);
            printf("CPF  : %s\n", u.cpf);
            printf("Nasc.: %s\n", u.nascimento);
        }
    }

    printf("--------------------\n");
    printf("Total: %d usuario(s)\n", total);
    fclose(f);
    pausar();
}

// Remove usuario pelo nome usando arquivo temporario, mesmo padrao do deletar()
/* ===== ADM: DELETAR USUARIO ===== */
void admDeletarUsuario() {
    FILE *f    = fopen("usuarios.txt", "r");
    FILE *temp = fopen("tempusr.txt", "w");

    if (f == NULL || temp == NULL) {
        printf("Erro ao abrir arquivo!\n");
        if (f)    fclose(f);
        if (temp) fclose(temp);
        pausar();
        return;
    }

    struct Usuario u;
    char linha[250];
    char nome[100];
    int deletado = 0;

    titulo("DELETAR USUARIO");

    printf("Nome do usuario a deletar: ");
    ler(nome, 100);

    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\n")] = 0;
        if (sscanf(linha, "%99[^;];%14[^;];%19[^;];%14[^\n]",
                   u.nome, u.cpf, u.senha, u.nascimento) == 4) {
            if (strcmp(nome, u.nome) == 0) {
                deletado = 1;
            } else {
                fprintf(temp, "%s;%s;%s;%s\n",
                        u.nome, u.cpf, u.senha, u.nascimento);
            }
        }
    }

    fclose(f);
    fclose(temp);
    remove("usuarios.txt");
    if (rename("tempusr.txt", "usuarios.txt") != 0) {
        printf("Erro ao renomear arquivo!\n");
        remove("tempusr.txt"); // Limpar arquivo temporário
    } else {
        if (deletado) printf("Usuario deletado com sucesso!\n");
        else          printf("Usuario nao encontrado!\n");
    }

    pausar();
}

// Login fixo hardcoded para o administrador do sistema
/* ===== LOGIN ADM ===== */
void loginAdm() {
    
    char usuario[7] = "adsAdm", senha[6] = "ads1n"; //SENHA!!!!!!
    char d_usuario[7], d_senha[6];


    titulo("LOGIN ADM");
    printf("Usuario: "); ler(d_usuario, 7);
    printf("Senha: ");   ler(d_senha, 6);

	if ( strcmp (d_usuario, usuario)==0 && strcmp (d_senha, senha)==0) {
		menuAdm();
	} else { 
		printf("LOGIN INCORRETO!!!!!!");
		pausar();
	}
    
    
  
}

/* ===== MENU ADM ===== */
void menuAdm() {
    char opStr[10];
     while (1) {
        limpar();
        printf("|==========================================================|\n");
        printf("|                  MENU ADMINISTRADOR                      |\n");
        printf("|==========================================================|\n");
        printf("|                                                          |\n");
        printf("|                  1 - Listar Usuarios                     |\n");
        printf("|                  2 - Deletar Usuario                     |\n");
        printf("|                  3 - Ver Ocorrencias                     |\n");
        printf("|                  0 - Sair                                |\n");
        printf("|                                                          |\n");
        printf("|==========================================================|\n");
        printf("Opcao: ");
        ler(opStr, 10);
        int op = atoi(opStr);

		switch(op) {
		
        case 1: admListarUsuarios(); break;
        case 2: admDeletarUsuario(); break;
        case 3: listar(); break;
        default : printf("Opcao sair/incorreta"); return;
    }
}

}

// Garante que os arquivos existam antes do programa comecar a usá-los
// fopen com "a" cria o arquivo se nao existir, sem apagar se ja existir
/* ===== CRIA ARQUIVOS ===== */
void criarArquivos() {
    FILE *f;
    f = fopen("usuarios.txt",    "a"); if (f) fclose(f);
    f = fopen("ocorrencias.txt", "a"); if (f) fclose(f);
    
}

// Ponto de entrada: inicializa arquivos e exibe o menu principal em loop
/* ===== MAIN ===== */
int main() {
    criarArquivos();

    char opStr[10];
    while (1) {
        limpar();
        printf("|==========================================================|\n");
        printf("|               SEJA BEM VINDO AO FAROL                    |\n");
        printf("|==========================================================|\n");
        printf("|                                                          |\n");
        printf("|                  1 - Login Usuario                       |\n");
        printf("|                  2 - Cadastro                            |\n");
        printf("|                  3 - Login ADM                           |\n");
        printf("|                  0 - Sair                                |\n");
        printf("|                                                          |\n");
        printf("|==========================================================|\n");
        printf("Opcao: ");
        ler(opStr, 10);
        int op = atoi(opStr);

		switch (op) {
		

        case 1:
            login();
        	break;
        
		case 2:
          	cadastrar();
        	break;
        
		case 3:
        	loginAdm();
        	break;
        default: 
        printf("saindo...");
        return 0;
    }
}

    return 0;
}
