/* Breno Perricone Fischer 1810349 3WB */

#include "gera.h"
#include<stdlib.h>


static void error (const char *msg, int line) {
  fprintf(stderr, "erro %s na linha %d\n", msg, line);
  exit(EXIT_FAILURE);
}

/* funcoes auxiliares */
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

void movVar1Const (unsigned char *codigo, int i, int idx) {
  /* mov const1 para aux1 */
  unsigned char byte;
  int j;
  codigo[i] = 0xb9;
  i++;
  for (j=0;j<4;j++) {
    byte = (idx >> (8*j)) & 0xFF;
    codigo[i+j] = byte;
   }
   /* fim mov const1 para aux1 */
}
void movVar2Const (unsigned char *codigo, int i, int idx) {
    /* mov const2 para aux2 */
    unsigned char byte;
    int j;
    codigo[i] = 0x41;
    codigo[i+1] = 0xb8;
    i+=2;
    for (j=0;j<4;j++) {
      byte = (idx >> (8*j)) & 0xFF;
      codigo[i+j] = byte;
    }
   /* fim mov const2 para aux2 */
}
void movVar1V (unsigned char *codigo, int i, int idx) {
  /* mov var 1 para aux1 */
  codigo[i] = 0x8b;
  codigo[i+1] = 0x4d;
  codigo[i+2] = atribuiMovVariavel(idx);
  /* fim mov var1 para aux1 */
}

void movVar2V (unsigned char *codigo, int i, int idx) {
  codigo[i] = 0x44;
  codigo[i+1] = 0x8b;
  codigo[i+2] = 0x45;
  codigo[i+3] = atribuiMovVariavel(idx);
}

void movFimOperacaoA (unsigned char *codigo, int i, int idx) {
  codigo[i] = 0x89;
  codigo[i+1] = 0x4d;
  codigo[i+2] = atribuiMovVariavel(idx);
}
/* ****** fim funcoes auxiliares ******* */


funcp gera(FILE *f) {
  unsigned char **enderecoLinha;
  unsigned char *codigo, byte;
  int guardaN[30];
  int i,c,j=0,line=1,l=0,x=0,p,count=0;
  
  enderecoLinha = (unsigned char **) malloc (30*sizeof(double));
  if (enderecoLinha == NULL) return NULL;

  /* aloca um valor equialente a 4000 bytes(abre espaco para o pior caso para 30 linhas de comandos    Simples) */
  codigo = (unsigned char *) malloc (4000*sizeof(char));
  if (codigo == NULL) return NULL; 

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
  l=0;
  while ((c = fgetc(f)) != EOF) {
  enderecoLinha[l] = &codigo[i]; //guarda endereco da instrucao de cada linha
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
        codigo[i] = 0xc9; //leavq 
        codigo[i+1] = 0xc3; //retq
        i+=2;
        break;        
      }
      
      case 'v': { /* atribuicao e operacao aritmetica */
        int idx0, idx1;
        char c0, var1;
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
            codigo[i+1] = atribuiMovParam(idx1);
            codigo[i+2] = atribuiMovVariavel(idx0);
            i+=3;
          }
          else { //atribui variavel
            /* passa para uma aux o valor da variavel a ser atribuida */
            codigo[i] = 0x8b;
            codigo[i+1] = 0x4d;
            codigo[i+2] = atribuiMovVariavel(idx1);
            /* ********************** */
            /* passa a aux para a variavel a atribuir */
            codigo[i+3] = 0x89;
            codigo[i+4] = 0x4d;
            codigo[i+5] = atribuiMovVariavel(idx0);
            /* ********************** */
            i+=6;
          }
        }
        else { //operacao aritmetica
          char var2, op;
          int idx2;
          if (c0 != '=')
            error ("comando invalido", line);
          if (fscanf(f, " %c%d %c %c%d", &var1, &idx1, &op, &var2, &idx2) != 5)
            error("comando invalido", line);

          if (op == '-') { //operacao = '-'
            if (var1 == '$') { //var 1 = constante
              movVar1Const(codigo, i, idx1);
              i+=5;

              if (var2 == '$') { //CONST - CONST
                movVar2Const(codigo, i, idx2);
                i+=6;
              }

              else if (var2 == 'v') { //CONST - V
                movVar2V(codigo, i, idx2);
                i+=4;
              } 
            }
            else if (var1 == 'v') { //var1 = v
              movVar1V(codigo, i, idx1);
              i+=3;

              if (var2 == 'v') { //V - V
                movVar2V(codigo, i, idx2);
                i+=4;
              }
              else if (var2 == '$') { //V - CONST
                movVar2Const(codigo, i, idx2);
                i+=6;
              }
            }
            /* sub aux2=aux1 - aux2 */
            codigo[i] = 0x44;
            codigo[i+1] = 0x29;
            codigo[i+2] = 0xc1;
            i+=3;
            /* fim sub ************* */
          }
          else if (op == '+') { //operacao '+'
            if (var1 == 'v') { //var1 = v
              movVar1V(codigo, i, idx1);
              i+=3;
              if (var2 == 'v') { // V + V
                movVar2V(codigo, i, idx2);
                i+=4;
              }
              else if (var2 == '$') { // V + Const
                movVar2Const(codigo, i, idx2);
                i+=6;
              }
            }
            else if (var1 == '$') { //var1 = const
              movVar1Const(codigo, i, idx1);
              i+=5;
              if (var2 == '$') { //Const + const
                movVar2Const(codigo, i, idx2);
                i+=6;
              }
              else if (var2 == 'v') { //const + v
                movVar2V(codigo, i, idx2);
                i+=4;
              }
            }
            /* add aux2=aux1 + aux2 */
            codigo[i] = 0x44;
            codigo[i+1] = 0x01;
            codigo[i+2] = 0xc1;
            i+=3;
            /* fim add ************ */
          }
          else if (op == '*') { //operacao '*'
            if (var1 == '$') { //var 1 = constante
              movVar1Const(codigo, i, idx1);
              i+=5;

              if (var2 == '$') { //CONST * CONST
                movVar2Const(codigo, i, idx2);
                i+=6;
              }

              else if (var2 == 'v') { //CONST * V
                movVar2V(codigo, i, idx2);
                i+=4;
              } 
            }
            else if (var1 == 'v') { //var1 = v
              movVar1V(codigo, i, idx1);
              i+=3;

              if (var2 == 'v') { //V * V
                movVar2V(codigo, i, idx2);
                i+=4;
              }
              else if (var2 == '$') { //V * CONST
                movVar2Const(codigo, i, idx2);
                i+=6;
              }
            }
            /* imul aux2=aux1 * aux2 */
            codigo[i] = 0x41;
            codigo[i+1] = 0x0f;
            codigo[i+2] = 0xaf;
            codigo[i+3] = 0xc8;
            i+=4;
            /* fim imul ************ */
          }
          /* ****** mov aux2 para v ******* */
          movFimOperacaoA(codigo, i, idx0);
          i+=3;
          /* ******* fim mov ********* */ 
        }
        break;
      }
      case 'i': { /* desvio condicional */
        char var0;
        int idx0, n;
        if (fscanf(f, "flez %c%d %d", &var0, &idx0, &n) !=3)
          error("comando invalido", line);
        
        codigo[i] = 0x83;
        codigo[i+1] = 0x7d;
        codigo[i+2] = atribuiMovVariavel(idx0);
        codigo[i+3] = 0x00;
        codigo[i+4] = 0x7e;
        codigo[i+5] = 0x00; //sera substituido pelo endereco da (linha n - linha i+6)
        guardaN[x] = n; //guarda a linha que sera desviada
        x++;
        i+=6;
        break;
      }
    }
    line ++;
    l++;
    fscanf(f, " "); 
  }
  for (p=0;p<i;p++) {
    if (codigo[p] == 0x7e) {
      codigo[p+1] = enderecoLinha[guardaN[count]-1] - &codigo[p+2];
      count++;
    }
  }  
  return (funcp)codigo;
}


void libera (void *pf) {
  free(pf);
}

