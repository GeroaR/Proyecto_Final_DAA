#include <stdio.h>
#include <stdlib.h>

int num_productos = 0;
int num_demandas = 0;
int num_pedidos = 0;
int num_clientes = 0;
int num_localidades = 0;

NUM_MAX_PEDIDOS = 50;
NUM_MAX_DEMANDAS = 100;
NUM_MAX_DIRECC = 5;

// listas de tipo ENUM
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

typedef struct {
    int localidad_id; // *R_Localidad
    char calle[50]; // Calle del cliente
    char numero[10]; // Número exterior/interior (I:1340|E:B-20|)
    char colonia[50]; // Colonia o barrio
    char referencias[100]; // Descripción adicional (opcional)
} Direccion;

/* typedef struct {
    int dia;
    int mes;
    int anio;
} Fecha; */ //Por si acaso


// structs PRODUCTO, DEMANDA Y PEDIDO
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
    //char fecha[12]; // opcional
} Pedido;

typedef struct {
    int producto_id;
    int cantidad;
    //char fecha[12]; // opcional
} Demanda;

// structs LOCALIDAD y CARRETERA
typedef struct {
    int id;
    char nombre[50];
    Coordenadas coor;
    int historial_demandas[NUM_MAX_DEMANDAS];
    int num_demandas;
} Localidad;

typedef struct {
    char nombre[50];
    int origen_id; //*R_Localidad
    int destino_id; //*R_Localidad
    Medida distancia; // km
    float tiempo; // horas
    float penalizacion; // factor de tráfico
} Carretera;

// structs CAMION y CLIENTE
typedef struct {
    int id;
    Medida capacidad_peso;
    Medida capacidad_volumen;
    int ruta[20]; //*R_Localidad a visitar
    int num_paradas;
    EstadoCamion estado;
} Camion;

typedef struct {
    int id;
    char nombre[50];
    Direccion direcciones[5];
    int num_direcciones;
    int historial_pedidos[NUM_MAX_PEDIDOS]; //*R_Pedidos lista de demandas
    int num_pedidos;
} Cliente;


// SETS de cada 

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

int set_demanda(int id_producto,Demanda *lista_demandas, int cantidad) {
    /* if (num_demandas >= MAX_DEMANDAS) 
        return -1; */
    Demanda d = {id_producto, cantidad};
    lista_demandas[num_demandas] = d;
    return num_demandas++;
}

int set_pedido(Pedido *lista_pedidos,int id_producto, int cantidad) {
    /* if (num_pedidos >= MAX_PEDIDOS) 
        return -1; */
    Pedido p = {id_producto, cantidad};
    lista_pedidos[num_pedidos] = p;
    return num_pedidos++;
}

int set_cliente(Cliente *lista_clientes, char nombre[]) {
    /* if (num_clientes >= MAX_CLIENTES) 
        return -1; */
    Cliente c;
    c.id = num_clientes;
    strcpy(c.nombre, nombre);
    c.num_direcciones = 0;
    c.num_pedidos = 0;
    lista_clientes[num_clientes] = c;
    return num_clientes++;
}

int agregar_direccion_cliente(Cliente *lista_clientes,int id_cliente, Direccion d) {
    /* if (id_cliente >= num_clientes) 
        return -1; */
    Cliente* c = &lista_clientes[id_cliente];
    if (c->num_direcciones >= NUM_MAX_DIRECC) return -1;
    c->direcciones[c->num_direcciones++] = d;
    return 0;
}

int agregar_pedido_cliente(Cliente *lista_clientes, int cliente_id, int pedido_id) {
    /* if (cliente_id >= num_clientes || pedido_id >= MAX_PEDIDOS) 
        return -1; */
    Cliente* c = &lista_clientes[cliente_id];
    if (c->num_pedidos >= NUM_MAX_PEDIDOS) return -1;
    c->historial_pedidos[c->num_pedidos++] = pedido_id;
    return 0;
}

int set_localidad(Localidad *lista_localidades,char nombre[], Coordenadas coor) {
    /*if (num_localidades >= MAX_LOCALIDADES) 
        return -1; */
    Localidad l;
    l.id = num_localidades;
    strcpy(l.nombre, nombre);
    l.coor = coor;
    l.num_demandas = 0;
    lista_localidades[num_localidades] = l;
    return num_localidades++;
}

int agregar_demanda_localidad(Localidad *lista_localidades,int id_localidad, int id_demanda) {
    /* if (localidad_id >= num_localidades || demanda_id >= MAX_DEMANDAS) 
        return -1; */
    Localidad* l = &lista_localidades[id_localidad];
    if (l->num_demandas >= NUM_MAX_DEMANDAS) 
        return -1;
    l->historial_demandas[l->num_demandas++] = id_demanda;
    return 0;
}


// VALIDACIONES 
int validar_medida(Medida m, const char* campo) {
    if (m.valor < 0) {
        printf("❌ Error en %s: valor negativo.\n", campo);
        return 0;
    }
    return 1;
}

int es_texto_valido(char texto[]) {
    return strlen(texto) > 0;
}


int esta_en_rango(float valor, float min, float max, const char* campo) {
    if (valor < min || valor > max) {
        printf("❌ Error: %s fuera de rango (%.2f - %.2f). Valor recibido: %.2f\n", campo, min, max, valor);
        return 0;
    }
    return 1;
}





