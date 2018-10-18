/*
 * token.h
 *
 *  Created on: 17 oct. 2018
 *      Author: froux
 */

#ifndef RTOS2_PRACTICA02_SRC_TOKEN_H_
#define RTOS2_PRACTICA02_SRC_TOKEN_H_


typedef struct {
  uint32_t id_de_paquete;       // identificacion de tránsito de paq. por el sistema. Autonumérico
  uint8_t* payload;             // puntero a paquete de datos a procesar



} token_t;


#endif /* RTOS2_PRACTICA02_SRC_TOKEN_H_ */
