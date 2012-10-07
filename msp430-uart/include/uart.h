/*
 *                             ZLib license
 *
 * Copyright (C) 2012 Sergey Pepyakin
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Sergey Pepyakin
 */

#ifndef __UART_H
#define __UART_H

#include <stdbool.h>

/**
 * Инициализирует UART 9600 бод.
 */
void uart_init();

/**
 * Процедура для вывода одного символа в UART.
 * Асинхронна: Процедура не ожидает завершения передачи данных.
 */
void uart_putc(unsigned char ch);

/**
 * Процедура для вывода 0-terminated строки в UART.
 * Асинхронна: Процедура не ожидает завершения передачи данных.
 */
void uart_puts(const char *str);

/**
 * Процедура, посылающая count символов (в том числе и нулевого байта (0x00)) из буффера str в UART.
 */
void uart_putn(unsigned char *str, unsigned int count);

/**
 * Процедура ожидающая полного освобождения буфера передачи.
 */
void uart_flush();

/**
 * Возвращает символ из входного буфера. Если символа нет - блокируется и ждет прихода символа.
 * Блокировка происходит в LPM0 + GIE. GIE после блокировки востанавливается до предыдущего состояния.
 */
unsigned char uart_getc();

/**
 * Последовательное считывание двух байт uart_getc, и возвращение беззнакового short.
 */
unsigned short uart_getw();

/**
 *
 */
bool uart_getc_noblock(unsigned char *ch);

/**
 *
 */
bool uart_getw_noblock(unsigned short *sh);

#endif
