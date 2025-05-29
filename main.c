#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#define NUM_MAX_PEDIDOS 25
#define NUM_MAX_DEMANDA 100
#define CHAR_S 30
#define CHAR_M 60
#define CHAR_G 120
#define CHARSET 128

//esta puede variar de acuerdo al maximo o minimo de un camion
int NUM_MAX_CARGA_PESO_KG = 500; 
int NUM_MAX_CARGA_VOL_M3 = 500;

// --------------------------------------------------------------

struct Localidad;
struct  PedidosxDireccion;
struct Camion;

struct Nodo;
struct Cola;
struct TrieNode;

struct Cola colaLoc;
struct Cola colaCamion;
struct Cola colaCat;
struct Cola colaProd;
struct Cola colaCliente;
struct Cola colaDirecc;
struct Cola colaCarretera;
struct Cola colaPedidos;


struct TrieNode *trieProd;
int trie_actualizado = 1;  // bandera para saber si necesita reconstrucción


//prioridad por numero de demanda

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

// -------------------------------

//structs para más clarida de inserción
typedef struct {
    int contador_ids;
    int tam;
} ColaHeader;

typedef struct {
    float valor;
    Unidad unidad;
} Medida;

typedef struct {
    float lat;
    float lon;
} Coordenadas;

typedef struct {
    int id;
    struct Localidad *R_localidad; //*R_Localidad
    int id_localidad; //*R_Localidad
    int id_cliente;
    int activo; // 1: Activo, 0: Eliminado
    char calle[CHAR_M]; //Calle del cliente
    char numero[CHAR_S]; //Número exterior/interior (I:1340|E:B-20|)
    char colonia[CHAR_M]; //Colonia o barrio
    char referencias[CHAR_G]; //Descripción adicional (opcional)
    struct Cola *historial_pedidos;
} Direccion; //ordenarlo POR LOCALIDAD

// -------------------------------

//structs CATEGORIA Y PRODUCTO
typedef struct {
    int id;
    char nombre[CHAR_M];
    int numxcategoria; //cantidad de productos por categoria
    int estado; // 0: Inactivo, 1: Activo
} Categoria;

typedef struct {
    int id;
    Categoria *R_categoria; //si se elimina una categoria, pasa a ser: SIN CATEGORIA
    int id_categoria;
    char nombre[CHAR_S];
    float valor; //Valor económico
    Medida peso; //En kg o g
    Medida volumen; //En m^3 o cm^3
    int stock; //cantidad de producto en almacenamiento
    int estado; // 0: Inactivo, 1: Activo
} Producto; 

// -------------------------------
typedef struct {
    int id_producto;
    Producto *R_producto;
    int cantidad;
} Pedido; //

typedef struct {
    int id_pedido;

    int id_direccion; 
    Direccion *R_direccion; //como una direccion está asociada a un cliente, para sacar su historial
    Pedido pedidos[NUM_MAX_PEDIDOS];
    int tam;
    int total_cantidad;
    float total_valor;
    float total_peso; //kg
    float total_vol; //m3
    int id_camion; //0 o -1 como no asignado aún
    EstadoPedidos estado;
} CargaPedido;
//cuando se hace el pedido, está en espera (no se ha asignado a un camión y el camión aún no tiene ruta)
//cuando se le asigne a un camión se le asigna una ruta, se sustrae la cantidad de productos de la cantidad de productos en stock
/* Ordenar los productos que aún no se han asignado a un camión por nombre, peso y volumen 
• Ordenar los productos que se han asignado a un camión en específico por nombre, peso y volumen 
• Restricción de Complejidad: Deberá ocupar un algoritmo distinto para cada uno de los ordenes solicitados (o sea, 6 algoritmos de ordenamiento en total) 
en donde NO se permitirá un orden mayor o igual que 𝜃(𝑛^2)  */

typedef struct {
    int id;
    int id_camion;
    struct Camion *R_camion;

    CargaPedido productos[NUM_MAX_DEMANDA];
    int num_productos; //suma de total_cantidad
    float total_valor;
    float total_peso;
    float total_volumen;
} CargaCamion;
//AQUÍ VENDRÍA EL RESULTADO DE USAR KNAPSACK ALGORITHM, los productos pueden ser de otras direcciones o localidades
/* Asignar productos a cada camión de manera eficiente según su peso, volumen y demanda. 
• Garantizar que los camiones no excedan su capacidad de carga. 
• Elegir la opción de maximizar el valor total de los productos entregados o 
satisfacer la mayor demanda posible.
 */

typedef struct {
    int id;
    CargaCamion carga; //como CargaCamion puede tener de otras localidades, se obtiene un listado de las localidades a visitar y de ahí se calcula la ruta
    int *paradas; //arreglo de ids de las carreteras 
    float total_valor; 
    float tiempo_estimado; //de ida y de regreso
} RutaCamion;
/* Encontrar la mejor ruta utilizando dos algoritmos eficientes (uno para cada modo de optimización) para minimizar tiempo o distancia. 
Incluir dos modos de optimización:  
o Modo 1: Minimizar el tiempo total, aunque implique más distancia. 
o Modo 2: Minimizar la distancia, aunque implique más tiempo. 
Qué otro algoritmo me recomiendas, ademásd el Djikstra*/


//structs LOCALIDAD, CARRETERA y RUTA
typedef struct Localidad{
    int id;
    char nombre[CHAR_M];
    Coordenadas coor;
    int estado;
} Localidad;

typedef struct {
    int id;
    Localidad *R_origen; //*R_Localidad
    int id_origen;
    Localidad *R_destino; //*R_Localidad
    int id_destino;
    float distancia; //km
    float tiempo; //horas
    float penalizacion; //factor de tráfico
} Carretera;


// COLAS (Básicos)
typedef struct Nodo{
    void *dato;
    struct Nodo *siguiente;
} Nodo;

typedef struct Cola{
    Nodo *frente;
    Nodo *final;
    int tam;
    int contador_ids;  // opcional si tus entidades tienen id
}Cola;

// -------------------------------

//structs CAMION, CLIENTE y CLIENTE_DIRECCION
typedef struct Camion{
    int id;
    Medida capacidad_peso;
    Medida capacidad_volumen;
    EstadoCamion estado;
} Camion; //COLA

typedef struct {
    int id;
    int id_cliente;
    Direccion *direccion;
} Cliente_Direccion; //COLA

typedef struct {
    int id;
    char nombre[CHAR_S];
    Cola direcciones;
    int num_pedidos; //cantidad de pedidos realizados
} Cliente; //COLA

// -------------------------------
// struct TrieNode para la búsqueda de nombres de productos
typedef struct TrieNode {
    struct TrieNode *hijos[CHARSET];
    Producto *producto;  // Referencia al producto
} TrieNode;


void limpiar_buffer();
int leer_float_seguro(float *numero);
int leer_int_seguro(int *numero);


int validar_medida(Medida m, const char* campo);
int es_texto_valido(char texto[],const char* campo);
int esta_en_rango(float valor, float min, float max, const char* campo);
void mostrar_unidades_posibles(Unidad opciones[], int num_opciones);
Medida pedir_medida(Unidad opciones[], int num_opciones, const char* campo);
Coordenadas pedir_coordenadas();

Categoria* crearCategoria(Cola *cola, char nombre[]);
int criterioCategoriaPorID(void *dato, int id);
void mostrarCategoria(void *dato);
Categoria *buscarCategoria(Cola *colaCat, int idCat);
void editarCategoriaNombre(void *dato);

Localidad* crearLocalidad(Cola *cola, char nombre[], Coordenadas coor);
int criterioLocalidadPorID(void *dato, int id);
void mostrarLocalidad(void *dato);
Localidad *buscarLocalidad(Cola *colaLoc, int idLoc);
void editarLocalidad(void *dato);

Producto* crearProducto(Cola *cola, char nombre[], Categoria *R_categoria, float valor, Medida peso, Medida volumen, int stock);
int criterioProductoPorID(void *dato, int id);
void mostrarProducto(void *dato);
void editarProducto(void *dato);
void eliminarProducto(void *dato);
void reconstruirReferenciasProducto(Cola *productos, Cola *categorias);
void limpiarReferenciasProducto(Cola *productos);

Direccion *crearDireccion(Cola *cola, Localidad *localidad, int id_cliente, char calle[], char numero[], char colonia[], char referencias[]);
Cliente_Direccion* crearCliente_Direccion(Cola *cola, Direccion *direccion, int id_cliente);
int criterioDireccionPorID(void *dato, int id);
int criterioClienteDireccionPorID(void *dato, int id);
void mostrarDireccion(void *dato);
void mostrarClienteDireccion(void *dato);
void editarClienteDireccion(void *dato);
Cliente_Direccion *buscarClienteDireccion(Cola *colaClienteDir, int idDirecc);
void eliminarDireccion(void *dato);
void limpiarReferenciasDireccion(Cola *colaDirecc);
void asociarLocalidadesADirecciones(Cola *colaDirecc, Cola *colaLoc);


Cliente* crearCliente(Cola *cola, char nombre[]);
int criterioClientePorID(void *dato, int id);
void mostrarCliente(void *dato);
void editarCliente(void *dato);
Cliente *buscarCliente(Cola *cola, int id_buscar);
void agregarDireccionesACliente(Cliente *cliente, Cola *colaLoc, Cola *colaDirecc);
void limpiarReferenciasClienteDireccion(Cola *colaCliente, Cola *colaDirecc);


void inicializarCola(Cola *cola);
Nodo *encolar(Cola *cola, void *dato, size_t tam_dato);
void *desencolar(Cola *cola);
void listarCola(Cola *cola, void (*mostrar)(void *));
void editarCola(Cola *cola, int (*criterio)(void *, int), void (*editar)(void *));
void cargarCola(Cola *cola, const char *nombre_archivo, size_t tam_dato);
void guardarCola(Cola *cola, const char *nombre_archivo, size_t tam_dato);
void liberarCola(Cola *cola);
Nodo *extraerNodoPorCriterio(Cola *cola, int (*criterio)(void *, int));

void menu_admin();
int menu_edicion(const char *titulo, const char *campos[], int num_campos);
const char* obtener_nombre_unidad(Unidad u);
const char* obtener_nombre_estado_camion(EstadoCamion e);
const char* obtener_nombre_estado_pedido(EstadoPedidos e);

void submenu_categoria(Cola *colaCat);
void submenu_localidad(Cola*colaLoc);
void submenu_cliente(Cola *colaCliente, Cola *colaLoc, Cola *colaDirecc);
void menu_inventario(Cola *colaCat, Cola *colaLoc, Cola *colaProd, Cola *colaCliente, Cola *colaDirecc, Cola *colaCamion, Cola *colaCarretera);


TrieNode *crearNodoTrie();

void insertarProductoTrie(TrieNode *raiz, const char *nombre, Producto *producto);

void reconstruirTrieProductos(Cola *colaProd, TrieNode *raiz);
Producto *buscarProductoTrie(TrieNode *raiz, const char *nombre);
void liberarTrie(TrieNode *nodo);

// --------------------------- FUNCIONES PARA EVITAR ERROR DE INSERCION ---------------------------
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

// --------------------------- VALIDACION INFORMACION ---------------------------
int validar_medida(Medida m, const char* campo) {
    if (m.valor < 0) {
        printf("❌ Error en %s: valor negativo.\n", campo);
        return 0;
    }
    return 1;
}

int es_texto_valido(char texto[],const char* campo) {
    if (!strlen(texto)) {
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

Medida pedir_medida(Unidad opciones[], int num_opciones, const char* campo) {
    Medida m;
    int unidad_valida = 0;
    int opcion_seleccionada = 0;

    do {
        printf("Ingrese el valor de %s: ", campo);
        if (!leer_float_seguro(&m.valor)) {
            printf("❌ Entrada invalida. Ingrese un numero valido.\n");
            continue;
        }

        if (!validar_medida(m, campo)) {
            continue;
        }

        mostrar_unidades_posibles(opciones, num_opciones);
        printf("Seleccione la unidad (por numero): ");
        if (!leer_int_seguro(&opcion_seleccionada)) {
            printf("❌ Entrada invalida. Vuelva a INGRESAR TODO.\n");
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

    if (!esta_en_rango(c.lat,-90,90,"Latitud")) {
        c.lat = 0;
    }
    if (!esta_en_rango(c.lat,-180,180,"Longitud")) {
        c.lon = 0;
    }

    return c;
}

// --------------------------- FUNCIONES CATEGORIA ---------------------------
Categoria* crearCategoria(Cola *cola, char nombre[]) {
    Categoria *nuevo = (Categoria *)malloc(sizeof(Categoria));
    if (!nuevo)
        return NULL;
    cola->contador_ids++;
    nuevo->id = cola->contador_ids;
    strcpy(nuevo->nombre, nombre);
    nuevo->estado = 1;
    nuevo->numxcategoria = 0;
    return nuevo;
}

int criterioCategoriaPorID(void *dato, int id) {
    Categoria *c = (Categoria *)dato;
    return c->id == id;
}

void mostrarCategoria(void *dato) {
    Categoria *c = (Categoria *)dato;
    if (c->estado == 1)
        printf("ID: %d | Nombre: %s\n", c->id, c->nombre);
}

Categoria *buscarCategoria(Cola *colaCat, int idCat){
    Nodo *temp = colaCat->frente;
    while (temp != NULL) {
        Categoria *c = (Categoria *)temp->dato;
        if (c->id == idCat) 
            return c;
        temp = temp->siguiente;
    }
    return NULL;
}

void editarCategoriaNombre(void *dato) {
    Categoria *c = (Categoria *)dato;
    printf("Nuevo nombre de la categoría: ");
    fgets(c->nombre, sizeof(c->nombre), stdin);
    c->nombre[strcspn(c->nombre, "\n")] = 0;
    printf("✅ Nombre actualizado a: %s\n", c->nombre);
}

// --------------------------- FUNCIONES LOCALIDAD ---------------------------
Localidad* crearLocalidad(Cola *cola, char nombre[], Coordenadas coor) {
    Localidad *nuevo = (Localidad *)malloc(sizeof(Localidad));
    if (!nuevo)
        return NULL;
    cola->contador_ids++;
    nuevo->id = cola->contador_ids;
    strcpy(nuevo->nombre, nombre);
    nuevo->coor = coor;
    nuevo->estado = 1;
    return nuevo;
}

int criterioLocalidadPorID(void *dato, int id) {
    Localidad *l = (Localidad *)dato;
    return l->id == id;
}

void mostrarLocalidad(void *dato) {
    Localidad *l = (Localidad *)dato;
    if (l->estado == 1)
        printf("ID: %d | Nombre: %s | Lat: %.2f | Lon: %.2f\n", l->id, l->nombre, l->coor.lat, l->coor.lon);

}

Localidad *buscarLocalidad(Cola *colaLoc, int idLoc) {
    Nodo *tempL = colaLoc->frente;
    while (tempL != NULL) {
        Localidad *l = (Localidad *)tempL->dato;
        if (l->id == idLoc) {
            return l;
        }
        tempL = tempL->siguiente;
    }
    return NULL;
}

void editarLocalidad(void *dato) {
    Localidad *l = (Localidad *)dato;
    int opc = 0;
    const char *campos[] = {"nombre", "coordenadas"};
    do {
        opc = menu_edicion(l->nombre,campos,2);
        if (opc == -1) continue;

        switch (opc) {
            case 1:
                printf("Nuevo nombre: ");
                fgets(l->nombre, sizeof(l->nombre), stdin);
                l->nombre[strcspn(l->nombre, "\n")] = 0;
                printf("✅ Nombre actualizado.\n");
                break;
            case 2:
                printf("Ingrese nuevas coordenadas:\n");
                l->coor = pedir_coordenadas();
                printf("✅ Coordenadas actualizadas.\n");
                break;
            case 0:
                printf("↩ Volviendo al menú anterior.\n");
                break;
            default:
                printf("❌ Opción inválida.\n");
        }
    } while (opc != 0);

    return;
}

// --------------------------- FUNCIONES PRODUCTO ---------------------------
Producto* crearProducto(Cola *cola, char nombre[], Categoria *R_categoria, float valor, Medida peso, Medida volumen, int stock) {
    Producto *nuevo = (Producto *)malloc(sizeof(Producto));
    if (!nuevo)
        return NULL;
    cola->contador_ids++;
    nuevo->id = cola->contador_ids;
    strcpy(nuevo->nombre, nombre);
    nuevo->R_categoria = R_categoria;
    nuevo->id_categoria = R_categoria->id;
    nuevo->valor = valor;
    nuevo->peso = peso;
    nuevo->volumen = volumen;
    nuevo->stock = stock;
    nuevo->estado = 1;
    return nuevo;
}

int criterioProductoPorID(void *dato, int id) {
    Producto *p = (Producto *)dato;
    return p->id == id;
}

void mostrarProducto(void *dato) {
    Producto *p = (Producto *)dato;
    if (p->estado == 1)
        printf("ID: %d | Nombre: %s | Valor: %.2f | Peso: %.2f %s | Volumen: %.2f %s | Stock: %d | Categoría: %s\n",
               p->id, p->nombre, p->valor,
               p->peso.valor, obtener_nombre_unidad(p->peso.unidad),
               p->volumen.valor, obtener_nombre_unidad(p->volumen.unidad),
               p->stock, p->R_categoria->nombre);
}

Producto *buscarProducto(Cola *colaProd, int idProd) {
    Nodo *tempL = colaProd->frente;
    while (tempL != NULL) {
        Producto *l = (Producto *)tempL->dato;
        if (l->id == idProd) {
            return l;
        }
        tempL = tempL->siguiente;
    }
    return NULL;
}

// Función para editar producto con submenú de campos
void editarProducto(void *dato) {
    Producto *p = (Producto *)dato;
    int opc = 0;
    const char *campos[] = {"nombre", "valor", "peso", "volumen", "stock"};
    do {
        opc = menu_edicion(p->nombre, campos, 5);
        if (opc == -1) continue;

        switch (opc) {
            case 1:
                printf("Nuevo nombre: ");
                fgets(p->nombre, sizeof(p->nombre), stdin);
                p->nombre[strcspn(p->nombre, "\n")] = 0;
                printf("✅ Nombre actualizado.\n");
                trie_actualizado = 0;
                break;
            case 2:
                printf("Nuevo valor económico: ");
                if (leer_float_seguro(&p->valor)) {
                    printf("✅ Valor actualizado.\n");
                } else printf("❌ Entrada inválida.\n");
                break;
            case 3:
                printf("Ingrese nuevo peso:\n");
                p->peso = pedir_medida((Unidad[]){KG, G}, 2, "Peso");
                break;
            case 4:
                printf("Ingrese nuevo volumen:\n");
                p->volumen = pedir_medida((Unidad[]){M3, CM3}, 2, "Volumen");
                break;
            case 5:
                printf("Nuevo stock: ");
                if (leer_int_seguro(&p->stock)) {
                    printf("✅ Stock actualizado.\n");
                } else printf("❌ Entrada inválida.\n");
                break;
            case 0:
                printf("↩ Volviendo al menú anterior.\n");
                break;
            default:
                printf("❌ Opción inválida.\n");
        }
    } while (opc != 0);
}

// Soft delete (cambia estado a inactivo)
void eliminarProducto(void *dato) {
    Producto *p = (Producto *)dato;
    p->estado = 0;
    printf("🗑️ Producto marcado como inactivo.\n");
    trie_actualizado = 0;
}

void buscarProductoPorNombre(TrieNode *trieProd){
    if (!trie_actualizado) {
        liberarTrie(trieProd);  // limpiar si fue reconstruido antes
        trieProd = crearNodoTrie();
        reconstruirTrieProductos(&colaProd, trieProd);
        trie_actualizado = 1;  // ya está actualizado
    }
    char nombre[CHAR_S] = {0};
    printf("Nombre producto: ");
    fgets(nombre, sizeof(nombre), stdin);
    nombre[strcspn(nombre, "\n")] = 0;
    if (!es_texto_valido(nombre, "Nombre producto"))
        return;

    Producto *p = buscarProductoTrie(trieProd, nombre);
    if (p) {
        printf("✅ Producto encontrado:\n");
        mostrarProducto(p);  // ✅ si ya tienes esta función implementada
    } else {
        printf("❌ Producto no encontrado.\n");
    }
}

void ordenarProductosPorCategoria(Cola *colaProd, Cola *colaCat) {
    int max_categorias = (colaCat->tam) + 1;

    Nodo **buckets = (Nodo **)calloc(max_categorias, sizeof(Nodo *));
    Nodo **tails = (Nodo **)calloc(max_categorias, sizeof(Nodo *));
    Categoria **categorias = (Categoria **)calloc(max_categorias, sizeof(Categoria *));

    // Mapear las categorías activas
    Nodo *n = colaCat->frente;
    while (n) {
        Categoria *c = (Categoria *)n->dato;
        if (c->estado && c->id >= 0 && c->id < max_categorias)
            categorias[c->id] = c;
        n = n->siguiente;
    }

    // Insertar productos ordenados alfabéticamente en sus buckets
    Nodo *temp = colaProd->frente;
    while (temp) {
        Producto *p = (Producto *)temp->dato;
        if (p->estado && p->id_categoria >= 0 && p->id_categoria < max_categorias) {
            Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
            nuevo->dato = p;
            nuevo->siguiente = NULL;

            Nodo **bucket = &buckets[p->id_categoria];
            if (!*bucket) {
                *bucket = nuevo;
            } else {
                // Inserción ordenada alfabéticamente por nombre
                Nodo *actual = *bucket, *prev = NULL;
                while (actual && strcmp(((Producto *)actual->dato)->nombre, p->nombre) < 0) {
                    prev = actual;
                    actual = actual->siguiente;
                }
                if (!prev) {
                    nuevo->siguiente = *bucket;
                    *bucket = nuevo;
                } else {
                    nuevo->siguiente = prev->siguiente;
                    prev->siguiente = nuevo;
                }
            }
        }
        temp = temp->siguiente;
    }

    // Imprimir productos por categoría
    for (int i = 0; i < max_categorias; i++) {
        if (buckets[i]) {
            Categoria *c = categorias[i];
            if (c)
                printf("\n---- %s ----\n", c->nombre);
            else
                printf("\n---- Categoría #%d ----\n", i);

            Nodo *actual = buckets[i];
            while (actual) {
                Producto *p = (Producto *)actual->dato;
                printf("\tID: %d | Nombre: %s | Precio: %.2f | Stock: %d\n",
                       p->id, p->nombre, p->valor, p->stock);
                Nodo *lib = actual;
                actual = actual->siguiente;
                free(lib);  // liberar nodo temporal
            }
        }
    }

    free(buckets);
    free(tails);
    free(categorias);
}

void reconstruirReferenciasProducto(Cola *productos, Cola *categorias) {
    if(productos->tam > 0){    
        Nodo *temp = productos->frente;
        while (temp != NULL) {
            Producto *p = (Producto *)temp->dato;
            p->R_categoria = buscarCategoria(categorias, p->id_categoria);
            temp = temp->siguiente;
        }
    }
    else
        puts("No hay productos");
}

void limpiarReferenciasProducto(Cola *productos) {
    Nodo *temp = productos->frente;
    while (temp != NULL) {
        Producto *p = (Producto *)temp->dato;
        p->R_categoria = NULL;
        temp = temp->siguiente;
    }
}

// --------------------------- FUNCIONES DIRECCION ---------------------------
Direccion *crearDireccion(Cola *cola, Localidad *R_localidad, int id_cliente, char calle[], char numero[], char colonia[], char referencias[]) {
    Direccion *nuevo = (Direccion *)malloc(sizeof(Direccion));
    cola->contador_ids++;
    nuevo->id = cola->contador_ids;
    nuevo->R_localidad = R_localidad;
    nuevo->id_localidad = R_localidad->id;
    nuevo->id_cliente = id_cliente;
    nuevo->activo = 1;
    nuevo->historial_pedidos = NULL;

    strcpy(nuevo->calle, calle);
    strcpy(nuevo->numero, numero);
    strcpy(nuevo->colonia, colonia);
    strcpy(nuevo->referencias, referencias);

    return nuevo;
}

Cliente_Direccion* crearCliente_Direccion(Cola *cola, Direccion *direccion, int id_cliente) {
    Cliente_Direccion *nuevo = (Cliente_Direccion *)malloc(sizeof(Cliente_Direccion));
    cola->contador_ids++;
    nuevo->id = direccion->id;
    nuevo->id_cliente = id_cliente;
    nuevo->direccion = direccion;
    return nuevo;
}

int criterioDireccionPorID(void *dato, int id) {
    Direccion *d = (Direccion *)dato;
    return d->id == id;
}

int criterioClienteDireccionPorID(void *dato, int id) {
    Cliente_Direccion *d = (Cliente_Direccion *)dato;
    return d->id == id && d->direccion->activo;
}

void mostrarDireccion(void *dato) {
    Direccion *d = (Direccion *)dato;
    if (d->activo) {
        printf("\tID Dir: %d | Calle: %s | Número: %s | Colonia: %s | Ref: %s | Cliente ID: %d | Localidad: %s\n",
               d->id, d->calle, d->numero, d->colonia, d->referencias, d->id_cliente, d->R_localidad->nombre);
    }
}

void mostrarClienteDireccion(void *dato) {
    Cliente_Direccion *d = (Cliente_Direccion *)dato;
    if (d->direccion->activo) {
        printf("\tID Dir: %d | Calle: %s | Número: %s | Colonia: %s | Ref: %s | Localidad: %s\n",
               d->id, d->direccion->calle,  d->direccion->numero,  d->direccion->colonia,  d->direccion->referencias, d->direccion->R_localidad->nombre);
    }
}

Direccion *buscarDireccion(Cola *colaDirecc, int idDirecc) {
    Nodo *tempL = colaDirecc->frente;
    while (tempL != NULL) {
        Direccion *l = (Direccion *)tempL->dato;
        if (l->id == idDirecc) {
            return l;
        }
        tempL = tempL->siguiente;
    }
    return NULL;
}


Cliente_Direccion *buscarClienteDireccion(Cola *colaClienteDir, int idDirecc){
    Nodo *temp = colaClienteDir->frente;
    while (temp != NULL) {
        Cliente_Direccion *c = (Cliente_Direccion *)temp->dato;
        if (c->id == idDirecc) 
            return c;
        temp = temp->siguiente;
    }
    return NULL;
}

// Función para editar direccion con submenú de campos
void editarClienteDireccion(void *dato) {
    Cliente_Direccion *cd = (Cliente_Direccion *)dato;
    Direccion *d = cd->direccion;
    const char *campos[] = {"localidad", "calle", "número", "colonia", "referencias", "cliente"};
    int opc = 0;

    do {
        opc = menu_edicion("Dirección", campos, 6);
        if (opc == -1) continue;
        switch (opc) {
            case 1:{
                printf("Nueva localidad: ");
                listarCola(&colaLoc, mostrarLocalidad);
                int idLoc = 0;
                printf("Ingrese ID de la localidad:");
                if (!leer_int_seguro(&idLoc)) 
                    break;
                Localidad *loc = buscarLocalidad(&colaLoc,idLoc);
                if(loc){
                    d->R_localidad = loc;
                    d->id_localidad = loc->id;
                }
                else{
                    printf("❌ Opción inválida.\n");
                }
                break;
            }
            case 2:{
                printf("Nueva calle: ");
                fgets(d->calle, sizeof(d->calle), stdin);
                d->calle[strcspn(d->calle, "\n")] = 0;
                break;
            }
            case 3:{
                printf("Nuevo número: ");
                fgets(d->numero, sizeof(d->numero), stdin);
                d->numero[strcspn(d->numero, "\n")] = 0;
                break;
            }
            case 4:{
                printf("Nueva colonia: ");
                fgets(d->colonia, sizeof(d->colonia), stdin);
                d->colonia[strcspn(d->colonia, "\n")] = 0;
                break;
            }
            case 5:{
                printf("Nuevas referencias: ");
                fgets(d->referencias, sizeof(d->referencias), stdin);
                d->referencias[strcspn(d->referencias, "\n")] = 0;
                break;
            }
            case 6:{
                printf("Nuevo cliente: ");
                listarCola(&colaCliente,mostrarCliente);
                int idCli = 0;
                printf("Ingrese ID del cliente:");
                if (!leer_int_seguro(&idCli)) 
                    break;
                Cliente *c = buscarCliente(&colaCliente,idCli);
                if(c){
                    d->id_cliente = c->id;
                }
                else{
                    printf("❌ Opción inválida.\n");
                }
                puts("❕Se actualizará al volver a cargar el programa");
                break;
            }
            case 0:
                puts("↩ Volviendo al menú anterior.\n");
                break;
            default:
                puts("❌ Opción inválida.\n");
        }
    } while (opc != 0);
}
// Soft delete (cambia estado a inactivo)
void eliminarDireccion(void *dato) {
    Direccion *d = (Direccion *)dato;
    d->activo = 0;
    printf("🗑️ Dirección marcada como inactiva.\n");
}

void limpiarReferenciasDireccion(Cola *colaDirecc) {
    Nodo *temp = colaDirecc->frente;
    while (temp != NULL) {
        Direccion *p = (Direccion *)temp->dato;
        p->R_localidad = NULL;
        temp = temp->siguiente;
    }
}

//tarjeta sd para almacenar datos estando en zonas de baja cobertura

// --------------------------- FUNCIONES CLIENTE ---------------------------
Cliente* crearCliente(Cola *cola, char nombre[]) {
    Cliente *nuevo = (Cliente *)malloc(sizeof(Cliente));
    cola->contador_ids++;
    nuevo->id = cola->contador_ids;
    strcpy(nuevo->nombre, nombre);
    inicializarCola(&nuevo->direcciones);
    nuevo->num_pedidos = 0;
    return nuevo;
}

int criterioClientePorID(void *dato, int id) {
    Cliente *c = (Cliente *)dato;
    return c->id == id;
}

void mostrarCliente(void *dato) {
    Cliente *c = (Cliente *)dato;
    printf("ID Cliente: %d | Nombre: %s\n", c->id, c->nombre);

    if (c->direcciones.tam > 0){
        listarCola(&c->direcciones,mostrarClienteDireccion);
    }
    return;
    
}
    
void editarCliente(void *dato) {
    Cliente *c = (Cliente *)dato;
    int opc = 0;
    const char *campos[] = {"nombre", "direccion"};
    do {
        opc = menu_edicion(c->nombre, campos, 2);
        switch (opc) {
            case 1:
                printf("Nuevo nombre: ");
                fgets(c->nombre, sizeof(c->nombre), stdin);
                c->nombre[strcspn(c->nombre, "\n")] = 0;
                printf("✅ Nombre actualizado.\n");
                break;
            case 2:
                printf("--DIRECCION--");
                listarCola(&c->direcciones,mostrarClienteDireccion);
                editarCola(&c->direcciones,criterioClienteDireccionPorID,editarClienteDireccion);
                break;
            case 0:
                printf("↩ Volviendo al menú anterior.\n");
                break;
            default:
                printf("❌ Opción inválida.\n");
        }
    } while (opc != 0);
}

Cliente *buscarCliente(Cola *cola, int id_buscar){
    Nodo *temp = cola->frente;
    Cliente *cliente_encontrado = NULL;
    while (temp != NULL) {
        Cliente *c = (Cliente *)temp->dato;
        if (c->id == id_buscar) {
            cliente_encontrado = c;
            break;
        }
        temp = temp->siguiente;
    }
    return cliente_encontrado;
}

void agregarDireccionesACliente(Cliente *cliente, Cola *colaLoc, Cola *colaDirecc) {
    int num = 0;
    printf("¿Cuántas direcciones desea ingresar?: ");
    if (!leer_int_seguro(&num) || num <= 0) {
        printf("❌ Número inválido.\n");
        return;
    }

    if (cliente->direcciones.tam==0) {
        inicializarCola(&cliente->direcciones);
        puts("✅ Cola de direcciones inicializada");
    }

    
    int aux = 0;
    while(aux!=num) {
        char calle[CHAR_M] = {0};
        char numero[CHAR_S] = {0}; 
        char colonia[CHAR_M] = {0}; 
        char referencias[CHAR_G] = {0}; 
        Localidad *loc = NULL;

        listarCola(colaLoc, mostrarLocalidad);
        int idLoc = 0;
        printf("Ingrese ID de la localidad:");
        if (!leer_int_seguro(&idLoc)) 
            continue;
        loc = buscarLocalidad(colaLoc,idLoc);
        if(!loc){
            printf("❌ Opción inválida.\n");
            continue;
        }
        
        
        printf("Calle: ");
        fgets(calle, sizeof(calle), stdin);
        calle[strcspn(calle, "\n")] = 0;
        if (!es_texto_valido(calle, "Calle"))
            continue;

        printf("Número (I:1340|E:B-20|): ");
        fgets(numero, sizeof(numero), stdin);
        numero[strcspn(numero, "\n")] = 0;
        if (!es_texto_valido(numero, "Numero"))
            continue;

        printf("Colonia: ");
        fgets(colonia, sizeof(colonia), stdin);
        colonia[strcspn(colonia, "\n")] = 0;
        if (!es_texto_valido(colonia, "Colonia"))
            continue;

        printf("Referencias (opcional): ");
        fgets(referencias, sizeof(referencias), stdin);
        referencias[strcspn(referencias, "\n")] = 0;

        Nodo* dir = encolar(colaDirecc, crearDireccion(colaDirecc,loc,cliente->id,calle,numero,colonia,referencias),sizeof(Direccion));
        if(!dir){
            printf("❌ ¡Error! No se pudo insertar la direccion.\n");
        }
        else{
            Direccion *d = (Direccion *)dir->dato;
            if(encolar(&cliente->direcciones,crearCliente_Direccion(&cliente->direcciones,d,cliente->id),sizeof(Cliente_Direccion))){
                printf("✅ Se agregó: ");
                mostrarDireccion(d);
                puts("");
            }
            else
                printf("❌ ¡Error! No se pudo insertar la direccion.\n");
        }
        aux ++;
    }

    printf("✅ Direcciones agregadas.\n");
}

void limpiarReferenciasClienteDireccion(Cola *colaCliente, Cola *colaDirecc) {
    Nodo *nodoCliente = colaCliente->frente;

    while (nodoCliente != NULL) {
        Cliente *cliente = (Cliente *)nodoCliente->dato;

        // Liberar cada Cliente_Direccion
        Nodo *nodoDir = cliente->direcciones.frente;
        while (nodoDir != NULL) {
            Nodo *temp = nodoDir;
            nodoDir = nodoDir->siguiente;

            if (temp->dato != NULL) {
                Cliente_Direccion *cd = (Cliente_Direccion *)temp->dato;
                cd->direccion = NULL;
                free(cd);
            }
            free(temp);
        }

        // Resetear la cola interna
        cliente->direcciones.frente = NULL;
        cliente->direcciones.final = NULL;
        cliente->direcciones.tam = 0;
        cliente->direcciones.contador_ids = 0;

        nodoCliente = nodoCliente->siguiente;
    }

    // Limpiar referencias de R_localidad en cada dirección
    Nodo *nodoGlobalDir = colaDirecc->frente;
    while (nodoGlobalDir != NULL) {
        Direccion *d = (Direccion *)nodoGlobalDir->dato;
        if (d) {
            d->R_localidad = NULL;
        }
        nodoGlobalDir = nodoGlobalDir->siguiente;
    }
}

// --------------------------- FUNCIONES CAMION ---------------------------
Camion* crearCamion(Cola *cola, Medida capacidad_peso, Medida capacidad_volumen) {
    Camion *nuevo = (Camion *)malloc(sizeof(Camion));
    cola->contador_ids++;
    nuevo->id = cola->contador_ids;
    nuevo->capacidad_peso = capacidad_peso;
    nuevo->capacidad_volumen = capacidad_volumen;
    nuevo->estado = 0;
    return nuevo;
}

int criterioCamionPorID(void *dato, int id) {
    Camion *c = (Camion *)dato;
    return c->id == id;
}

void mostrarCamion(void *dato) {
    Camion *c = (Camion *)dato;
    printf("ID: %d | Estado: %s", c->id,obtener_nombre_estado_camion(c->estado));
    printf(" | Capacidad Peso: %.2f %s | Volumen: %.2f %s\n",
        c->capacidad_peso.valor, obtener_nombre_unidad(c->capacidad_peso.unidad),
        c->capacidad_volumen.valor, obtener_nombre_unidad(c->capacidad_volumen.unidad));
}

void editarCamion(void *dato) {
    Camion *c = (Camion *)dato;
    int opcion = 0;
    const char *campos[] = {"capacidad peso", "capacidad volumen", "estado"};

    do {
        opcion = menu_edicion("Camión", campos, 3);
        if (opcion == -1) continue;

        switch (opcion) {
            case 1:
                puts("--Nueva Capacidad de Peso--");
                c->capacidad_peso = pedir_medida((Unidad[]){KG, G}, 2, "Capacidad de Peso");
                break;
            case 2:
                puts("--Nueva Capacidad de Volumen--");
                c->capacidad_volumen = pedir_medida((Unidad[]){M3, CM3}, 2, "Capacidad de Volumen");
                break;
            case 3:
                printf("Seleccione nuevo estado:\n 0. Disponible\n 1. En Ruta\n 2. Mantenimiento\n Opción: ");
                int est = 0;
                if (leer_int_seguro(&est) && est >= 0 && est <= 2) {
                    c->estado = (EstadoCamion)est;
                } else {
                    printf("❌ Estado inválido.\n");
                }
                break;
            case 0:
                printf("↩ Volviendo al menú anterior.\n");
                break;
            default:
                printf("❌ Opción inválida.\n");
        }
    } while (opcion != 0);
}

// --------------------------- FUNCIONES CARRETERA ---------------------------

Carretera* crearCarretera(Cola *cola, Localidad* R_origen, Localidad* R_destino, float distancia, float tiempo, float penalizacion) {
    Carretera *nuevo = (Carretera *)malloc(sizeof(Carretera));
    nuevo->id = cola->contador_ids++;
    if (R_origen == NULL){
        nuevo->id_origen = 0;
        nuevo->R_origen = NULL;
    }
    else{
        nuevo->id_origen = R_origen->id;
        nuevo->R_origen = R_origen;
    }

        
    if (R_destino == NULL){
        nuevo->id_destino = 0;
        nuevo->R_destino = NULL;
    }
    else{
        nuevo->id_destino = R_destino->id;
        nuevo->R_destino = R_destino;
    }

        
    nuevo->distancia = distancia;
    nuevo->tiempo = tiempo;
    nuevo->penalizacion = penalizacion;
    return nuevo;
}

void mostrarCarretera(void *dato) {
    Carretera *c = (Carretera *)dato;
    printf("ID: %d | Origen: %d | Destino: %d | Distancia: %.2f KM | Tiempo: %.2f h | Penalización: %.2f\n",
           c->id, c->id_origen, c->id_destino, c->distancia, c->tiempo, c->penalizacion);
}

int criterioCarreteraPorID(void *dato, int id) {
    Carretera *c = (Carretera *)dato;
    return c->id == id;
}

void limpiarReferenciasCarreteras(Cola *colaCarretera) {
    Nodo *nodo = colaCarretera->frente;
    while (nodo) {
        Carretera *c = (Carretera *)nodo->dato;
        c->R_origen = NULL;
        c->R_destino = NULL;
        nodo = nodo->siguiente;
    }
    printf("🧹 Referencias de carreteras limpiadas.\n");
}

// --------------------------- FUNCIONES TRIE ---------------------------
TrieNode *crearNodoTrie() {
    TrieNode *nodo = (TrieNode *)calloc(1, sizeof(TrieNode));
    return nodo;
}

void insertarProductoTrie(TrieNode *raiz, const char *nombre, Producto *producto) {
    TrieNode *actual = raiz;
    for (int i = 0; nombre[i]; i++) {
        unsigned char c = nombre[i];
        if (!actual->hijos[c])
            actual->hijos[c] = crearNodoTrie();
        actual = actual->hijos[c];
    }
    actual->producto = producto;
}

void reconstruirTrieProductos(Cola *colaProd, TrieNode *raiz) {
    Nodo *temp = colaProd->frente;
    while (temp) {
        Producto *p = (Producto *)temp->dato;
        insertarProductoTrie(raiz, p->nombre, p);
        temp = temp->siguiente;
    }
}

Producto *buscarProductoTrie(TrieNode *raiz, const char *nombre) {
    TrieNode *actual = raiz;
    for (int i = 0; nombre[i]; i++) {
        unsigned char c = nombre[i];
        if (!actual->hijos[c]) return NULL;
        actual = actual->hijos[c];
    }
    return actual->producto;
}

void liberarTrie(TrieNode *nodo) {
    if (!nodo) return;
    for (int i = 0; i < 128; i++) {
        if (nodo->hijos[i]) {
            liberarTrie(nodo->hijos[i]);
        }
    }
    free(nodo);
}

// --------------------------- FUNCIONES PEDIDO ---------------------------
float convertirAPesoKG(Medida peso) {
    if (peso.unidad == G) {
        return peso.valor / 1000.0;  // gramos a kilogramos
    } else if (peso.unidad == KG) {
        return peso.valor;
    }
    return 0;
}

float convertirAVolumenM3(Medida volumen) {
    if (volumen.unidad == CM3) {
        return volumen.valor / 1000000.0;  // cm³ a m³
    } else if (volumen.unidad == M3) {
        return volumen.valor;
    }
    return 0;
}

void realizarPedido(Cola *colaCliente, Cola *colaProd, Cola *colaPedidos) {
    listarCola(colaCliente, mostrarCliente);
    int idCli = 0;
    printf("Ingrese ID del cliente: ");
    if (!leer_int_seguro(&idCli)) return;

    Cliente *c = buscarCliente(colaCliente, idCli);
    if (!c) {
        puts("❌ Cliente no encontrado.");
        return;
    }

    listarCola(&c->direcciones, mostrarClienteDireccion);
    int idDir = 0;
    printf("Seleccione ID de dirección para el pedido: ");
    if (!leer_int_seguro(&idDir)) return;

    Cliente_Direccion *cliente_dir = buscarClienteDireccion(&c->direcciones, idDir);
    if (!cliente_dir) {
        puts("❌ Dirección no encontrada.");
        return;
    }
    Direccion *dir = cliente_dir->direccion;

    CargaPedido nuevo;
    colaPedidos->contador_ids++;
    nuevo.id_pedido = colaPedidos->contador_ids;
    nuevo.id_direccion = dir->id;
    nuevo.R_direccion = dir;
    nuevo.id_camion = 0;
    nuevo.estado = EN_ESPERA;
    nuevo.total_cantidad = 0;
    nuevo.total_peso = 0;
    nuevo.total_vol = 0;
    nuevo.total_valor = 0;
    nuevo.tam = 0;

    int continuar = 1, pedidos = 0;

    if(!colaProd->frente){
        puts("❌ No hay productos disponibles."); 
        return;
    }

    while (continuar && pedidos < NUM_MAX_PEDIDOS) {
        listarCola(colaProd, mostrarProducto);
        printf("ID producto: ");
        if (!leer_int_seguro(&nuevo.pedidos[pedidos].id_producto)) break;

        Producto *p = buscarProducto(colaProd, nuevo.pedidos[pedidos].id_producto);
        if (!p || p->stock <= 0) {
            puts("❌ Producto no disponible.");
            continue;
        }

        printf("Cantidad a pedir (máx %d): ", p->stock);
        if (!leer_int_seguro(&nuevo.pedidos[pedidos].cantidad)) continue;
        if (!esta_en_rango(nuevo.pedidos[pedidos].cantidad,1,p->stock,"Cantidad")){
            puts("❌ No puede ni pedir más del stock disponible ni pedir 0.");
            continue;
        }

        // Referencias y acumuladores
        nuevo.pedidos[pedidos].R_producto = p;
        nuevo.total_cantidad += nuevo.pedidos[pedidos].cantidad;
        nuevo.total_valor += nuevo.pedidos[pedidos].cantidad * p->valor;
        nuevo.total_peso += nuevo.pedidos[pedidos].cantidad * convertirAPesoKG(p->peso);
        nuevo.total_vol  += nuevo.pedidos[pedidos].cantidad * convertirAVolumenM3(p->volumen);
        pedidos++;
        nuevo.tam ++;

        puts("¿Agregar otro producto? (1=Sí / 0=No): ");
        leer_int_seguro(&continuar);
    }

    encolar(colaPedidos, &nuevo, sizeof(CargaPedido));

    puts("✅ Pedido realizado y almacenado.");
}

void limpiarReferenciasPedidos(Cola *cola) {
    Nodo *temp = cola->frente;
    while (temp) {
        CargaPedido *p = (CargaPedido *)temp->dato;
        p->R_direccion = NULL;
        for (int i = 0; i < NUM_MAX_PEDIDOS; i++) {
            p->pedidos[i].R_producto = NULL;
        }
        temp = temp->siguiente;
    }
}

void reconstruirReferenciasPedidos(Cola *colaPedidos, Cola *colaDirecc, Cola *colaProd) {
    Nodo *temp = colaPedidos->frente;
    while (temp) {
        CargaPedido *pedido = (CargaPedido *)temp->dato;
        pedido->R_direccion = buscarDireccion(colaDirecc, pedido->id_direccion);
        for (int i = 0; i < NUM_MAX_PEDIDOS; i++) {
            if (pedido->pedidos[i].id_producto != 0)
                pedido->pedidos[i].R_producto = buscarProducto(colaProd, pedido->pedidos[i].id_producto);
        }
        temp = temp->siguiente;
    }
}

void registrarHistorialEntrega(CargaPedido *pedido) {
    if (!pedido || !pedido->R_direccion) return;

    int idCli = pedido->R_direccion->id_cliente;
    char archivo[64];
    sprintf(archivo, "historial_cliente_%d.txt", idCli);

    FILE *hist = fopen(archivo, "a");
    if (!hist) {
        printf("⚠ No se pudo abrir el historial del cliente %d\n", idCli);
        return;
    }

    fprintf(hist, "✅ Pedido COMPLETADO - ID: %d, Dirección: %d, Total: $%.2f\n",
            pedido->id_pedido, pedido->id_direccion, pedido->total_valor);

    for (int i = 0; i < NUM_MAX_PEDIDOS; i++) {
        Pedido p = pedido->pedidos[i];
        if (p.id_producto > 0 && p.cantidad > 0)
            fprintf(hist, "\tProducto ID: %d, Cantidad: %d\n", p.id_producto, p.cantidad);
    }

    fclose(hist);
}

void mostrarCargaPedido(void *dato) {
    CargaPedido *p = (CargaPedido *)dato;
    printf("\n📦 Pedido ID: %d | Dirección ID: %d | Productos: %d | Valor Total: $%.2f | Peso: %.2f kg | Volumen: %.2f m^3 | Estado: %s\n",
        p->id_pedido, p->id_direccion, p->total_cantidad, p->total_valor,
        p->total_peso, p->total_vol,
        obtener_nombre_estado_pedido(p->estado));

    for (int i = 0; i < p->tam; i++) {
        Pedido *item = &p->pedidos[i];
        printf("\t🛒 Producto ID: %d | Cantidad: %d\n", item->id_producto, item->cantidad);
    }
}


// --------------------------- FUNCIONES COLAS ---------------------------
void inicializarCola(Cola *cola) {
    cola->frente = NULL;
    cola->final = NULL;
    cola->tam = 0;
    cola->contador_ids = 0;
}

Nodo *encolar(Cola *cola, void *dato, size_t tam_dato) {
    if (!dato){
        return NULL;
    }
    Nodo *nuevo = (Nodo *)malloc(sizeof(Nodo));
    nuevo->dato = malloc(tam_dato);
    memcpy(nuevo->dato, dato, tam_dato);
    nuevo->siguiente = NULL;

    if (cola->final == NULL)
        cola->frente = cola->final = nuevo;
    else {
        cola->final->siguiente = nuevo;
        cola->final = nuevo;
    }

    cola->tam++;
    return nuevo;  // ✅ retornamos el puntero al nodo insertado
}

void *desencolar(Cola *cola) {
    if (cola->frente == NULL) {
        printf("⚠ La cola está vacía.\n");
        return NULL;
    }

    Nodo *temp = cola->frente;
    void *dato = temp->dato;

    cola->frente = cola->frente->siguiente;
    if (cola->frente == NULL)
        cola->final = NULL;

    free(temp);
    cola->tam--;

    return dato;  // El dato debe ser procesado antes de ser liberado externamente
}

void listarCola(Cola *cola, void (*mostrar)(void *)) {
    Nodo *temp = cola->frente;
    if (temp == NULL) {
        printf("📋 La cola está vacía.\n");
        return;
    }

    while (temp != NULL) {
        mostrar(temp->dato);
        temp = temp->siguiente;
    }
}

void editarCola(Cola *cola, int (*criterio)(void *, int), void (*editar)(void *)) {
    int id = 0;
    printf("Ingrese el ID a buscar: ");
    if (!leer_int_seguro(&id)) {
        printf("❌ Entrada inválida.\n");
        return;
    }

    Nodo *temp = cola->frente;
    while (temp != NULL) {
        if (criterio(temp->dato,id)) {
            editar(temp->dato);
            printf("✅ Elemento editado.\n");
            return;
        }
        temp = temp->siguiente;
    }
    printf("❌ Elemento no encontrado.\n");
}

void cargarCola(Cola *cola, const char *nombre_archivo, size_t tam_dato) {
    inicializarCola(cola);

    FILE *archivo = fopen(nombre_archivo, "rb");
    if (!archivo) {
        printf("⚠ No se encontró archivo '%s'.\n", nombre_archivo);
        return;
    }

    ColaHeader header;
    fread(&header, sizeof(ColaHeader), 1, archivo);  // ✅ lee encabezado
    cola->contador_ids = header.contador_ids;

    void *buffer = malloc(tam_dato);
    for (int i = 0; i < header.tam; i++) {
        if (fread(buffer, tam_dato, 1, archivo) == 1) {
            encolar(cola, buffer, tam_dato);
        }
    }

    free(buffer);
    fclose(archivo);
    printf("📂 Cola cargada desde '%s'.\n", nombre_archivo);
}

void guardarCola(Cola *cola, const char *nombre_archivo, size_t tam_dato) {
    FILE *archivo = fopen(nombre_archivo, "wb");
    if (!archivo) {
        printf("❌ Error al abrir archivo '%s'.\n", nombre_archivo);
        return;
    }

    ColaHeader header = {cola->contador_ids, cola->tam};
    fwrite(&header, sizeof(ColaHeader), 1, archivo);  // ✅ guarda encabezado

    Nodo *temp = cola->frente;
    while (temp != NULL) {
        fwrite(temp->dato, tam_dato, 1, archivo);
        temp = temp->siguiente;
    }

    fclose(archivo);
    printf("💾 Cola guardada en '%s'.\n", nombre_archivo);
}

void liberarCola(Cola *cola) {
    Nodo *actual = cola->frente;
    while (actual != NULL) {
        Nodo *temp = actual;
        actual = actual->siguiente;
        free(temp->dato);
        free(temp);
    }
    cola->frente = NULL;
    cola->final = NULL;
    cola->tam = 0;
}

Nodo *extraerNodoPorCriterio(Cola *cola, int (*criterio)(void *, int)) {
    Nodo *actual = cola->frente;
    Nodo *anterior = NULL;

    int id = 0;
    printf("Ingrese el ID a buscar: ");
    if (!leer_int_seguro(&id)) {
        printf("❌ Entrada inválida.\n");
        return NULL;
    }

    while (actual != NULL) {
        if (criterio(actual->dato, id)) {
            // Reorganizar punteros
            if (anterior == NULL) {
                // Es el primer nodo
                cola->frente = actual->siguiente;
                if (cola->frente == NULL)
                    cola->final = NULL;
            } else {
                anterior->siguiente = actual->siguiente;
                if (actual == cola->final)
                    cola->final = anterior;
            }

            cola->tam--;
            return actual;  // ✅ retornar el nodo extraído
        }

        anterior = actual;
        actual = actual->siguiente;
    }

    return NULL;  // ❌ No encontrado
}

// --------------------------- MENUS SECUNDARIOS ---------------------------

int menu_edicion(const char *titulo, const char *campos[], int num_campos) {
    int opcion = 0;
    printf("\n-- Editando %s --\n", titulo);
    for (int i = 0; i < num_campos; i++) {
        printf("%d. Editar %s\n", i + 1, campos[i]);
    }
    printf("0. Volver\n");
    printf("Seleccione una opción: ");

    if (!leer_int_seguro(&opcion)) {
        printf("❌ Entrada inválida.\n");
        return -1;
    }

    if (opcion < 0 || opcion > num_campos) {
        printf("❌ Opción inválida.\n");
        return -1;
    }

    return opcion;
}

const char* obtener_nombre_unidad(Unidad u) {
    switch (u) {
        case KG: return "KG";
        case G: return "G";
        case M3: return "M3";
        case CM3: return "CM3";
        case KM: return "KM";
        case H: return "Horas";
        case MIN: return "Minutos";
        default: return "Desconocido";
    }
}

const char* obtener_nombre_estado_camion(EstadoCamion e) {
    switch (e) {
        case DISPONIBLE: return "Disponible";
        case EN_RUTA: return "Ocupado";
        case MANTENIMIENTO: return "Fuera de Serivicio";
        default: return "Desconocido";
    }
}

const char* obtener_nombre_estado_pedido(EstadoPedidos e) {
    switch (e) {
        case EN_ESPERA: return "En espera";
        case EN_CAMINO: return "El pedido está en camino";
        case COMPLETADO: return "Entragado";
        default: return "Desconocido";
    }
}


// --------------------------- SUBMENUS ---------------------------
void submenu_categoria(Cola *colaCat) {
    int op = 0;
    do {
        printf("\n-- SUBMENÚ CATEGORÍA --\n");
        printf("1. Insertar\n2. Editar\n3. Listar\n0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: {
                char nombre[CHAR_S] = {0};
                printf("Nombre categoría: ");
                fgets(nombre, sizeof(nombre), stdin);
                nombre[strcspn(nombre, "\n")] = 0;
                if (es_texto_valido(nombre, "Nombre Categoría"))
                    if(!encolar(colaCat, crearCategoria(colaCat,nombre),sizeof(Categoria)))
                         printf("❌ ¡Error! No se pudo insertar la direccion.\n");
            } break;
            case 2: editarCola(colaCat,criterioCategoriaPorID,editarCategoriaNombre); break;
            case 3: listarCola(colaCat,mostrarCategoria); break;
            case 0: printf("↩ Volviendo al menú anterior.\n"); break;
            default: printf("❌ Opción inválida.\n");
        }
    } while (op != 0);
}

void submenu_localidad(Cola*colaLoc) {
    int op = 0;
    do {
        printf("\n-- SUBMENÚ LOCALIDAD --\n");
        printf("1. Insertar\n2. Editar\n3. Listar\n0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: {
                char nombre[CHAR_S];
                printf("Nombre localidad: ");
                fgets(nombre, sizeof(nombre), stdin);
                nombre[strcspn(nombre, "\n")] = 0;
                if (es_texto_valido(nombre, "Nombre Localidad")) {
                    Coordenadas coor = pedir_coordenadas();
                    if (!encolar(colaLoc, crearLocalidad(colaLoc,nombre,coor),sizeof(Localidad)))
                        printf("❌ ¡Error! No se pudo insertar.\n");
                }
            } break;
            case 2: editarCola(colaLoc,criterioLocalidadPorID,editarLocalidad); break;
            case 3: listarCola(colaLoc,mostrarLocalidad); break;
            case 0: printf("↩ Volviendo al menú anterior.\n"); break;
            default: printf("❌ Opción inválida.\n");
        }
    } while (op != 0);
}

void submenu_producto(Cola *colaProd, Cola *colaCat) {
    int op = 0;
    do {
        printf("\n-- SUBMENÚ PRODUCTO --\n");
        printf("1. Insertar\n2. Editar\n3. Eliminar (marcar inactivo)\n4. Listar\n0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: {
                char nombre[CHAR_S] = {0};
                printf("Nombre producto: ");
                fgets(nombre, sizeof(nombre), stdin);
                nombre[strcspn(nombre, "\n")] = 0;
                if (!es_texto_valido(nombre, "Nombre Producto")) break;

                printf("Ingrese valor económico: ");
                float valor = 0;
                if (!leer_float_seguro(&valor)) {
                    printf("❌ Entrada inválida.\n");
                    break;
                }

                puts("--Peso--");
                Medida peso = pedir_medida((Unidad[]){KG, G}, 2, "Peso");

                puts("--Volumen--");
                Medida volumen = pedir_medida((Unidad[]){M3, CM3}, 2, "Volumen");

                printf("Ingrese stock inicial: ");
                int stock = 0;
                if (!leer_int_seguro(&stock)) {
                    printf("❌ Entrada inválida.\n");
                    break;
                }
                puts("----------");
                    listarCola(colaCat, mostrarCategoria);
                printf("Seleccione una categoría existente: ");

                int idCat = 0;
                if (!leer_int_seguro(&idCat)) {
                    printf("❌ Entrada inválida.\n");
                    break;
                }
                Categoria *categoria_seleccionada = buscarCategoria(colaCat,idCat);

                if (!categoria_seleccionada) {
                    printf("❌ Categoría no encontrada.\n");
                    break;
                }
                if(!encolar(colaProd, crearProducto(colaProd, nombre, categoria_seleccionada, valor, peso, volumen, stock), sizeof(Producto)))
                    printf("❌ ¡Error! No se pudo insertar el Producto.\n");
                trie_actualizado = 0;
            } break;
            case 2: editarCola(colaProd, criterioProductoPorID, editarProducto); break;
            case 3: editarCola(colaProd, criterioProductoPorID, eliminarProducto); break;
            case 4: listarCola(colaProd, mostrarProducto); break;
            case 0: printf("↩ Volviendo al menú anterior.\n"); break;
            default: printf("❌ Opción inválida.\n");
        }
    } while (op != 0);
}

void submenu_cliente(Cola *colaCliente, Cola *colaLoc, Cola *colaDirecc) {
    int op = 0;
    do {
        printf("\n-- SUBMENÚ CLIENTE --\n");
        printf("1. Insertar cliente\n2. Agregar direcciones a cliente\n3. Editar cliente\n");
        printf("4. Eliminar dirección (desactivar)\n5. Eliminar cliente\n");
        printf("6. Listar clientes con direcciones\n7. Listar Direcciones\n0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: {
                char nombre[CHAR_S] = {0};
                printf("Nombre cliente: ");
                fgets(nombre, sizeof(nombre), stdin);
                nombre[strcspn(nombre, "\n")] = 0;
                if (es_texto_valido(nombre, "Nombre Cliente"))
                    if(!encolar(colaCliente, crearCliente(colaCliente, nombre), sizeof(Cliente)))
                         printf("❌ ¡Error! No se pudo insertar la direccion.\n");
            } break;

            case 2: {
                listarCola(colaCliente, mostrarCliente);
                int idCli = 0;
                printf("Ingrese ID del cliente para agregar direcciones: ");
                if (!leer_int_seguro(&idCli)) break;

                Cliente *cliente_encontrado = buscarCliente(colaCliente,idCli);

                if (!cliente_encontrado) {
                    printf("❌ Cliente no encontrado.\n");
                    break;
                }
                agregarDireccionesACliente(cliente_encontrado, colaLoc, colaDirecc);
            } break;

            case 3: {
                listarCola(colaCliente,mostrarCliente);
                editarCola(colaCliente,criterioClientePorID,editarCliente);
            } break;

        case 4: {
                listarCola(colaCliente, mostrarCliente);
                int idCli = 0;
                printf("Ingrese ID del cliente para agregar direcciones: ");
                if (!leer_int_seguro(&idCli)) break;

                Cliente *cliente_encontrado = buscarCliente(colaCliente,idCli);

                if(!cliente_encontrado){
                    printf("❌ Cliente no encontrado.\n");
                    break;
                }

                // 1. Mostrar las direcciones del cliente
                printf("📋 Direcciones del cliente:\n");
                listarCola(&cliente_encontrado->direcciones,mostrarClienteDireccion);
                int idDir = 0;
                printf("Ingrese ID de la direccion: ");
                if (!leer_int_seguro(&idDir)) 
                    break;

                Nodo* nodoExtraido = extraerNodoPorCriterio(&cliente_encontrado->direcciones,criterioClienteDireccionPorID);

                Nodo *nodoDireccionGlobal = *(Nodo **)nodoExtraido->dato;
                Direccion *d = (Direccion *)nodoDireccionGlobal->dato;

                // 3. Desactivar la dirección global

                d->activo = 0;

                // 4. Liberar el nodo de la cola interna
                free(nodoExtraido->dato);  // liberar el puntero a Nodo *
                free(nodoExtraido);
                printf("✅ Dirección desactivada y referencia eliminada del cliente.\n");
        } break;

        case 5: {
            listarCola(colaCliente, mostrarCliente);

            Nodo* nodoExtraido = extraerNodoPorCriterio(colaCliente,criterioClientePorID);

            // 4. Liberar el nodo de la cola interna
            free(nodoExtraido->dato);  // liberar el puntero a Nodo *
            free(nodoExtraido);
            
            printf("✅ Dirección desactivada y referencia eliminada del cliente.\n");
        } break;
        
            case 6: listarCola(colaCliente, mostrarCliente); break;
            
            case 7: listarCola(colaDirecc, mostrarDireccion); break;

            case 0: printf("↩ Volviendo al menú anterior.\n"); break;

            default: printf("❌ Opción inválida.\n");
        }
    } while (op != 0);
}

void submenu_camion(Cola *colaCamion) {
    int op = 0;
    do {
        printf("\n-- SUBMENÚ CAMIÓN --\n");
        printf("1. Insertar\n2. Editar\n3. Listar\n0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: {
                puts("--Capacidad de Peso--");
                Medida capacidad_peso = pedir_medida((Unidad[]){KG, G}, 2, "Capacidad de Peso");

                puts("--Capacidad de Volumen--");
                Medida capacidad_volumen = pedir_medida((Unidad[]){M3, CM3}, 2, "Capacidad de Volumen");

                if(!encolar(colaCamion, crearCamion(colaCamion,capacidad_peso,capacidad_volumen),sizeof(Camion)))
                    printf("❌ ¡Error! No se pudo insertar la direccion.\n");
            } break;
            case 2: editarCola(colaCamion,criterioCamionPorID,editarCamion); break;
            case 3: listarCola(colaCamion,mostrarCamion); break;
            case 4: {
                listarCola(colaCamion, mostrarCamion);

                Nodo* nodoExtraido = extraerNodoPorCriterio(colaCamion,criterioClientePorID);

                // 4. Liberar el nodo de la cola interna
                free(nodoExtraido->dato);  // liberar el puntero a Nodo *
                free(nodoExtraido);

            }break;
            case 0: printf("↩ Volviendo al menú anterior.\n"); break;
            default: printf("❌ Opción inválida.\n");
        }
    } while (op != 0);
}

void submenu_carreteras(Cola *colaCarretera, Cola *colaLoc) {
    int op = 0;
    do {
        printf("\n=== SUBMENÚ CARRETERAS ===\n");
        puts("1. Insertar conexión");
        puts("2. Eliminar conexión");
        puts("3. Listar conexiones");
        printf("0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: {
                listarCola(colaLoc, mostrarLocalidad);
                int idOrg,idDes;
                printf("Ingrese ID de la localidad de origen:");
                if (!leer_int_seguro(&idOrg)) 
                    break;
                printf("Ingrese ID de la localidad de destino:");
                if (!leer_int_seguro(&idDes)) 
                    break;
                Localidad *R_origen = buscarLocalidad(colaLoc,idOrg);
                Localidad *R_destino = buscarLocalidad(colaLoc,idDes);

                if (!R_origen || !R_destino) {
                    printf("❌ No se encontraron ambas localidades.\n");
                    break;
                }
                float distancia=0, tiempo=0, penalizacion=0;
                printf("Distancia estimada (en kilómetros): ");
                if (!leer_float_seguro(&tiempo)) break;
                printf("Tiempo estimado (en horas): ");
                if (!leer_float_seguro(&tiempo)) break;
                printf("Penalización (por tráfico, ej. 1.0 - 2.0): ");
                if (!leer_float_seguro(&penalizacion)) break;

                Carretera *nueva = crearCarretera(colaCarretera, R_origen, R_destino, distancia, tiempo, penalizacion);
                encolar(colaCarretera, nueva, sizeof(Carretera));
                printf("✅ Carretera registrada correctamente.\n");
            } break;

            case 2:{
                listarCola(colaCarretera, mostrarCarretera);
                Nodo *n = extraerNodoPorCriterio(colaCarretera, criterioCarreteraPorID);
                if (n) {
                    free(n->dato);
                    free(n);
                    printf("🛣️ Carretera eliminada correctamente.\n");
                } else {
                    printf("❌ Carretera no encontrada.\n");
                }

            }
                break;

            case 3:
                listarCola(colaCarretera, mostrarCarretera);
                break;

            case 0:
                printf("↩ Volviendo al menú anterior.\n");
                break;

            default:
                printf("❌ Opción inválida.\n");
        }
    } while (op != 0);
}


// --------------------------- MENU PRINCIPAL ---------------------------

void menu_inventario(Cola *colaCat, Cola *colaLoc, Cola *colaProd, Cola *colaCliente, Cola *colaDirecc, Cola *colaCamion, Cola *colaCarretera) {
    int op = 0;
    do {
        printf("\n=== GESTIÓN DE INVENTARIO ===\n");
        printf("1. Categoría\n2. Localidad\n3. Producto\n4. Clientes y Direcciones\n5. Camión\n0. Salir\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) 
            continue;

        switch (op) {
            case 1: submenu_categoria(colaCat); break;
            case 2: submenu_localidad(colaLoc); break;
            case 3: submenu_producto(colaProd,colaCat); break;
            case 4: submenu_cliente(colaCliente,colaLoc,colaDirecc); break;
            case 5: submenu_camion(colaCamion); break;
            case 6: submenu_carreteras(colaCarretera,colaLoc); break;
            case 0: printf("👋 Saliendo modo Admin\n"); break;
            default: printf("❌ Opción inválida.\n");
        }
    } while (op != 0);
}

void submenu_cliente_pedido(Cola *colaCliente, Cola *colaDirecc){
    int op = 0;
    do {
        puts("\n=== CLIENTES Y PEDIDOS ===");
        puts("1. Realizar Pedido");
        puts("2. Editar o Cancelar Pedidos");
        puts("3. Listar Pedidos en EN ESPERA");
        puts("4. Historial de Cliente");
        printf("0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: realizarPedido(colaCliente,&colaProd,&colaPedidos); break;
            case 2: listarCola(colaDirecc,mostrarDireccion); break; //Ordenar los productos que se han asignado a un camión en específico por nombre, peso y volumen
            case 3: listarCola(&colaPedidos,mostrarCargaPedido);
            case 0: puts("↩ Volviendo al menú anterior."); break;
            default: puts("❌ Opción inválida.");
        }
    } while (op != 0);
    return;
}

void submenu_rutas(Cola *colaCliente, Cola *colaDirecc, Cola *colaCarretera, Cola *colaProd){
    int op = 0;
    do {
        puts("\n=== ASIGNAR RUTAS Y PEDIDOS A CAMIONES ===");
        puts("1. Asignar pedidos a camion (MAYOR DEMANDA o CANTIDAD)");
        puts("2. Asignar pedidos a camion (MAXIMIZAR VALOR PRODUCTOS)");
        puts("3. Asignar ruta a camion (MENOR DISTANCIA)");
        puts("4. Asignar ruta a camion (MENOR TIEMPO)");
        printf("0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: listarCola(colaCarretera,mostrarCarretera); break;
            case 2: ordenarProductosPorCategoria(colaProd,&colaCat); break;
            case 3: listarCola(colaCliente,mostrarCliente); break; //Ordenar los productos que aún no se han asignado a un camión por nombre, peso y volumen
            case 4: listarCola(colaDirecc,mostrarDireccion); break; //Ordenar los productos que se han asignado a un camión en específico por nombre, peso y volumen
            case 0: puts("↩ Volviendo al menú anterior."); break;
            default: puts("❌ Opción inválida.");
        }
    } while (op != 0);
}

void submenu_consultas(Cola *colaCliente, Cola *colaDirecc,TrieNode *trieProd){
    int op = 0;
    do {
        puts("\n=== CONSULTAS Y ORDENAMIENTOS ===");
        puts("1. Buscar Producto por Nombre");
        puts("2. Ordenar Productos por Categoria");
        puts("\n--- ORDENAR PRODUCTOS EN ESPERA ---");
        puts("3. por NOMBRE");
        puts("4. por PESO");
        puts("5. por VOLUMEN");
        puts("\n--- ORDENAR PRODUCTOS EN UN CAMION ---");
        puts("6. por NOMBRE");
        puts("7. por PESO");
        puts("8. por VOLUMEN");
        printf("0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: buscarProductoPorNombre(trieProd); break;
            case 2: ordenarProductosPorCategoria(&colaProd,&colaCat); break;
            case 3: listarCola(colaCliente,mostrarCliente); break; //Ordenar los productos que aún no se han asignado a un camión por nombre, peso y volumen
            case 4: listarCola(colaDirecc,mostrarDireccion); break; //Ordenar los productos que se han asignado a un camión en específico por nombre, peso y volumen
            case 0: puts("↩ Volviendo al menú anterior."); break;
            default: puts("❌ Opción inválida.");
        }
    } while (op != 0);
}

void menu_rutas_y_pedidos(Cola *colaCliente, Cola *colaDirecc, Cola *colaCarretera, TrieNode *trieProd, Cola *colaProd) {
    int op = 0;
    do {
        puts("\n=== GESTIÓN DE RUTAS Y PEDIDOS ===");
        puts("1. Clientes y Pedidos");
        puts("2. Asignar rutas y pedidos a camiones");
        puts("3. Consultas y Ordenamientos");
        printf("0. Volver\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1: submenu_cliente_pedido(colaCliente, colaDirecc); break;
            case 2: submenu_rutas(colaCliente, colaDirecc, colaCarretera, colaProd); break;
            case 3: submenu_consultas(colaCliente, colaDirecc,trieProd); break;
            case 0: puts("↩ Volviendo al menú anterior."); break;
            default: puts("❌ Opción inválida.");
        }
    } while (op != 0);
}

void menu_admin() {
    int op = 0;
    do {
        puts("\n=== MENÚ ADMINISTRADOR ===");
        puts("1. Gestión de Inventario");
        puts("2. Gestión de Rutas y Pedidos");
        printf("0. Salir\nSeleccione una opción: ");
        if (!leer_int_seguro(&op)) continue;

        switch (op) {
            case 1:
                menu_inventario(&colaCat, &colaLoc, &colaProd, &colaCliente,&colaDirecc, &colaCamion, &colaCarretera);
                break;
            case 2:
                menu_rutas_y_pedidos(&colaCliente, &colaDirecc, &colaCarretera, trieProd, &colaProd);
                break;
            case 0:
                puts("👋 Cerrando sesión de administrador...");
                break;
            default:
                puts("❌ Opción inválida.");
        }
    } while (op != 0);
}

// --------------------------- CREAR RELACIONES ---------------------------

void asociarDireccionesPorCliente(Cola *colaCliente, Cola *colaDirecc) {
    if (colaCliente->tam == 0 || colaDirecc->tam == 0) return;

    // Paso 1: Crear arreglo de punteros a Cliente indexado por ID
    int max_id = colaCliente->contador_ids;
    Cliente **clientesPorID = (Cliente **)calloc(max_id + 1, sizeof(Cliente *));
    if (!clientesPorID) {
        printf("❌ Error al asignar memoria para arreglo de clientes.\n");
        return;
    }

    Nodo *nodoCli = colaCliente->frente;
    while (nodoCli != NULL) {
        Cliente *c = (Cliente *)nodoCli->dato;
        if (c->id <= max_id) {
            clientesPorID[c->id] = c;
        }
        nodoCli = nodoCli->siguiente;
    }

    // Paso 2: Recorrer las direcciones y asignar a su cliente correspondiente
    Nodo *nodoDir = colaDirecc->frente;
    while (nodoDir != NULL) {
        Direccion *d = (Direccion *)nodoDir->dato;
        if (!d->activo) {
            nodoDir = nodoDir->siguiente;
            continue;
        }

        if (d->id_cliente > 0 && d->id_cliente <= max_id) {
            Cliente *cliente = clientesPorID[d->id_cliente];
            if (cliente) {
                if (cliente->direcciones.frente == NULL)
                    inicializarCola(&cliente->direcciones);

                Cliente_Direccion *cd = (Cliente_Direccion *)malloc(sizeof(Cliente_Direccion));
                cd->id = d->id;
                cd->id_cliente = cliente->id;
                cd->direccion = d;

                encolar(&cliente->direcciones, cd, sizeof(Cliente_Direccion));
            }
        }

        nodoDir = nodoDir->siguiente;
    }

    free(clientesPorID);  // Liberamos el arreglo auxiliar
    puts("🔁 Referencias de clientes-direcciones reconstruidas correctamente.");

}

void asociarLocalidadesADirecciones(Cola *colaDirecc, Cola *colaLoc) {
    Nodo *nodoDir = colaDirecc->frente;

    while (nodoDir != NULL) {
        Direccion *d = (Direccion *)nodoDir->dato;
        if (d && d->activo) {
            Localidad *loc = buscarLocalidad(colaLoc, d->id_localidad);
            if (loc) {
                d->R_localidad = loc;
            } else {
                d->R_localidad = NULL;
                printf("⚠️ Localidad con ID %d no encontrada para dirección ID %d.\n", d->id_localidad, d->id);
            }
        }
        nodoDir = nodoDir->siguiente;
    }
    puts("🔁 Referencias de direcciones-localidad reconstruidas correctamente.");

}

void asociarLocalidadesACarreteras(Cola *colaCarretera, Cola *colaLocalidad) {
    // Crear arreglo auxiliar
    int n = colaLocalidad->tam;
    Localidad **mapa_localidades = (Localidad **)calloc(n, sizeof(Localidad *));

    Nodo *tempLoc = colaLocalidad->frente;
    while (tempLoc) {
        Localidad *l = (Localidad *)tempLoc->dato;
        if (l->id >= 0 && l->id < n)
            mapa_localidades[l->id] = l;
        tempLoc = tempLoc->siguiente;
    }

    Nodo *tempCarretera = colaCarretera->frente;
    while (tempCarretera) {
        Carretera *c = (Carretera *)tempCarretera->dato;

        if (c->id_origen >= 0 && c->id_origen < n)
            c->R_origen = mapa_localidades[c->id_origen];
        if (c->id_destino >= 0 && c->id_destino < n)
            c->R_destino = mapa_localidades[c->id_destino];

        tempCarretera = tempCarretera->siguiente;
    }

    free(mapa_localidades);

    puts("🔁 Referencias de carreteras reconstruidas correctamente.");
}

int main() {
    SetConsoleOutputCP(65001); // Cambiar código de página a UTF-8

    inicializarCola(&colaCat);
    inicializarCola(&colaLoc);
    inicializarCola(&colaProd);
    inicializarCola(&colaCliente);
    inicializarCola(&colaDirecc);
    inicializarCola(&colaCamion);
    inicializarCola(&colaCarretera);
    inicializarCola(&colaPedidos);


    trieProd = NULL;  // al principio
    trieProd = crearNodoTrie();  // Inicializa raíz del Trie

    // Cargar datos al inicio
    cargarCola(&colaCat, "categorias.bin",sizeof(Categoria));
    cargarCola(&colaLoc, "localidades.bin",sizeof(Localidad));
    cargarCola(&colaProd, "productos.bin",sizeof(Producto));
    cargarCola(&colaCliente, "clientes.bin",sizeof(Cliente));
    cargarCola(&colaDirecc, "direcciones.bin",sizeof(Direccion));
    cargarCola(&colaCamion, "camiones.bin",sizeof(Camion));
    cargarCola(&colaCarretera, "rutas.bin",sizeof(RutaCamion));
    cargarCola(&colaPedidos, "pedidos.bin",sizeof(CargaPedido));


    // Construir el árbol TRIE
    Nodo *t = colaProd.frente;
    while (t) {
        insertarProductoTrie(trieProd, ((Producto *)t->dato)->nombre, t->dato);
        t = t->siguiente;
    }
    
    reconstruirReferenciasProducto(&colaProd,&colaCat);
    asociarLocalidadesADirecciones(&colaDirecc, &colaLoc);
    asociarDireccionesPorCliente(&colaCliente, &colaDirecc);
    asociarLocalidadesACarreteras(&colaCarretera,&colaLoc);
    reconstruirReferenciasPedidos(&colaPedidos,&colaDirecc,&colaProd);

    listarCola(&colaPedidos,mostrarCargaPedido);

    menu_admin();

    limpiarReferenciasPedidos(&colaPedidos);
    limpiarReferenciasProducto(&colaProd);
    limpiarReferenciasClienteDireccion(&colaCliente,&colaDirecc);
    limpiarReferenciasCarreteras(&colaCarretera);
    limpiarReferenciasDireccion(&colaDirecc);


    // Guardar datos al salir
    guardarCola(&colaCat, "categorias.bin",sizeof(Categoria));
    guardarCola(&colaLoc, "localidades.bin",sizeof(Localidad));
    guardarCola(&colaProd, "productos.bin",sizeof(Producto));
    guardarCola(&colaCliente, "clientes.bin",sizeof(Cliente));
    guardarCola(&colaDirecc, "direcciones.bin",sizeof(Direccion));
    guardarCola(&colaCamion, "camiones.bin",sizeof(Camion));
    guardarCola(&colaCarretera, "carreteras.bin",sizeof(Carretera));
    guardarCola(&colaPedidos, "pedidos.bin", sizeof(CargaPedido));



    liberarTrie(trieProd);

    liberarCola(&colaCat);
    liberarCola(&colaLoc);
    liberarCola(&colaProd);
    liberarCola(&colaCliente);
    liberarCola(&colaDirecc);

    liberarCola(&colaCamion);
    liberarCola(&colaCarretera);
    liberarCola(&colaPedidos);


    return 0;
}

//para hacer el ordenamiento de productos que se encuentran en un camión
//una vez ingresados en el camión, los productos se ordenan dentro de cada camión de acuerdo al nombre, peso o volumen (en total)
//ES POR CAMION, ELIJO EL CAMION Y DE AHI
//los pedidos solo se guardan los que están en espera, no los que se completaron (esos se pueden insertar en un .txt, junto con la ruta)