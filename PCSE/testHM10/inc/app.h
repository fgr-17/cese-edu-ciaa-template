/*
 * app.h
 *
 *  Created on: 26 ago. 2018
 *      Author: froux
 */

#ifndef PCSE_TESTHM10_INC_APP_H_
#define PCSE_TESTHM10_INC_APP_H_

// Tamaño del paquete en bytes
#define BYTES_PAQ     sizeof(frame_t)

/* --------------------------- tareas ---------------------------------- */
extern void tareaInicializarSistema( void* taskParmPtr );
extern void tareaBLEaPC( void* taskParmPtr );
extern void tareaPCaBLE( void* taskParmPtr );

/* --------------------------- tipos de datos ---------------------------------- */

/* ========================== [ enumeraciones ] ====================== */

/** @brief modo de funcionamiento de la aplicacion ppal */
typedef enum {MODO_CMD = 0x10, MODO_PASAMANOS} estadoApp_t;
/** @brief estado de protocolo */
typedef enum {ESPERANDO_LOGIN = 0x20, ESPERANDO_USUARIO, ESPERO_HYF, ESPERO_INICIO_LOG, ENVIO_DATOS} estadoAplicacionBLE_t;
/** @brief comandos del protocolo */
typedef enum {LOGIN = 'a', USUARIO, HYF, INICIAR_LOG, DATO, FIN_LOG} cmd_t;
/** @brief tipos de respuesta  a una cadena con comando */
typedef enum {RESP_OK = 0x30, RESP_ERR_CHKSUM, RESP_ERR_CMD, RESP_ERR_CTX} resp_t;

/* ========================== [ estructuras ] ====================== */

/** @brief tamaño del buffer de datos de la queue */
#define BUF_ADC_L                       (1000)
/** @brief tamaño del tipo de dato en bytes */
#define DATO_L                          (2)
/** @brief frecuencia de muestreo que quiero simular */
#define FS_SIMULADA_HZ                  (500)
/** @brief delay de la tarea que simula la produccion de datos del ADC */
#define TAREA_SIMULAR_MUESTREO_DELAY    ( (TickType_t) (BUF_ADC_L / FS_SIMULADA_HZ))

/** @brief item de la Queue del ADC simulado */
typedef struct {
  int16_t*pEnvio;
}itemQueueADC_t;

/** @brief largo de la cola de datos del ADC */
#define QUEUE_ADC_L  ( 3 )

/** @brief buffers del ADC */
int16_t buf1 [BUF_ADC_L];
int16_t buf2 [BUF_ADC_L];
/** @brief puntero de envio de datos generados por el ADC */
int16_t*pEnv;
/** @brief puntero activo donde guardo lo recibido por el ADC */
int16_t*pAct;



/** @brief campos del frame */
typedef struct {

  int8_t cmd;
  int8_t d0;
  int8_t d1;
  int8_t chksm;
} frame_t;

/** @brief union entre array de bytes y el frame */
typedef union {

  frame_t frame;
  uint8_t bytes[BYTES_PAQ];

} paq_t;


#endif /* PCSE_TESTHM10_INC_APP_H_ */
