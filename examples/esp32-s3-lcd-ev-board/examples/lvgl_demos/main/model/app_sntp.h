/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef _APP_SNTP_H_
#define _APP_SNTP_H_


#ifdef __cplusplus
extern "C" {
#endif

// Inicialización bloqueante tradicional
void app_sntp_init(void);

// Inicialización no-bloqueante (solo inicia proceso, no espera)
void app_sntp_init_async(void);

#ifdef __cplusplus
}
#endif

#endif
