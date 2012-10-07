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

#include <msp430.h>
#include <stdbool.h>

#include <uart.h>
#include "ringbuf.h"

// #define SMCLK 16000000

/*
 * Задекларировать пины для ввода-вывода.
 */
#define RX_PIN		BIT1
#define TX_PIN		BIT2

static ring_buffer rx_buffer;
static ring_buffer tx_buffer;

#ifdef UART_COUNT_RXTX
unsigned short uart_bytes_rx;
unsigned short uart_bytes_tx;
#endif

// Флаг ожидания ввода. Сброс в USCI0RX_ISR.
static volatile bool waiting_for_rx = false;
static volatile bool waiting_for_tx = false;

/**
 * Ждать выполнения некоторого условия.
 */
static __inline void wait_for_rxtx(volatile bool *condition) {
	// Сохрнатить состояние флага GIE (General Interrupt Enable).
	bool gie_was_enabled = _get_SR_register() & GIE;

	while (*condition) {
		_BIS_SR(LPM0_bits | GIE);
	}

	// Убираем флаг GIE из SR, если он был установлен.
	if (!gie_was_enabled) {
		_BIC_SR(GIE);
	}
}

void uart_init() {
	// Выбрать специальную функцию для пинов RX_PIN и TX_PIN

	P1SEL |= RX_PIN | TX_PIN;
	P1SEL2 |= RX_PIN | TX_PIN;

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	// As said in 15.3.1 of holy user guide,
	// before initialization of USCI we must reset it.
	UCA0CTL1 = UCSWRST;

	UCA0CTL1 |= UCSSEL_2;

	UCA0CTL0 = 0;
	UCA0ABCTL = 0;

	UCA0BR0 = 104;
	UCA0BR1 = 0;
	UCA0MCTL = UCBRS0;

	// Запустить таймер.
	UCA0CTL1 &= ~UCSWRST;

	UC0IE |= UCA0RXIE;
	IE2 |= UCA0RXIE;

#ifdef UART_COUNT_RXTX
	uart_bytes_rx = 0;
	uart_bytes_tx = 0;
#endif
}

void uart_putc(unsigned char ch) {
	// TODO: Заблокировать, до освобождения буфера.

	ring_push(&tx_buffer, ch);

	// Включить прерывания передачи.
	UC0IE |= UCA0TXIE;
}

void uart_puts(const char *str) {
	while (*str) {
		ring_push(&tx_buffer, *str++);
	}

	// Включить прерывания передачи.
	UC0IE |= UCA0TXIE;
}

void uart_putn(unsigned char *str, unsigned int count) {
	while (count--) {
		ring_push(&tx_buffer, *str++);
	}

	// Включить прерывания передачи.
	UC0IE |= UCA0TXIE;
}

void uart_flush() {
	if (ring_empty(&tx_buffer)) {
		// Буфер и так пуст.
		return;
	}

	waiting_for_tx = true;
	wait_for_rxtx(&waiting_for_tx);
}

unsigned char uart_getc() {
	unsigned char ch;

	if (ring_empty(&rx_buffer)) {
		waiting_for_rx = true;
		wait_for_rxtx(&waiting_for_rx);
	}

	ring_pop(&rx_buffer, &ch);
	return ch;
}

unsigned short uart_getw() {
	unsigned char buf[2];

	buf[0] = uart_getc();
	buf[1] = uart_getc();

	return *(unsigned short *) buf;
}

bool uart_getc_noblock(unsigned char *ch) {
	// Кольцевой буфер займется всей грязной работой.
	return ring_pop(&rx_buffer, ch);
}

bool uart_getw_noblock(unsigned short *sh) {
	if (ring_len(&rx_buffer) < 2) {
		return false;
	}

	unsigned char buf[2];
	if (ring_pop(&rx_buffer, &buf[0]) && ring_pop(&rx_buffer, &buf[1])) {
		*sh = *(unsigned short *) buf;

		return true;
	} else {
		return false;
	}
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

	// Прочитать символ из буфера-приемника.
	unsigned char ch = UCA0RXBUF;

	if (waiting_for_rx) {
		// Если мы ждем передачи, то сбросить флаг ожидания передачи и
		// возвратить CPU в нормальное состояние.
		waiting_for_rx = false;
		_BIC_SR_IRQ(LPM0_bits);
	}

	// Положить во входной буффер пришедший символ.
	ring_push(&rx_buffer, ch);

#ifdef UART_COUNT_RXTX
	uart_bytes_rx++;
#endif
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {

	if (ring_empty(&tx_buffer)) {
		// Исходящий буфер пуст. Выключаем прерывание на вывод USCI.
		UC0IE &= ~UCA0TXIE;

		if (waiting_for_tx) {
			// Если мы ждем окончания передачи данных, то сбросить
			// флаг и перейти в активный режим CPU.
			waiting_for_tx = false;
			_BIC_SR_IRQ(LPM0_bits);
		}

		return;
	}

	unsigned char ch;

	if (ring_pop(&tx_buffer, &ch)) {
		UCA0TXBUF = ch;

#ifdef UART_COUNT_RXTX
		uart_bytes_tx++;
#endif
	}
}
