#include <iostream>

#include "MultiJetBaseline.h"
#include "parameters.h"


void MultiJetBaseline(
    input_t inputs[N_INPUT_1_1*N_INPUT_2_1],
    qDense_out_reg_result_t layer22_out[N_LAYER_22], layer24_t layer24_out[N_LAYER_20]
) {

    // hls-fpga-machine-learning insert IO
    #pragma HLS ARRAY_RESHAPE variable=inputs complete dim=0
    #pragma HLS ARRAY_PARTITION variable=layer22_out complete dim=0
    #pragma HLS ARRAY_PARTITION variable=layer24_out complete dim=0
    #pragma HLS INTERFACE ap_vld port=inputs,layer22_out,layer24_out 
    #pragma HLS DATAFLOW 

    // hls-fpga-machine-learning insert load weights
#ifdef __HLS4ML_LOAD_TXT_WEIGHTS__
    static bool loaded_weights = false;
    if (!loaded_weights) {
        nnet::load_weights_from_txt<batchnorm_inputs_scale_t, 21>(s2, "s2.txt");
        nnet::load_weights_from_txt<batchnorm_inputs_bias_t, 21>(b2, "b2.txt");
        nnet::load_weights_from_txt<qdense_phi_1_weight_t, 672>(w28, "w28.txt");
        nnet::load_weights_from_txt<qdense_phi_1_bias_t, 32>(b28, "b28.txt");
        nnet::load_weights_from_txt<qdense_phi_2_weight_t, 1024>(w29, "w29.txt");
        nnet::load_weights_from_txt<qdense_phi_2_bias_t, 32>(b29, "b29.txt");
        nnet::load_weights_from_txt<weight11_t, 1024>(w11, "w11.txt");
        nnet::load_weights_from_txt<bias11_t, 32>(b11, "b11.txt");
        nnet::load_weights_from_txt<weight14_t, 1024>(w14, "w14.txt");
        nnet::load_weights_from_txt<bias14_t, 32>(b14, "b14.txt");
        nnet::load_weights_from_txt<weight16_t, 1024>(w16, "w16.txt");
        nnet::load_weights_from_txt<bias16_t, 32>(b16, "b16.txt");
        nnet::load_weights_from_txt<weight20_t, 256>(w20, "w20.txt");
        nnet::load_weights_from_txt<bias20_t, 8>(b20, "b20.txt");
        nnet::load_weights_from_txt<weight22_t, 32>(w22, "w22.txt");
        nnet::load_weights_from_txt<bias22_t, 1>(b22, "b22.txt");
        loaded_weights = true;    }
#endif
    // ****************************************
    // NETWORK INSTANTIATION
    // ****************************************

    // hls-fpga-machine-learning insert layers

    batchnorm_inputs_result_t layer2_out[N_INPUT_1_1*N_INPUT_2_1];
    #pragma HLS ARRAY_PARTITION variable=layer2_out complete dim=0
    nnet::normalize<input_t, batchnorm_inputs_result_t, config2>(inputs, layer2_out, s2, b2); // batchnorm_inputs

    qDense_phi_1_result_t layer28_out[N_OUTPUTS_28*N_FILT_28];
    #pragma HLS ARRAY_PARTITION variable=layer28_out complete dim=0
    nnet::pointwise_conv_1d_cl<batchnorm_inputs_result_t, qDense_phi_1_result_t, config28>(layer2_out, layer28_out, w28, b28); // qDense_phi_1

    layer5_t layer5_out[N_LAYER_1_3*N_LAYER_2_3];
    #pragma HLS ARRAY_PARTITION variable=layer5_out complete dim=0
    nnet::relu<qDense_phi_1_result_t, layer5_t, relu_config5>(layer28_out, layer5_out); // qActivation_phi_1

    qDense_phi_2_result_t layer29_out[N_OUTPUTS_29*N_FILT_29];
    #pragma HLS ARRAY_PARTITION variable=layer29_out complete dim=0
    nnet::pointwise_conv_1d_cl<layer5_t, qDense_phi_2_result_t, config29>(layer5_out, layer29_out, w29, b29); // qDense_phi_2

    layer8_t layer8_out[N_LAYER_1_6*N_LAYER_2_6];
    #pragma HLS ARRAY_PARTITION variable=layer8_out complete dim=0
    nnet::relu<qDense_phi_2_result_t, layer8_t, relu_config8>(layer29_out, layer8_out); // qActivation_phi_2

    layer9_t layer9_out[N_LAYER_1_6*N_LAYER_2_6];
    #pragma HLS ARRAY_PARTITION variable=layer9_out complete dim=0
    nnet::linear<layer8_t, layer9_t, linear_config9>(layer8_out, layer9_out); // qActivationForPool

    layer10_t layer10_out[N_FILT_10];
    #pragma HLS ARRAY_PARTITION variable=layer10_out complete dim=0
    nnet::global_pooling1d_cl<layer9_t, layer10_t, config10>(layer9_out, layer10_out); // avgpool

    qDense_rho_1_result_t layer11_out[N_LAYER_11];
    #pragma HLS ARRAY_PARTITION variable=layer11_out complete dim=0
    nnet::dense<layer10_t, qDense_rho_1_result_t, config11>(layer10_out, layer11_out, w11, b11); // qDense_rho_1

    layer13_t layer13_out[N_LAYER_11];
    #pragma HLS ARRAY_PARTITION variable=layer13_out complete dim=0
    nnet::relu<qDense_rho_1_result_t, layer13_t, relu_config13>(layer11_out, layer13_out); // qActivation_rho_1

    qDense_2_class_result_t layer14_out[N_LAYER_14];
    #pragma HLS ARRAY_PARTITION variable=layer14_out complete dim=0
    nnet::dense<layer13_t, qDense_2_class_result_t, config14>(layer13_out, layer14_out, w14, b14); // qDense_2_class

    qDense_2_reg_result_t layer16_out[N_LAYER_16];
    #pragma HLS ARRAY_PARTITION variable=layer16_out complete dim=0
    nnet::dense<layer13_t, qDense_2_reg_result_t, config16>(layer13_out, layer16_out, w16, b16); // qDense_2_reg

    layer18_t layer18_out[N_LAYER_14];
    #pragma HLS ARRAY_PARTITION variable=layer18_out complete dim=0
    nnet::relu<qDense_2_class_result_t, layer18_t, relu_config18>(layer14_out, layer18_out); // qActivation_2_class

    layer19_t layer19_out[N_LAYER_16];
    #pragma HLS ARRAY_PARTITION variable=layer19_out complete dim=0
    nnet::relu<qDense_2_reg_result_t, layer19_t, relu_config19>(layer16_out, layer19_out); // qActivation_2_reg

    qDense_out_class_result_t layer20_out[N_LAYER_20];
    #pragma HLS ARRAY_PARTITION variable=layer20_out complete dim=0
    nnet::dense<layer18_t, qDense_out_class_result_t, config20>(layer18_out, layer20_out, w20, b20); // qDense_out_class

    nnet::dense<layer19_t, qDense_out_reg_result_t, config22>(layer19_out, layer22_out, w22, b22); // qDense_out_reg

    nnet::softmax<qDense_out_class_result_t, layer24_t, softmax_config24>(layer20_out, layer24_out); // output_class


    std::cout << "Layer 2 out: ";
    for (int i = 0; i < N_INPUT_1_1*N_INPUT_2_1; ++i ){
        std::cout <<  layer2_out[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "===========" << std::endl; 

    std::cout << "Layer 19 out: ";
    for (int i = 0; i <N_LAYER_16; ++i ){
        std::cout <<  layer19_out[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "===========" << std::endl; 

    std::cout << "Layer 20 out: ";
    for (int i = 0; i <N_LAYER_20; ++i ){
        std::cout <<  layer20_out[i] << ", ";
    }
    std::cout << std::endl;
    std::cout << "===========" << std::endl; 

}

