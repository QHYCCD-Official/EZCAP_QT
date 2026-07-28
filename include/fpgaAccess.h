#ifndef FPGAACCESS_H
#define FPGAACCESS_H

#include <stdint.h>

typedef void qhyccd_handle;

uint32_t qhyWriteFPGA(qhyccd_handle *handle, uint16_t index, uint16_t value);
uint8_t qhyReadFPGA(qhyccd_handle *handle, uint16_t index);
uint32_t qhyWriteFPGAExtend(qhyccd_handle *handle, uint16_t index, uint32_t value);
uint32_t qhyReadFPGAExtend(qhyccd_handle *handle, uint16_t index);

#endif // FPGAACCESS_H
