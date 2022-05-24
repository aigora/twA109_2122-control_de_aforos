#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include <string.h>
#include "SerialClass/SerialClass.h"
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define AFORO 1
#define TAM 20

typedef struct
{
	int dia;
	int mes;
	int anio;
	int hora;
	int minuto;
	int segundo;
}FechaHora;

typedef struct
{
	int dia;
	int mes;
	int anyo;
	int hora;
	int duracion;
}fecha;

typedef struct
{
	char nombre[TAM];
	char apellido[TAM];
	int telefono;
	int penalizaciones;
	fecha fnacimiento;
	fecha cita;
	int presente;
}datos;


FechaHora ObtenerFechaHora(void);
int menu(void);
int nuevo_cliente(datos[], int);
int buscar_cliente(datos[], int);
void reservar_cita(datos[], int);
int entrada_gimnasio(datos[], int, int);
int comparar_fechas(fecha, FechaHora);
FechaHora calcular_hora_salida(fecha);
int comparar_horas(fecha, FechaHora);
void consultar_cliente(datos[], int);
void mostrar_datos_clientes(datos);
void listado_clientes(datos[], int);
void leer_fichero_usuarios_v1(datos*, int*, int);
int escribir_fichero_usuarios_v1(datos*, int);
//int crear_fichero(datos*, char, int);

int main(void)
{
	Serial* Arduino = NULL;
	char puerto[] = "COM3";
	int aforo = 0;

	int opcion_menu;
	int i = 0, exito = 0, n = 0;
	int longitud = TAM;
	datos clientes[TAM]; // Vector con los datos de los clientes
	int total_clientes = 0; // Total clientes del Gimnasio
	int contador = 0; // Aforo actual en el gimnasio
	FechaHora ahora;
	char mensaje_enviar[TAM];

	Arduino = new Serial((char*)puerto);

	if (Arduino->IsConnected())
	{
		sprintf_s(mensaje_enviar, "SET_AFORO\n");
		Arduino->WriteData((char*)mensaje_enviar, strlen(mensaje_enviar));
		sprintf_s(mensaje_enviar, "%d\n", aforo);
		Arduino->WriteData((char*)mensaje_enviar, strlen(mensaje_enviar));
	}
	else
		printf("Atención: No se ha podido conectar con Arduino\n");

	sprintf_s(mensaje_enviar, "SET_AFORO\n");
	Arduino->WriteData((char*)mensaje_enviar, strlen(mensaje_enviar));
	sprintf_s(mensaje_enviar, "%d\n", aforo);
	Arduino->WriteData((char*)mensaje_enviar, strlen(mensaje_enviar));

	leer_fichero_usuarios_v1(clientes, &n, longitud);

	setlocale(LC_ALL, "es-ES");
	do
	{
		ahora = ObtenerFechaHora();
		printf("Hoy es %2d/%2d/%4d\n", ahora.dia, ahora.mes, ahora.anio);
		printf("y son las %2d:%2d:%2d\n", ahora.hora, ahora.minuto, ahora.segundo);

		opcion_menu = menu();
		switch (opcion_menu)
		{
		case 1:
			total_clientes = nuevo_cliente(clientes, total_clientes);
			break;
		case 2:
			reservar_cita(clientes, total_clientes);
			break;
		case 3:
			contador = entrada_gimnasio(clientes, total_clientes, contador);
			break;
		case 4:
			if (buscar_cliente(clientes, total_clientes) != -1)
				contador--;
			break;

		case 5:
			consultar_cliente(clientes, total_clientes);
			break;
		case 6:
			listado_clientes(clientes, total_clientes);
			break;
		case 7:
			break;
		default:
			printf("Opción incorrecta\n");
		}
		if (total_clientes == 0)
			escribir_fichero_usuarios_v1(clientes, n);
	} while (opcion_menu != 7);
	return 0;
}

int menu(void)
{
	int opcion;
	char intro;

	printf("Introduzca la opcion deseada:\n 1. Registrarse\n 2. Solicitar Cita\n 3. Entrar Gimnasio\n ");
	printf("4. Salir Gimnasio\n 5. Datos clientes\n 6. Listado de clientes\n 7.Fin programa\n");
	scanf_s("%d", &opcion);
	scanf_s("%c", &intro); // Para que el intro tras la opción no se quede en el buffer.
	return opcion;
}

int nuevo_cliente(datos clientes[], int n)
{
	if (n < TAM)
	{
		printf("Introduzca su nombre:\n");
		gets_s(clientes[n].nombre, TAM);
		printf("Introduzca primer apellido:\n");
		gets_s(clientes[n].apellido, TAM);

		printf("Introduzca fecha de nacimiento d m a:\n");
		scanf_s("%d", &clientes[n].fnacimiento.dia);
		scanf_s("%d", &clientes[n].fnacimiento.mes);
		scanf_s("%d", &clientes[n].fnacimiento.anyo);

		printf("Introduzca su telefono:\n");
		scanf_s("%d", &clientes[n].telefono);
		clientes[n].presente = 0;
		clientes[n].cita.duracion = 0;
		clientes[n].penalizaciones = 0;
		n++;
	}
	else
		printf("No hay espacio en memoria para dar de alta nuevos clientes\n");
	return n;
}

int buscar_cliente(datos clientes[], int n)
{
	int i, posicion = -1; // Consideramos inicialmente que no existe
	char nombre[TAM];

	printf("Introduzca su nombre:\n");
	gets_s(nombre, TAM);
	for (i = 0; i < n && posicion == -1; i++)
		if (strcmp(nombre, clientes[i].nombre) == 0)
			posicion = i;
	return posicion;
}

void reservar_cita(datos clientes[], int n)
{
	int p;
	p = buscar_cliente(clientes, n);
	if (p == -1)
		printf("El cliente indicado no existe\n");
	else
	{
		printf("Introduzca fecha del dia que desea acudir (d m a):\n");
		scanf_s("%d", &clientes[p].cita.dia);
		scanf_s("%d", &clientes[p].cita.mes);
		scanf_s("%d", &clientes[p].cita.anyo);

		printf("Introduzca duracion de estancia (MAX 120min):\n");
		scanf_s("%d", &clientes[p].cita.duracion);

		printf("Introduzca la hora de llegada:\n"); // horas en punto
		scanf_s("%d", &clientes[p].cita.hora);
	}
	return;
}


int entrada_gimnasio(datos clientes[], int n, int aforo_actual)
{
	int tiene_cita = 0;
	FechaHora fechahora_actual;
	int p, i;
	int liberada_plaza = 0;
	p = buscar_cliente(clientes, n);
	if (p == -1)
		printf("El cliente indicado no existe\n");
	else
	{
		fechahora_actual = ObtenerFechaHora();
		if (comparar_fechas(clientes[p].cita, fechahora_actual) == 1 && comparar_horas(clientes[p].cita, fechahora_actual) == 1)
			tiene_cita = 1;
		if (tiene_cita == 1)
		{
			if (aforo_actual < AFORO)
			{
				printf("%s %s entra en el gimnasio\n", clientes[p].nombre, clientes[p].apellido);
				clientes[p].presente = 1;
				aforo_actual++;
			}
			else
			{
				printf("Aforo completo...intentando echar a alguien\n");
				for (i = 0; i < n && liberada_plaza == 0; i++)
				{
					if (clientes[i].presente == 1 && comparar_horas(clientes[i].cita, fechahora_actual) == 0)
					{
						clientes[i].presente = 0;
						clientes[i].penalizaciones++;
						printf("Echando a %s %s\n", clientes[i].nombre, clientes[i].apellido);
						liberada_plaza = 1;
						printf("%s %s entra en el gimnasio\n", clientes[p].nombre, clientes[p].apellido);
						clientes[p].presente = 1;
					}
				}
				if (liberada_plaza == 0)
					printf("Aforo completo. No se ha podido liberar ninguna plaza\n");
			}
		}
	}
	return aforo_actual;
}

int comparar_fechas(fecha f1, FechaHora f2)
{
	int iguales;
	if (f1.dia == f2.dia && f1.mes == f2.mes && f1.anyo == f2.anio)
		iguales = 1;
	else
		iguales = 0;
	return iguales;
}

int comparar_horas(fecha f1, FechaHora f2)
{
	int compatibles = 0;
	FechaHora hora_salida;

	hora_salida = calcular_hora_salida(f1); // Calculamos la hora de salida

	if (f2.hora >= f1.hora && f2.hora < hora_salida.hora)  // Si la hora actual es mayor o igual a la de la cita y menor a la salida
		compatibles = 1;
	else
		if (f2.hora >= f1.hora && f2.hora == hora_salida.hora && f2.minuto < hora_salida.minuto)  // Para los minutos
			compatibles = 1;
	return compatibles;
}

FechaHora calcular_hora_salida(fecha f1)
{
	FechaHora hora_salida;
	int total_minutos_salida;

	total_minutos_salida = f1.hora * 60 + f1.duracion;
	hora_salida.hora = total_minutos_salida / 60;
	hora_salida.minuto = total_minutos_salida % 60;
	return hora_salida;
}

void consultar_cliente(datos clientes[], int n)
{
	int p;
	p = buscar_cliente(clientes, n);
	if (p == -1)
		printf("El cliente indicado no existe\n");
	else
	{
		mostrar_datos_clientes(clientes[p]);
	}
}

FechaHora ObtenerFechaHora(void)
{
	FechaHora fh = { 0,0,0,0,0,0 };
	time_t ltime;
	struct tm today;
	errno_t err;

	_tzset(); // Establecemos la "Time Zone" del S.O.
	time(&ltime);
	err = _localtime64_s(&today, &ltime);
	if (err)
		printf("Error al capturar la fecha/hora\n");
	else
	{
		fh.dia = today.tm_mday;
		fh.mes = 1 + today.tm_mon;
		fh.anio = 1900 + today.tm_year;
		fh.hora = today.tm_hour;
		fh.minuto = today.tm_min;
		fh.segundo = today.tm_sec;
	}
	return fh;
}

void mostrar_datos_clientes(datos clientes)
{
	printf("Nombre y apellido=%s\t%s\t", clientes.nombre, clientes.apellido);
	printf("Fecha nacimiento=%d\t%d\t%d\t", clientes.fnacimiento.dia, clientes.fnacimiento.mes, clientes.fnacimiento.anyo);
	printf("Teléfono=%d\n", clientes.telefono);
	printf("Penalizaciones=%d\n", clientes.penalizaciones);
}

void listado_clientes(datos clientes[], int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		printf("Cliente % d\n", i + 1);
		mostrar_datos_clientes(clientes[i]);
	}
	return;
}

void leer_fichero_usuarios_v1(datos p[], int* pnumero, int longitud)
{
	FILE* fichero; // Puntero para manipular el fichero
	int num = 0; // Variable auxiliar para numero de usuarios leídos
	int i, pos; // Variable bucle y posicion final cadena
	errno_t cod_error; // Codigo de error tras el proceso de apertura.
	char intro[2];

	cod_error = fopen_s(&fichero, "Usuario.txt", "rt"); // Se intenta abrir el fichero de texto
	if (cod_error != 0)  // Si el fichero no se ha podido abrir
		*pnumero = 0; // La lista estará vacía
	else  // Si el fichero ha podido abrirse 
	{
		fscanf_s(fichero, "%d", &num); // Se lee la cantidad de registros
		if (num == 0) // Si esa cantidad es cero
			*pnumero = 0; // La lista estará vacía
		else  // Si hay registros para leer (según el entero leído)
		{
			if (num > longitud) // Si no hay memoria suficiente
			{
				printf("Memoria insuficiente para almacenar los datos del fichero\n");
				*pnumero = 0;
			}
			else // Si hay memoria suficiente
			{
				fgets(intro, 2, fichero); // Saltamos el intro que hay tras el número 
				for (i = 0; i < num; i++)  // Se leen los registros uno por uno
				{
					fgets((p + i)->nombre, TAM, fichero);
					pos = strlen((p + i)->nombre);  // Calcula la longitud del nombre para ubicar el \n.
					(p + i)->nombre[pos - 1] = '\0';
					fgets((p + i)->apellido, TAM, fichero);
					pos = strlen((p + i)->apellido);  // Calcula la longitud del nombre para ubicar el \n.
					(p + i)->apellido[pos - 1] = '\0';
					fscanf_s(fichero, "%d", &(p + i)->penalizaciones);
					// fgets(intro, 2, fichero); ??
					fscanf_s(fichero, "%d", &(p + i)->telefono);
					fgets(intro, 2, fichero); // Saltamos el intro que hay tras el número
				}
				*pnumero = num;
			}
		}
		fclose(fichero); // Se cierra el fichero
	}
}

int escribir_fichero_usuarios_v1(datos* lista, int numero)
{
	int i;
	FILE* fichero;
	errno_t err;

	err = fopen_s(&fichero, "Usuarios.txt", "w");
	if (err == 0) // Si el fichero se ha podido crear
	{
		fprintf(fichero, "%d\n", numero); // Se graba en el fichero el número de usuarios
		for (i = 0; i < numero; i++)
		{

			fprintf(fichero, "%s\n", (lista + i)->nombre);
			fprintf(fichero, "%s\n", (lista + i)->apellido);
			fprintf(fichero, "%d\n", (lista + i)->penalizaciones);
			fprintf(fichero, "%d\n", (lista + i)->telefono);
		}
		fclose(fichero);
	}
	else
		printf("Se ha producido un problema a la hora de grabar el fichero de usuarios\n");
	return err;
}