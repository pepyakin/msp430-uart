msp430-uart
===========

Simple asynchronous implementation of hardware UART protocol.
It occupies 2 interrupt vectors:
* USCIAB0RX_VECTOR
* USCIAB0TX_VECTOR 

Project defines 2 macros:
UART_BUFFER_SIZE - maximal byte count receive/transmit buffer can hold.
UART_COUNT_RXTX - define this macro if you want to count received and transmitted bytes.

Interrupts must be enabled in order to receive or transmit data, blocking functions might enable interrupts
(set GIE flag in SR) at will for waiting in low-power mode (LPM0). 
Also, interrupts must be enabled if you want to receive data asynchronously.


Some basic example: https://gist.github.com/3853003