#include "include/fpgaAccess.h"
#include "include/dllqhyccd.h"

uint32_t qhyWriteFPGA(qhyccd_handle *handle, uint16_t index, uint16_t value)
{
    if(handle == NULL || libqhyccd->QHYCCDVendRequestWrite == NULL)
        return QHYCCD_ERROR;

    uint8_t data[10] = { 0 };
    return libqhyccd->QHYCCDVendRequestWrite(handle, 0xb9, value, index, 1, data);
}

uint8_t qhyReadFPGA(qhyccd_handle *handle, uint16_t index)
{
    uint8_t data[10] = { 0 };
    if(handle != NULL && libqhyccd->QHYCCDVendRequestRead != NULL)
        libqhyccd->QHYCCDVendRequestRead(handle, 0xbc, 0, index, 1, data);
    return data[0];
}

uint32_t qhyWriteFPGAExtend(qhyccd_handle *handle, uint16_t index, uint32_t value)
{
    uint32_t ret = qhyWriteFPGA(handle, 228, 0x00);
    if(ret != QHYCCD_SUCCESS)
        return ret;

    ret = qhyWriteFPGA(handle, 229, 0x00);
    if(ret != QHYCCD_SUCCESS)
        return ret;

    uint8_t indexMask = 0x03;
    uint8_t valueMask = 0x0f;
    uint8_t address = 223;
    while(indexMask > 0)
    {
        ret = qhyWriteFPGA(handle, address, (uint8_t)(index & 0x00FF));
        if(ret != QHYCCD_SUCCESS)
            return ret;
        address--;
        indexMask >>= 1;
        index >>= 8;
    }

    address = 227;
    while(valueMask > 0)
    {
        ret = qhyWriteFPGA(handle, address, (uint8_t)(value & 0x000000FF));
        if(ret != QHYCCD_SUCCESS)
            return ret;
        address--;
        valueMask >>= 1;
        value >>= 8;
    }

    ret = qhyWriteFPGA(handle, 228, 0x08);
    if(ret != QHYCCD_SUCCESS)
        return ret;

    return qhyWriteFPGA(handle, 228, 0x00);
}

uint32_t qhyReadFPGAExtend(qhyccd_handle *handle, uint16_t index)
{
    uint32_t value = 0;

    qhyWriteFPGA(handle, 228, 0x00);
    qhyWriteFPGA(handle, 229, 0x00);

    uint8_t indexMask = 0x03;
    uint8_t address = 223;
    while(indexMask > 0)
    {
        qhyWriteFPGA(handle, address, (uint8_t)(index & 0x00FF));
        address--;
        indexMask >>= 1;
        index >>= 8;
    }

    qhyWriteFPGA(handle, 229, 0x08);
    address = 60;
    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t value8 = qhyReadFPGA(handle, address);
        address++;
        value = (value << 8) | value8;
    }
    qhyWriteFPGA(handle, 229, 0x00);
    return value;
}
