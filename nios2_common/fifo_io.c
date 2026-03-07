#include "fifo_io.h"

#ifdef __NIOS2__
// --- HARDWARE IMPLEMENTATION (Altera Nios II) ---
#include <io.h>
#include "altera_avalon_fifo_regs.h"

void fifo_init(unsigned int fifo_csr_base) {
    // Clear the FIFO event register to reset any lingering interrupt/error states
    IOWR_ALTERA_AVALON_FIFO_EVENT(fifo_csr_base, 0x0);
}

void fifo_read_block(unsigned int fifo_base, unsigned int fifo_csr, INT16 *buffer, int length) {
    int i;
    for(i = 0; i < length; i++) {
        // Spin-lock (block) while the FIFO fill level is 0
        while (IORD_ALTERA_AVALON_FIFO_LEVEL(fifo_csr) == 0) { }
        
        // Read 16-bit word from the FIFO data port
        buffer[i] = (INT16)IORD_ALTERA_AVALON_FIFO_DATA(fifo_base);
    }
}

void fifo_write_block(unsigned int fifo_base, unsigned int fifo_csr, INT16 *buffer, int length) {
    int i;
    for(i = 0; i < length; i++) {
        // Spin-lock (block) while the FIFO is completely FULL
        while (IORD_ALTERA_AVALON_FIFO_STATUS(fifo_csr) & ALTERA_AVALON_FIFO_STATUS_F_MSK) { }
        
        // Write 16-bit word to the FIFO data port
        IOWR_ALTERA_AVALON_FIFO_DATA(fifo_base, buffer[i]);
    }
}

#else
// --- PC MOCK IMPLEMENTATION (For Local Unit Testing) ---
#include <stdio.h>

void fifo_init(unsigned int fifo_csr_base) { 
    // Do nothing on PC
}

void fifo_read_block(unsigned int fifo_base, unsigned int fifo_csr, INT16 *buffer, int length) {
    // In actual PC testing, you'd hook this up to a global array or file
    printf("[MOCK FIFO] Read %d words from Base 0x%08X\n", length, fifo_base);
}

void fifo_write_block(unsigned int fifo_base, unsigned int fifo_csr, INT16 *buffer, int length) {
    printf("[MOCK FIFO] Wrote %d words to Base 0x%08X\n", length, fifo_base);
}
#endif