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

#ifndef RINGBUF_H_
#define RINGBUF_H_

/**
 * Пара примечаний по коду:
 * 1. В некоторых местах были использованны конструкции типа
 *     i = i + 1;
 *     if (i == MAX) { i = 0; }
 *    вместо операции % (mod). Такое решение было принято, т.к. операции умножения,
 *    деления и остатка от деления очень дороги на микроконтроллерах.
 */

#include <stdbool.h>

#define UART_BUFFER_SIZE 64

typedef struct {
	unsigned char buffer[UART_BUFFER_SIZE];

	/**
	 * Индекс для операции записи.
	 */
	volatile unsigned char head;

	/**
	 * Индекс для операции чтения.
	 */
	volatile unsigned char tail;
} ring_buffer;

__inline unsigned int ring_len(ring_buffer *buffer) {
	short firstLen = buffer->head - buffer->tail;

	if (firstLen >= 0) {
		/*
		 * Указатель для записи находится спереди или на том же месте
		 * как и указатель для чтения (в этом случае буфер пуст).
		 *
		 * |---T-----H----|
		 *
		 * T - buffer->tail
		 * H - buffer->head
		 */
		return firstLen;
	} else {
		/*
		 * Указатель для записи находится позади указателя чтения. Это
		 * значит что указатель головы (записи) перешёл на новый круг, в то время как
		 * указатель хвоста (чтения) остался на прежнем кругу.
		 *
		 * |--H---------T-|
		 *
		 * T - buffer->tail
		 * H - buffer->head*
		 */

		return -firstLen - 1;
	}
}

/*
 * Возвращает, пустой ли кольцевой буффер?
 */
__inline bool ring_empty(ring_buffer *buffer) {
	return buffer->head == buffer->tail;
}

/*
 * Кладет в буффер элемент.
 */
__inline void ring_push(ring_buffer *buffer, unsigned char ch) {

	// См. прим. 1.
	unsigned char i = (unsigned char) (buffer->head + 1);
	if (i == UART_BUFFER_SIZE) {
		i = 0;
	}

	if (i != buffer->tail) {
		buffer->buffer[buffer->head] = ch;
		buffer->head = i;
	}
}

/*
 * Возвращает последний элемент в списке.
 */
__inline bool ring_peek(ring_buffer *buffer, unsigned char *ch) {
	if (ring_empty(buffer)) {
		return false;
	}

	*ch = buffer->buffer[buffer->tail];

	return true;
}

/*
 * Извлекает один элемент из буффера.
 */
__inline bool ring_pop(ring_buffer *buffer, unsigned char *ch) {
	if (ring_empty(buffer)) {
		return false;
	}

	*ch = buffer->buffer[buffer->tail];

	// См. прим. 1
	unsigned char new_tail = buffer->tail + 1;
	if (new_tail == UART_BUFFER_SIZE) {
		new_tail = 0;
	}

	buffer->tail = new_tail;

	return true;
}

#endif /* RINGBUF_H_ */
