#ifndef MNIST_AIFES_H
#define MNIST_AIFES_H
#include <stdint.h>
/* Pure C inference API: reusable on STM32 and Zynq. Not reentrant. */
int mnist_aifes_init(void);
/* 784 centered float32 inputs [-0.5,0.5], 10 Q7 softmax values (-128..127).
   Real output value = (q + 128) / 256. */
int mnist_aifes_predict(const float *input, int8_t *output);
uint32_t mnist_aifes_memory_required(void);
#endif
