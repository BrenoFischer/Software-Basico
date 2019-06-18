#include "gera.h"
#include<stdlib.h>


static void error (const char *msg, int line) {
  fprintf(stderr, "erro %s na linha %d\n", msg, line);
  exit(EXIT_FAILURE);
}

int atribuiMovVariavel (int idx) {
  if (idx == 1)
    return 0xfc;
  else if (idx == 2)
    return 0xf8;
  else if (idx == 3)
    return 0xf4;
  else if (idx == 4)
    return 0xf0;
  else
    return 0xec;
}

int atribuiMovParam (int idx) {
  if (idx == 1)
    return 0x7d;
  else if (idx == 2)
    return 0x75;
  else
    return 0x55;
}



funcp gera(FILE *f) {
  unsigned char *codigo, byte;
  int i,c,j=0,line=1;;

  /* aloca um valor equialente a 4000 bytes(abre espaco para o pior caso para 30 linhas de comandos    Simples) */
  codigo = (unsigned char *) malloc (30*sizeof(char));

  codigo[0] = 0x55; /* push %rbp */
  /* mov %rsp, %rbp */
  codigo[1] = 0x48; 
  codigo[2] = 0x89; 
  codigo[3] = 0xe5;
  /* ************* */
  /* sub 0x10, %rsp */
  codigo[4] = 0x48;
  codigo[5] = 0x83;
  codigo[6] = 0xec;
  codigo[7] = 0x14; 
  /* ************* */

  i=8;
  while ((c = fgetc(f)) != EOF) {
    switch (c) {
      case 'r': { /* retorno */
        char var0;
        int idx0;
        if (fscanf(f, "et %c%d", &var0, &idx0) != 2)
          error("comando invalido", line);

        if (var0 == '$') { //var eh constante
        //movl const para %eax
          codigo[i] = 0xb8;
          i++;
          
          for (j=0;j<4;j++) {
            byte = (idx0 >> (8*j)) & 0xFF; //pega o ultimo byte (corretamente shiftado)
            codigo[i+j] = byte;
          }
          i+=4;
        }
        else if (var0 == 'v') { //retorna variavel
          codigo[i] = 0x8b;
          codigo[i+1] = 0x45;
          codigo[i+2] = atribuiMovVariavel(idx0);
          i+=3;
        }
        else { //retorna parametro
          codigo[i] = 0x89;
          //inserir o outro byte (descobrir os valores de edi, esi e edx)
          i+=2;
        }
        break;        
      }
      
      case 'v': { /* atribuicao e operacao aritmetica */
        int idx0, idx1;
        char var0 = c, c0, var1;
        if (fscanf(f, "%d %c", &idx0, &c0) != 2)
          error("comando invalido", line);

        if (c0 == '<') { /* atribuicao */
          if (fscanf(f, " %c%d", &var1, &idx1) != 2)
            error("comando invalido", line);
          if (var1 == '$') { //atribui constante
            codigo[i] = 0xc7;
            codigo[i+1] = 0x45;
            codigo[i+2] = atribuiMovVariavel(idx0);
            i+=3;
            for (j=0;j<4;j++) {
              byte = (idx1 >> (8*j)) & 0xFF; //pega o ultimo byte (corretamente shiftado)
              codigo[i+j] = byte;
            }
            i+=4;
          }
          else if (var1 == 'p') { //atribui parametro
            codigo[i] = 0x89;
            i++;
            codigo[i] = atribuiMovParam(idx1);
            i++;
            codigo[i] = atribuiMovVariavel(idx0);
            i++;
          }
        }
        else { /* operacao aritmetica */
          char var2, op;
          int idx2;
          if (c0 != '=')
            error("comando invalido", line);
          if (fscanf(myfp, " %c%d %c %c%d", &var1, &idx1, &op, &var2, &idx2) != 5)
            error("comando invalido", line);
          printf("%d %c%d = %c%d %c %c%d\n", 
                 line, var0, idx0, var1, idx1, op, var2, idx2);
        }
        break;
      }
    }
    line ++;
    fscanf(f, " "); 
  }
  codigo[i] = 0xc9; //leavq 
  codigo[i+1] = 0xc3; //retq
  return (funcp)codigo;
}
