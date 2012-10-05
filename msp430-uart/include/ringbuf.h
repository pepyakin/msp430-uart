/*
 * ringbuf.h
 *
 *  Created on: 03.10.2012
 *      Author: Sergey
 */

#ifndef RINGBUF_H_
#define RINGBUF_H_

#include <stdbool.h>

#define BUFFER_SIZE 64

typedef struct {
	unsigned char buffer[BUFFER_SIZE];
	volatile unsigned char head;
	volatile unsigned char tail;
} ring_buffer;

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
	unsigned char i = (unsigned char) (buffer->head + 1);
	if (i == BUFFER_SIZE) {
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

	unsigned char new_tail = buffer->tail + 1;
	if (new_tail == BUFFER_SIZE) {
		new_tail = 0;
	}

	buffer->tail = new_tail;

	return true;
}




#endif /* RINGBUF_H_ */
