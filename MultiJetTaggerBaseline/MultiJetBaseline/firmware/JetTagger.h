#ifndef JETTAGGER_H_
#define JETTAGGER_H_

#include "ap_fixed.h"
#include "ap_int.h"
#include "hls_stream.h"

#include "defines.h"


// Prototype of top level function for C-synthesis
void JetTagger(
    input_t inputs[N_INPUT_1_1*N_INPUT_2_1],
    qDense_out_reg_result_t layer22_out[N_LAYER_22], layer24_t layer24_out[N_LAYER_20]
);


#endif
