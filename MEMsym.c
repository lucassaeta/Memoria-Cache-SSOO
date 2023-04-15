#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TAM_LINEA 16
#define NUM_FILAS 8

typedef struct {
  unsigned char ETQ;
  unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

T_CACHE_LINE array[16];
int globaltime = 0;
int numfallos = 0;
int contador = 0;
char Simul_RAM[4096];
const char* binaryfilename = "CONTENTS_RAM.bin";
const char* textfilename = "accesos_memoria.txt";

//FUNCIONES

void LimpiarCACHE(T_CACHE_LINE array[NUM_FILAS]);
void ImprimirCache(T_CACHE_LINE* array);
int hexaToInt(char* dirmemory);
void ParsearDireccion(unsigned int addr, int* Etiqueta, int* palabra, int* linea, int* bloque);
void TratarFallo(T_CACHE_LINE* array, char* Simul_RAM, int ETQ, int linea, int bloque);
void VolcarCACHE(T_CACHE_LINE *tbl);

int main() {
  //LECTURA DE FICHEROS

  FILE* binaryfile = fopen(binaryfilename, "rb");
  if (!binaryfile) {
    printf("Error en la lectura del fichero de binario");
    return -1;
  }
  fread(Simul_RAM, sizeof(Simul_RAM), 1, binaryfile);
  
  FILE* textfile = fopen(textfilename, "r");
  if (!textfile) {
    printf("Error en la lectura del fichero de texto");
    return -1;
  }
  
  char dirmemory[4];
  char texto[100];
  int addr = 0;
  int Etiqueta;
  int palabra;
  int linea;
  int bloque;

  //FOR PARA IR LEYENDO CADA DIRECCION DEL ARCHIVO DE ACCESOS MEMORIA

  LimpiarCACHE(array);
  ImprimirCache(array);

  for (int i = 0; i < 14; i++) {
    Etiqueta = 0;
    linea = 0;
    palabra = 0;
    bloque = 0;
    
    fscanf(textfile, "\n%s\n", dirmemory);
    addr = hexaToInt(dirmemory);
    ParsearDireccion(addr, &Etiqueta, &palabra, &linea, &bloque);
    //printf("ETIQUETA: %02X LINEA: %02X PALABRA: %02X BLOQUE: %X\n", Etiqueta, linea, palabra, bloque);

    if ((int)array[linea].ETQ == Etiqueta) {
      printf("\nT: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X\n", globaltime, addr, Etiqueta, linea, palabra, array[linea].Data[palabra]);
      contador++;
    } else {
      numfallos++;
	  printf("T: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %X\n", globaltime,numfallos, addr,Etiqueta,linea,palabra, bloque);
	  globaltime = globaltime + 10;
	  TratarFallo(array, Simul_RAM, Etiqueta, linea, bloque);
	  printf("Cargando el bloque %X en la linea %02X\n", bloque, linea);
	  printf("T: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X\n\n", globaltime, addr, Etiqueta, linea, palabra, array[linea].Data[palabra]);
	  
	  ImprimirCache(array);
	  contador++;
	  sleep(1);
    }
    
    texto[i] = array[linea].Data[palabra];
    
  }
  
  printf("Numero de accesos totales: %d Numero de fallos: %d Tiempo medio: %d\n", contador, numfallos + 1 , globaltime/contador);
  printf("Texto leido: ");

  for(int k = 0 ; k < 12 ; k++){
	  printf("%c", texto[k]);
  }
  
  VolcarCACHE(array);
  fclose(textfile);
  fclose(binaryfile);
  return 0;
}

//ESTA FUNCION LIMPIA LA CACHE CON LOS CARACTERES 0XFF PARA LA ETQ Y 0X23 PARA EL RESTO

void LimpiarCACHE(T_CACHE_LINE array[NUM_FILAS]) {
  for (int i = 0; i < NUM_FILAS; i++) {
    array[i].ETQ = 0xFF;
    for (int j = 0; j < TAM_LINEA; j++) {
      array[i].Data[j] = (unsigned char) 0x23;
    }
  }
}

//ESTA FUNCION IMPRIME EL CONTENIDO DE LA CACHE

void ImprimirCache(T_CACHE_LINE* array) {
  for (int p = 0; p < NUM_FILAS; p++) {
    printf("ETQ: %02X ", array[p].ETQ);
    printf("Data: ");
    
    for (int l = 15; l >= 0; l--) {
      printf("%02X ", array[p].Data[l]);
    }

    printf("\n");
  }
}

//ESTA FUNCION CONVIERTE EL NUMERO HEXADECIMAL A INTEGER

int hexaToInt(char* dirmemory) {
  int entero = 0;
  int numero = 0;
  int potencia = 2;

  for (int j = 0; j < 4; j++) {
    switch (dirmemory[j]) {
    case '0':
      entero = 0;
      break;
      
    case '1':
      entero = 1;
      break;

    case '2':
      entero = 2;
      break;

    case '3':
      entero = 3;
      break;
      
    case '4':
      entero = 4;
      break;
      
    case '5':
      entero = 5;
      break;

    case '6':
      entero = 6;
      break;
      
    case '7':
      entero = 7;
      break;

    case '8':
      entero = 8;
      break;

    case '9':
      entero = 9;
      break;

    case 'A':
      entero = 10;
      break;

    case 'B':
      entero = 11;
      break;

    case 'C':
      entero = 12;
      break;

    case 'D':
      entero = 13;
      break;

    case 'E':
      entero = 14;
      break;

    case 'F':
      entero = 15;
      break;
    }
    numero = numero + (entero * pow(16, potencia));
    potencia--;
  }
  return numero;
}

//ESTA FUNCION PARSEA LA DIRECCION OBTENIDA EN EL ARCHIVO DE ACCESOS MEMORIA Y LA VUELCA EN LA CACHE

void ParsearDireccion(unsigned int addr, int *Etiqueta, int *palabra, int *linea, int *bloque) {
  int binario[12];
  int potenciaETQ = 4;
  int potenciaLinea = 2;
  int potenciaPalabra = 3;
  int potenciaBloque= 7;

  for (int i = 0; i < 12; i++) {
    binario[i] = (addr % 2);
    addr = addr / 2;
    //printf("%d", binario[i]);
  }

  for (int j = 12; j > 0; j--) {
    if (j < 13 && j > 7) { // ETQ
      *Etiqueta += binario[j - 1] * (int)pow(2, potenciaETQ);
      potenciaETQ--;
    } else if (j < 8 && j > 4) { // LINEA
      *linea += binario[j - 1] * (int)pow(2, potenciaLinea);
      potenciaLinea--;
    } else if (j < 5 && j > 0) { // PALABRA
      *palabra += binario[j - 1] * (int)pow(2, potenciaPalabra);
      potenciaPalabra--;
    }

    if(j < 13 && j > 4){
		*bloque += binario[j-1] * (int)pow(2,potenciaBloque);
		potenciaBloque--;
	}
  }
  printf("\n");
}

void TratarFallo(T_CACHE_LINE *array, char *Simul_RAM, int Etiqueta, int linea, int bloque){
	
	for(int j = 0 ; j < TAM_LINEA ; j++){
		
		array[linea].ETQ = Etiqueta;
		array[linea].Data[j] = Simul_RAM[(bloque*TAM_LINEA)+j];
		
	}

}

void VolcarCACHE(T_CACHE_LINE *array){
	FILE* cachebinaryfile = fopen("CONTENTS_CACHE.bin", "wb");

    if (!cachebinaryfile) {
    printf("Error en la escritura del fichero de binario");
    exit (1);
    }
		fwrite(array, 128, 1, cachebinaryfile); 
	
  fclose(cachebinaryfile);
}


