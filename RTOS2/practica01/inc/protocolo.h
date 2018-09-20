/*
 * protocolo.h
 *
 *  Created on: 18 sep. 2018
 *      Author: froux
 */

#ifndef RTOS2_PRACTICA01_INC_PROTOCOLO_H_
#define RTOS2_PRACTICA01_INC_PROTOCOLO_H_

#define TIMEOUT_RX_PAQUETE portMAX_DELAY


#define PRT_STX             0x55
#define PRT_ETX             0xAA

typedef enum {PRT_MAYUS = 0x00, PRT_MINUS = 0x01, PRT_REPSTACK = 0x02, PRT_REPHEAP = 0x03, PRT_MSJEST = 0x04} op_t;
typedef enum {RECIBIR_STX, RECIBIR_OP, RECIBIR_T, RECIBIENDO_DATOS, RECIBIR_ETX} estadoRecepcion_t;

extern int32_t inicializarRecibirPaquete (void);
extern void tareaRecibirPaquete (void* taskParam);


#endif /* RTOS2_PRACTICA01_INC_PROTOCOLO_H_ */
