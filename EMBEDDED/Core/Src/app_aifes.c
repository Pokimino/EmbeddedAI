#include "app_aifes.h"
#include "mnist_aifes.h"
#include "main.h"
#include <string.h>

extern UART_HandleTypeDef huart2;
volatile uint32_t aifes_last_error;
volatile uint32_t aifes_frames_ok;
static uint8_t init_status;
static float input[784];
static int8_t output[10];
#include "mnist_selftest.inc"

/* Protocol per image:
   PC -> AB; board -> "AIF7" + init_status (0=ready)
   PC -> 784 little-endian float32 centered pixels (3136 bytes)
   board -> frame_status (0=ok) + 10 unsigned (Q7+128) scores.
   Probability approximation = unsigned_score/256, not /255.
   The marker is only parsed while waiting for a new image. */
void APP_AIFES_Init(void)
{
    init_status=(uint8_t)mnist_aifes_init();
    if(!init_status) {
        int rc=mnist_aifes_predict(selftest_input,output);
        if(rc || memcmp(output,selftest_expected,10)!=0) init_status=10;
    }
    aifes_last_error=init_status;
    aifes_frames_ok=0;
}

void APP_AIFES_Process(void)
{
    uint8_t marker=0;
    HAL_StatusTypeDef status=HAL_UART_Receive(&huart2,&marker,1,100);
    if(status!=HAL_OK) {
        if(status!=HAL_TIMEOUT) {
            aifes_last_error=20;
            (void)HAL_UART_AbortReceive(&huart2);
            __HAL_UART_CLEAR_OREFLAG(&huart2);
        }
        return; /* Idle is normal; do not enter Error_Handler on timeout. */
    }
    if(marker!=0xAB) return;
    uint8_t hello[5]={'A','I','F','7',init_status};
    if(HAL_UART_Transmit(&huart2,hello,sizeof(hello),1000)!=HAL_OK) {
        aifes_last_error=21; return;
    }
    if(init_status) return;
    uint8_t reply[11]={0};
    status=HAL_UART_Receive(&huart2,(uint8_t *)input,sizeof(input),10000);
    if(status!=HAL_OK) {
        reply[0]=2;
        aifes_last_error=22;
        (void)HAL_UART_AbortReceive(&huart2);
        __HAL_UART_CLEAR_OREFLAG(&huart2);
    } else {
        int rc=mnist_aifes_predict(input,output);
        if(rc) {
            reply[0]=3;
            aifes_last_error=30+(uint32_t)rc;
        } else {
            for(unsigned i=0;i<10;i++) reply[i+1]=(uint8_t)((int16_t)output[i]+128);
            aifes_frames_ok++;
            aifes_last_error=0;
        }
    }
    if(HAL_UART_Transmit(&huart2,reply,sizeof(reply),1000)!=HAL_OK)
        aifes_last_error=23;
}
