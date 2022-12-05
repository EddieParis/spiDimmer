/*
 * spi.h
 *
 * Created: 18/11/2018 10:53:57
 *  Author: Parents
 */ 


#ifndef SPI_H_
#define SPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

void SpiInit(void);
uint8_t SpiGetData(void);
void SpiSetData(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */