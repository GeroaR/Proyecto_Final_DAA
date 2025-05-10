#include <stdio.h>
#include <stdlib.h>


// listas de estado del camión o unidad
typedef enum {
    KG, G, M3, CM3, KM
} Unidad;

typedef enum { 
    DISPONIBLE, EN_RUTA, MANTENIMIENTO 
} EstadoCamion;


// structs para más claridad en la función
typedef struct {
    float valor;
    Unidad unidad;
} Medida;

typedef struct {
    float lat;
    float lon;
} Coordenadas;

/* typedef struct {
    int dia;
    int mes;
    int anio;
} Fecha; */ //Por si acaso


// structs PRODUCTO y DEMANDA 
typedef struct {
    int id;
    char nombre[50];
    float valor; // Valor económico
    Medida peso; // En kg o g
    Medida volumen; // En m^3 o cm^3
} Producto;

typedef struct {
    int producto_id;
    int cantidad;
} Demanda;

// structs LOCALIDAD y CARRETERA

typedef struct {
    int id;
    char nombre[50];
    Coordenadas coor;
    int num_demandas;
} Localidad;

typedef struct {
    int origen_id;
    int destino_id;
    Medida distancia; // km
    float tiempo; // horas
    float penalizacion; // factor de tráfico
} Carretera;


// strurcts CAMION y CLIENTE
typedef struct {
    int id;
    Medida capacidad_peso;
    Medida capacidad_volumen;
    int ruta[20];  // IDs de localidades a visitar
    int num_paradas;
    EstadoCamion estado;
} Camion;

typedef struct {
    int id;
    char nombre[50];
    int localidad_id[50]; // *R_Localidad lista de localidades
    int historial_pedidos[50]; // *R_Demanda lista de demandas
    int num_pedidos;
} Cliente;






