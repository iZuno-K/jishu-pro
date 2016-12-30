#ifndef _ADXL345_H_
#define _ADXL345_H_

void adxl345_readData(int *accelData, int fd);
void adxl345_init(int fd);

#endif //_ADXL345_H_
