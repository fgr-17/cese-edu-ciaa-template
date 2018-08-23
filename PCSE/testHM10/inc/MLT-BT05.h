/*
 * MLT-BT05.h
 *
 *  Created on: 20 ago. 2018
 *      Author: froux
 */

#ifndef PCSE_TESTHM10_INC_MLT_BT05_H_
#define PCSE_TESTHM10_INC_MLT_BT05_H_

#define MLT_BT05_CMD_LMAX   20


/** @brief prefijo y sufijo para todos los parametros */
#define MLT_BT05_PREFIJO        "AT"
#define MLT_BT05_SUFIJO         "\r\n"

/** @brief comando para ver si esta vivo - NADA */
#define MLT_BT05_CMD_VIVO           ""
/** @brief respuesta si esta vivo */
#define MLT_BT05_CMD_VIVO_RESP      "OK\r\n"
/** @brief pregunto la version */
#define MLT_BT05_CMD_VERSION        "VERSION"
/** @brief pregunto o seteo el nombre si le agrego parametro */
#define MLT_BT05_CMD_NAME           "NAME"
/** @brief pregunto o seteo el pin si le agrego parametro */
#define MLT_BT05_CMD_PIN            "PIN"
/** @brief reinicio  */
#define MLT_BT05_CMD_RESET                    "RESET"
/** @brief lo mando a dormir */
#define MLT_BT05_CMD_SLEEP                    "SLEEP"

/** @brief pregunto o seteo el baudrate si le agrego parametro */
#define MLT_BT05_CMD_BAUDRATE                 "BAUD"
#define MLT_BT05_CMD_BAUDRATE_115200          "0"
#define MLT_BT05_CMD_BAUDRATE_57600           "1"
#define MLT_BT05_CMD_BAUDRATE_38400           "2"
#define MLT_BT05_CMD_BAUDRATE_19200           "3"
#define MLT_BT05_CMD_BAUDRATE_9600            "4"

#define MLT_BT05_BAUDRATE_5   (sizeof(mlt_bt05_baudrate_t) / sizeof(mlt_bt05_baudrate_t))

/* --------------------------- tipos de dato ---------------------------------- */

typedef enum {BPS115200, BPS57600, BPS38400, BPS19200, BPS9600, BPS_ERROR} mlt_bt05_baudrate_t;

/* --------------------------- funciones externas ---------------------------------- */

extern int32_t MLT_BT05_armarComando (char*cad, const char*cmd);
extern mlt_bt05_baudrate_t MLT_BT05_inicializar (uartMap_t uart);

/* --------------------------- variables globales ---------------------------------- */

extern uint32_t mlt_bt05_baudrate [];




#endif /* PCSE_TESTHM10_INC_MLT_BT05_H_ */
