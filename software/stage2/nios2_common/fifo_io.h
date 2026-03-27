#ifndef FIFO_IO_H
#define FIFO_IO_H

#include "datatype.h"

// Initializes the FIFO Control and Status Register (CSR)
void fifo_init(unsigned int fifo_csr_base);

// Blocking read: Waits until data is available, then reads 'length' items
void fifo_read_block(unsigned int fifo_base, unsigned int fifo_csr, INT16 *buffer, int length);

// Blocking write: Waits until space is available, then writes 'length' items
void fifo_write_block(unsigned int fifo_base, unsigned int fifo_csr, INT16 *buffer, int length);

#endif // FIFO_IO_H