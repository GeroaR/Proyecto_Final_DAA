#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#define NUM_MAX_PEDIDOS 50

int num_productos = 0;
int num_demandas = 0;
int num_pedidos = 0;
int num_clientes = 0;
int num_localidades = 0;

int NUM_MAX_DEMANDAS = 100;
int NUM_MAX_DIRECCIONES = 5;

//esta puede variar de acuerdo al maximo o minimo de un camion
int NUM_MAX_CARGA_PESO_KG = 500; 
int NUM_MAX_CARGA_VOL_M3 = 500;

//prioridad por numero de deamand

//listas de tipo ENUM
typedef enum {
    KG, G, M3, CM3, KM, H, MIN
} Unidad;

typedef enum { 
    DISPONIBLE,EN_RUTA,MANTENIMIENTO
} EstadoCamion;

typedef enum { 
    EN_ESPERA ,EN_CAMINO,COMPLETADO
} EstadoPedidos;


//structs para más clarida de inserción
typedef struct {
    float valor;
    Unidad unidad;
} Medida;

typedef struct {
    float lat;
    float lon;
} Coordenadas;

struct Localidad;
struct  PedidosxDireccion;

typedef struct {
    int id;
    struct Localidad *id_localidad; //*R_Localidad
    int id_cliente;
    int activo; // 1: Activo, 0: Eliminado
    char calle[50]; //Calle del cliente
    char numero[20]; //Número exterior/interior (I:1340|E:B-20|)
    char colonia[50]; //Colonia o barrio
    char referencias[100]; //Descripción adicional (opcional)
    struct PedidosxDireccion *historial_pedidos;
} Direccion; //ordenarlo POR LOCALIDAD

//structs CATEGORIA, PRODUCTO, DEMANDA Y PEDIDO
typedef struct {
    int id;
    char nombre[50];
    int numxcategoria; //cantidad de productos por categoria
} Categoria;

typedef struct {
    int id;
    Categoria *id_categoria; //si se elimina una categoria, pasa a ser: SIN CATEGORIA
    char nombre[50];
    float valor; //Valor económico
    Medida peso; //En kg o g
    Medida volumen; //En m^3 o cm^3
    int stock; //cantidad de producto en almacenamiento
    int estado; // 0: Inactivo, 1: Activo
} Producto; 
/*
Para buscar productos de acuerdo a categorías: 
    -Usar CountingSort
*/

typedef struct {
    Producto *id_producto; //*R_Producto
    int cantidad; //cuantas unidades
} Pedido;

typedef struct {
    int id;
    Direccion *id_direccion; //R_Direccion -> *R_Localidad (a cuál dirección del cliente)

    Pedido pedidos[NUM_MAX_PEDIDOS]; //ARRAY de pedidos
    int num_pedidos;
    float total_valor; //total a pagar
    Medida total_peso; //total de KG/G
    Medida total_volumen; //total de M3/CM3
    EstadoPedidos estado;
} PedidosxDireccion;
/*
Historial de Pedidos del Cliente
    -Que muestre los productos qué más ha pedido el cliente
    -Últimpos pedidos realizados
    -Se podría usar CountingSort
*/


typedef struct {
    int id;
    PedidosxDireccion *pedidos_clientes; //*R_Pedidos_Clientes x Localidad
    int num_pedidosxdireccion;
    float total_valor; //total a ganar
    Medida total_peso; //total de KG/G
    Medida total_volumen; //total de M3/CM3
} DemandasxLocalidad;

//structs LOCALIDAD y CARRETERA
typedef struct Localidad{
    int id;
    char nombre[50];
    Coordenadas coor;
} Localidad;

typedef struct {
    int *id_origen; //*R_Localidad
    int *id_destino; //*R_Localidad
    Medida distancia; //km
    float tiempo; //horas
    float penalizacion; //factor de tráfico
} Carretera;

typedef struct {
    int id;
    int id_camion;
    int *demandasxlocalidad; //en el arreglo debe coincidir la localidad con la demanda
    int *carreteras;
    int total_paradas;
    int total_tiempo;
    int total_distancia;
} Ruta; //el camión va recorriendo las carreteras en cierto tiempo, llega, pasa cierto tiempo y se marca completado en los pedidos.

//structs CAMION y CLIENTE
typedef struct {
    int id;
    Medida capacidad_peso;
    Medida capacidad_volumen;
    Ruta *historial_rutas;
    EstadoCamion estado;
} Camion; //COLA


typedef struct {
    int id;
    char nombre[50];
    Direccion *direcciones;
    int num_direcciones; //cantidad de direcciones que tiene
    int num_pedidos; //cantidad de pedidos realizados
} Cliente; //COLA


//SETS de cada 

int set_producto(Producto *lista_productos,char nombre[],float valor, Medida peso, Medida volumen) {
/*     if (num_productos >= MAX_PRODUCTOS) 
        return -1; */
    Producto p;
    p.id = num_productos;
    strcpy(p.nombre, nombre); //se copia lo que está en nombre y se pega dentro del mismo struct
    p.valor = valor;
    p.peso = peso;
    p.volumen = volumen;
    lista_productos[num_productos] = p;
    return num_productos++; // devuelve ID del producto recién creado
}


// VALIDACIONES TIPO DE DATO

// Limpia el buffer en caso de entrada incorrecta
void limpiar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Leer float validado
int leer_float_seguro(float *numero) {
    char buffer[100];
    char *endptr;

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return 0;
    *numero = strtof(buffer, &endptr);

    if (endptr == buffer || *endptr != '\n') {
        return 0; // error de conversión
    }
    return 1;
}

// Leer int validado
int leer_int_seguro(int *numero) {
    char buffer[100];
    char *endptr;

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return 0;
    *numero = strtol(buffer, &endptr, 10);

    if (endptr == buffer || *endptr != '\n') {
        return 0; // error de conversión
    }
    return 1;
}



// VALIDACIONES 
int validar_medida(Medida m, const char* campo) {
    if (m.valor < 0) {
        printf("❌ Error en %s: valor negativo.\n", campo);
        return 0;
    }
    return 1;
}

int es_texto_valido(char texto[],const char* campo) {
    if (strlen(texto)) {
        printf("❌ Error en %s: campo vacio.\n", campo);
        return 0;
    }
    return 1;
}

int esta_en_rango(float valor, float min, float max, const char* campo) {
    if (valor < min || valor > max) {
        printf("❌ Error: %s fuera de rango (%.2f - %.2f). Valor recibido: %.2f\n", campo, min, max, valor);
        return 0;
    }
    return 1;
}


void mostrar_unidades_posibles(Unidad opciones[], int num_opciones) {
    printf("Unidades disponibles:\n");
    for (int i = 0; i < num_opciones; i++) {
        printf(" %d. ", opciones[i]);
        switch(opciones[i]) {
            case KG: printf("KG\n"); break;
            case G: printf("G\n"); break;
            case M3: printf("M3\n"); break;
            case CM3: printf("CM3\n"); break;
            case KM: printf("KM\n"); break;
            case H: printf("Horas\n"); break;
            case MIN: printf("Minutos\n"); break;
        }
    }
}

// Función para solicitar y validar la medida
Medida pedir_medida(Unidad opciones[], int num_opciones, const char* campo) {
    Medida m;
    int unidad_valida = 0;
    int opcion_seleccionada;

    do {
        printf("Ingrese el valor de %s: ", campo);
        if (!leer_float_seguro(&m.valor)) {
            printf("❌ Entrada invalida. Ingrese un numero valido y hagalo de nuevo.\n");
            continue;
        }

        if (!validar_medida(m, campo)) {
            continue;
        }

        mostrar_unidades_posibles(opciones, num_opciones);
        printf("Seleccione la unidad (por numero): ");
        if (!leer_int_seguro(&opcion_seleccionada)) {
            printf("❌ Entrada invalida. Ingrese un numero valido y hagalo de nuevo.\n");
            continue;
        }

        for (int i = 0; i < num_opciones; i++) {
            if (opciones[i] == opcion_seleccionada) {
                m.unidad = opciones[i];
                unidad_valida = 1;
                break;
            }
        }

        if (!unidad_valida) {
            printf("❌ Unidad invalida. Intente de nuevo.\n");
        }

    } while (!unidad_valida);

    return m;
}

Coordenadas pedir_coordenadas() {
    Coordenadas c;
    printf("Ingrese la latitud (-90 a 90): ");
    if (!leer_float_seguro(&c.lat)) {
        printf("❌ Entrada invalida. Estableciendo latitud a 0.\n");
        c.lat = 0;
    }

    printf("Ingrese la longitud (-180 a 180): ");
    if (!leer_float_seguro(&c.lon)) {
        printf("❌ Entrada invalida. Estableciendo longitud a 0.\n");
        c.lon = 0;
    }

    if (c.lat < -90 || c.lat > 90) {
        printf("❌ Latitud fuera de rango. Estableciendo a 0.\n");
        c.lat = 0;
    }
    if (c.lon < -180 || c.lon > 180) {
        printf("❌ Longitud fuera de rango. Estableciendo a 0.\n");
        c.lon = 0;
    }

    return c;
}

int main() {
    SetConsoleOutputCP(65001); // Cambiar código de página a UTF-8 (para que se vean los emojis)

    // Ejemplo para peso
    Unidad opciones_peso[] = {KG, G};
    Medida peso = pedir_medida(opciones_peso, 2, "Peso");

    printf("✅ Medida ingresada: %.2f (Unidad %d)\n", peso.valor, peso.unidad);

    // Ejemplo para coordenadas
    Coordenadas coor = pedir_coordenadas();
    printf("✅ Coordenadas ingresadas: Lat %.4f, Lon %.4f\n", coor.lat, coor.lon);

    return 0;
}