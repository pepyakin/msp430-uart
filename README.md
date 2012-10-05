msp430-uart
===========

Simple asynchronous implementation of hardware UART protocol.
It occupies 2 interrupt vectors:
* USCIAB0RX_VECTOR
* USCIAB0TX_VECTOR 

Project defines 2 macros:
UART_BUFFER_SIZE - maximal byte count receive/transmit buffer can hold.