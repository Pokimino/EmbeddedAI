#include "mnist_aifes.h"
#include "aifes_basic.h"
#include <math.h>
#include <string.h>

#include "mnist_q7_weights.inc"

static uint16_t shape_in[2] = {1,784};
static uint16_t shape_out[2] = {1,10};
static ailayer_input_q7_t input_layer;
static ailayer_dense_q7_t dense[5];
static ailayer_relu_q7_t relu[4];
static ailayer_softmax_q7_t softmax;
static aimodel_t model;
static int8_t input_q7[784];
/* Aligned workspace. No malloc; parameter arrays stay in read-only memory. */
static uint32_t workspace[1024];
static uint32_t required;
static int initialized;

uint32_t mnist_aifes_memory_required(void) { return required; }

int mnist_aifes_init(void)
{
    const uint16_t neurons[5]={512,256,128,64,10};
    initialized=0;
    memset(&input_layer,0,sizeof(input_layer));
    memset(dense,0,sizeof(dense));
    memset(relu,0,sizeof(relu));
    memset(&softmax,0,sizeof(softmax));
    memset(&model,0,sizeof(model));
    input_layer.input_dim=2;
    input_layer.input_shape=shape_in;
    model.input_layer=ailayer_input_q7_default(&input_layer);
    model.input_layer->result.tensor_params=&result_params[0];
    ailayer_t *x=model.input_layer;
    for(unsigned i=0;i<5;i++) {
        dense[i].neurons=neurons[i];
        x=ailayer_dense_wt_q7_default(&dense[i],x);
        dense[i].weights.data=(void *)weights[i];
        dense[i].bias.data=(void *)biases[i];
        dense[i].weights.tensor_params=&weight_params[i];
        dense[i].bias.tensor_params=&bias_params[i];
        x->result.tensor_params=&result_params[i+1];
        if(i<4) x=ailayer_relu_q7_default(&relu[i],x);
    }
    model.output_layer=ailayer_softmax_q7_default(&softmax,x);
    if(aialgo_compile_model(&model)!=0) return 1;
    required=aialgo_sizeof_inference_memory(&model);
    if(required>sizeof(workspace)) return 2;
    if(aialgo_schedule_inference_memory(&model,workspace,required)!=0) return 3;
    for(x=model.input_layer;x;x=x->output_layer)
        if(x->calc_result_tensor_params) x->calc_result_tensor_params(x);
    initialized=1;
    return 0;
}

int mnist_aifes_predict(const float *input, int8_t *output)
{
    if(!initialized || !input || !output) return 1;
    const float scale=(float)(1u<<result_params[0].shift);
    for(unsigned i=0;i<784;i++) {
        /* Fail explicitly if the old uncentered host script is used. */
        if(!isfinite(input[i]) || input[i]<-0.50001f || input[i]>0.50001f) return 4;
        /* Round-to-nearest, ties away from zero, matching C roundf. */
        int q=(int)roundf(input[i]*scale)+result_params[0].zero_point;
        if(q < -128) q=-128;
        if(q > 127) q=127;
        input_q7[i]=(int8_t)q;
    }
    aitensor_t in={.dtype=aiq7,.dim=2,.shape=shape_in,
                  .tensor_params=&result_params[0],.data=input_q7};
    aitensor_t out={.dtype=aiq7,.dim=2,.shape=shape_out,
                   .tensor_params=model.output_layer->result.tensor_params,.data=output};
    return aialgo_inference_model(&model,&in,&out);
}
