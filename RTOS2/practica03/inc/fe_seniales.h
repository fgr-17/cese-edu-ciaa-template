/*
 * seniales.h
 *
 *  Created on: 15/06/2012
 *      Author: alejandro
 */

#ifndef SENIALES_H_
#define SENIALES_H_

//Aqui se deben listar las seniales que genera la aplicacion
/*
 typedef enum {
	SIG_MODULO_INICIAR = 0,
	SIG_TIMEOUT,
	SIG_BOTON_PULSADO,
	SIG_BOTON_LIBERADO,
	SIG_ULTIMO_VALOR = SIG_BOTON_LIBERADO
} Signal_t;
*/

/**
 * @brief señales definidas para la aplicacion
 *
 * @def SOGTO;EPIT
 *
 */
typedef enum {
  SIG_MODULO_INICIAR = 0,
  SIG_TIMEOUT,
  SIG_BOTON_PULSADO,
  SIG_BOTON_LIBERADO,
  SIG_ULTIMO_VALOR = SIG_BOTON_LIBERADO
} Signal_t;


#endif /* SENIALES_H_ */
