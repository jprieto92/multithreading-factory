#ifndef _DB_factory_
#define _DB_factory_

/*
 * Funcion que inicializa la base de datos. Usar solo una vez al inicio.
 * Entrada:
 * Salida:
 * Valor de retorno: 0 -> ok, -1 -> error
 */
int db_factory_init();

/*
 * Funcion que elimina los recusos utilizados de la base de datos.
 * Usar solo una vez al finalizar.
 * Entrada:
 * Salida:
 * Valor de retorno: 0 -> ok, -1 -> error
 */
int db_factory_destroy();

/*
 * Funcion que crea un nuevo elemento en el primer registro disponible
 * del almacen. No controla la existencia de otros elementos con el mismo nombre.
 * Entrada: nombre del elemento
 * Salida:
 * Valor de retorno: 0 -> ok, -1 -> error
 */
int db_factory_create_element(char * element_name, int stock, int * id);


/*
 * Funcion que actualiza el stock de un elemento
 * Entrada: id del elemento y stock del elemento
 * Salida:
 * Valor de retorno: 0 -> ok, -1 -> error
 */
int db_factory_update_stock(int id, int stock);

/*
 * Funcion que devuelve el stock de un elemento
 * Entrada: id del elemento
 * Salida: stock del elemento
 * Valor de retorno: 0 -> ok, -1 -> error
 */
int db_factory_get_stock(int id, int *stock);

/*
 * Funcion que permite asociar una serie de datos a un elemento concreto.
 * Se puede utilizar para guardar con el elemento datos relacionados con
 * la sincronizacion de procesos ligeros.
 * Entrada: id ddel elemento, puntero a los datos y tama�o de los mismos
 * Salida:
 * Valor de retorno: 0 -> ok, -1 -> error
 */
int db_factory_set_internal_data(int id, void *ptr, int size);

/* Funcion que permite saber el estado de un elemento a partir de su id.
 * Entrada: id del elemento
 * Salida: puntero al estado del elemento
 * Valor de retornor: 0 -> ok, -1 -> error
 */
int db_factory_get_ready_state(int id, int *state);

/*
 * Funcion que devuelve datos externos asociados a un elemento.
 * Entrada: id del elemento,
 * Salida: puntero a los datos y tama�o de los mismos
 * Valor de retorno: 0 -> ok, -1 -> error
 */
int db_factory_get_internal_data(int id, void **ptr, int *size);

/*
 * Funcion que devuelve el nombre de un elemento dado su id.
 * Entrada: id del elemento,
 * Salida: puntero al nombre del elemento
 * Valor de retorno: 0 -> ok, -1 -> error
 */

int db_factory_get_element_name(int id, char *name);

#endif //_DB_factory_
