/**
  ******************************************************************************
  * @file    minst_data_params.h
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-09-11T09:45:24+0200
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#ifndef MINST_DATA_PARAMS_H
#define MINST_DATA_PARAMS_H

#include "ai_platform.h"

/*
#define AI_MINST_DATA_WEIGHTS_PARAMS \
  (AI_HANDLE_PTR(&ai_minst_data_weights_params[1]))
*/

#define AI_MINST_DATA_CONFIG               (NULL)


#define AI_MINST_DATA_ACTIVATIONS_SIZES \
  { 3868, }
#define AI_MINST_DATA_ACTIVATIONS_SIZE     (3868)
#define AI_MINST_DATA_ACTIVATIONS_COUNT    (1)
#define AI_MINST_DATA_ACTIVATION_1_SIZE    (3868)



#define AI_MINST_DATA_WEIGHTS_SIZES \
  { 25912, }
#define AI_MINST_DATA_WEIGHTS_SIZE         (25912)
#define AI_MINST_DATA_WEIGHTS_COUNT        (1)
#define AI_MINST_DATA_WEIGHT_1_SIZE        (25912)



#define AI_MINST_DATA_ACTIVATIONS_TABLE_GET() \
  (&g_minst_activations_table[1])

extern ai_handle g_minst_activations_table[1 + 2];



#define AI_MINST_DATA_WEIGHTS_TABLE_GET() \
  (&g_minst_weights_table[1])

extern ai_handle g_minst_weights_table[1 + 2];


#endif    /* MINST_DATA_PARAMS_H */
