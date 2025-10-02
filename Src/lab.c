/*
This file contains the functions you will modify in order to complete the labs.
These functions allow you to perform DSP on a per-frame or per-sample basis.
When processing on a per-sample basis, there are separate functions for left and right channels.
When processing on a per-frame basis, the left and right channels are interleaved.
The sample rate and frame size can be modified in lab.h.
You can also configure which of the four functions are active in lab.h
*/

#include "main.h"
#include "lab.h"

//These functions allow estimation of the number of elapsed clock cycles
extern void tic(void);
extern uint32_t toc(void);

//variables used for the spectrum visualization
extern arm_rfft_fast_instance_f32 fft_inst;
extern float32_t fft_in[FRAME_SIZE/4];
extern float32_t fft_out[FRAME_SIZE/4];
extern float32_t fft_mag[FRAME_SIZE/8];

//declare variables local to this file
#define filterOrder 6 // filter order N
#define numCoefficients 7 // N + 1 coefficients
uint32_t elapsed_cycles;
float32_t current_phase = 0;
float32_t w0;
float32_t sin_w0;
float32_t cos_w0;
float32_t x[numCoefficients] = {0}; // x[0] is the signal x[n], x[1] is the signal x[n-1], etc
float32_t y[numCoefficients] = {0};
int16_t table[400];
int16_t output_buffer[8192];
uint32_t tableIndex = 0;
int8_t pos = 0;
const float32_t feedFwd[numCoefficients] = {0.069259,-0.005620,-0.188714,0.000000,0.188714,0.005620,-0.069259};
const float32_t feedBack[numCoefficients] = {1.000000,-1.331603,1.736054,-1.467887,1.431451,-0.696109,0.383105};


// below is lab 3 week 1 coeffs
//const float32_t b[numCoefficients] = {0.003461,0.003753,-0.001660,0.016044,0.011602,-0.055832,-0.065883,0.051136,0.093381,0.001214,0.010319,0.050610,-0.145323,-0.257413,0.094104,0.385833,0.094104,-0.257413,-0.145323,0.050610,0.010319,0.001214,0.093381,0.051136,-0.065883,-0.055832,0.011602,0.016044,-0.001660,0.003753,0.003461};
// below is lab 3 week 1 DMA buffer stuff
//float32_t filter_in[FRAME_SIZE/4] = {0};
//float32_t filter_out[FRAME_SIZE/4] = {0};
//float32_t state[31+(FRAME_SIZE/4)-1] = {0};
//arm_fir_instance_f32 filter_instance;
// Coefficients of b range from b[0] to b[30]


/*
This function will be called once before beginning the main program loop.
This is the best place to build a lookup table.
*/
void lab_init(int16_t* output_buffer)
{
	//arm_fir_init_f32(&filter_instance, 31, b, state, FRAME_SIZE/4); // LAB 3 week 1, frame based
	//w0 = M_PI / 8; // for f0 = 1kHz, fs = 16kHz
	//w0 = 15 * M_PI / 8; // for f0 = 15kHz, fs = 16kHz
	return;
}

// Lab 2: DMA
//w0 = M_PI * 11 / 200;
//float32_t amplitude;
//for (uint32_t n = 0; n < 400; n+=1)
//{
//    amplitude = arm_sin_f32(n * w0); // sin amplitude as a float within [-1, 1]
//    table[n] = (int16_t) (OUTPUT_SCALE_FACTOR * amplitude);
//}
//
//for (uint32_t i_sample = 0; i_sample < FRAME_SIZE; i_sample+=1)
//{
//    tableIndex = (i_sample/2) % 400;
//    output_buffer[i_sample] = table[tableIndex]; //left
//    i_sample += 1;
//    output_buffer[i_sample] = 0; //right
//}
//return;

/*
This function will be called each time a complete frame of data is recorded.
Modify this function as needed.
Default behavior:
	1. Deinterleave the left and right channels
	2. Combine the two channels (by addition) into one signal
	3. Save the result to the fft_in buffer which will be used for the display
	4. The original audio buffer is left unchanged (passthrough)
*/
void process_input_buffer(int16_t* input_buffer)
{
	// below is lab 3 week 1 DMA stuff
//	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=2)
//	{
//	    filter_in[i_sample/2] = ((float32_t)input_buffer[i_sample])*INPUT_SCALE_FACTOR;
//	}
//	arm_fir_f32(&filter_instance, filter_in, filter_out, FRAME_SIZE/4);
//	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
//	{
//	     input_buffer[i_sample] = OUTPUT_SCALE_FACTOR*filter_out[i_sample/2];
//	     i_sample+=1;
//	     input_buffer[i_sample] = 0;
//	}


	int16_t left_sample;
	int16_t right_sample;
	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
	{
		left_sample = input_buffer[i_sample];
		i_sample +=1;
		right_sample = input_buffer[i_sample];
		fft_in[i_sample/2] =  (((float32_t) left_sample) + ((float32_t) right_sample))/2;
	}
	arm_rfft_fast_f32(&fft_inst, fft_in, fft_out, 0);
	arm_cmplx_mag_f32(fft_out, fft_mag, FRAME_SIZE/8);
	return;
}

/*
This function provides access to each individual sample that is incoming on the left channel.
The returned value will be sent to the left channel DAC.
Default behavior:
	1. Copy input to output without modification (passthrough)
	2. Estimate the number of cycles that have elapsed during the function call
*/
int16_t process_left_sample(int16_t input_sample)
{
	tic();
	x[0] = ((float32_t) input_sample) * INPUT_SCALE_FACTOR;
	float32_t yN = 0;
	for (uint8_t i = 0; i < numCoefficients; i++) { // Calculate the part of y[n] that comes from feed forward
			yN += feedFwd[i] * x[i];
	}

	for (uint8_t i = filterOrder; i > 0 ; i--) { // Shifts x[n-1] -> x[n] for indices from 30 to 0. LINEAR BUFFER. Does it BW cuz it's easier
	    	x[i] = x[i - 1];
	}

	for (uint8_t i = 1; i < numCoefficients; i++) { // Calculate y[n]
			yN -= feedBack[i] * y[i];
	}

	y[0] = yN;
	for (uint8_t i = filterOrder; i > 0; i--) { // Shifts x[n-1] -> x[n] for indices from 30 to 0. LINEAR BUFFER. Does it BW cuz it's easier
	    	y[i] = y[i - 1];
	}

	int16_t output_sample = (int16_t) (yN * OUTPUT_SCALE_FACTOR);
	elapsed_cycles = toc();
	printf("Elapsed Cycles: %d\n", elapsed_cycles);
	return output_sample;
}

// Lab 1
//	float scaled_input = input_sample * INPUT_SCALE_FACTOR;
//	scaled_input *= scaled_input;
//  output_sample = (int16_t)(scaled_input * OUTPUT_SCALE_FACTOR);

// Lab 2 - phase accumulation
//  output_sample = OUTPUT_SCALE_FACTOR * arm_sin_f32(current_phase);
//	current_phase += w0;
//	if (current_phase > 2 * M_PI) current_phase -= 2 * M_PI;

// Lab 2 - Difference equation / Impulse Response
//tic();
//float32_t output_sample;
//
//y[0] = 2 * cos_w0 * y[1] - y[2] + sin_w0 * x[1]; // Calculate y[n]
//
//// Update y[n-2, n-1, n] and x[n-2. n-1, n]
//y[2] = y[1]; y[1] = y[0];
//x[2] = x[1]; x[1] = x[0]; x[0] = 0;
//
//output_sample = y[0] * OUTPUT_SCALE_FACTOR;
//
//elapsed_cycles = toc();
//printf("Elapsed Cycles: %d\n", elapsed_cycles);
//return (int16_t) output_sample;

// Lab 2: DMA
//tic();
//int16_t output_sample;
//output_sample = table[tableIndex];
//tableIndex += 1;
//if (tableIndex == 16) tableIndex = 0;
//elapsed_cycles = toc();
//printf("Elapsed Cycles: %d\n", elapsed_cycles);
//return output_sample;

// Lab 3: Linear Buffer
//tic();
//x[0] = ((float32_t) input_sample) * INPUT_SCALE_FACTOR;
//float32_t yN = 0;
//for (uint8_t i = 0; i < 31; i++) { // Calculate y[n]
//		yN += b[i] * x[i];
//}
//
//for (uint8_t i = 30; i > 0 ; i--) { // Shifts x[n-1] -> x[n] for indices from 30 to 0. LINEAR BUFFER. Does it BW cuz it's easier
//    	x[i] = x[i - 1];
//}
//int16_t output_sample = (int16_t) (yN * OUTPUT_SCALE_FACTOR);
//elapsed_cycles = toc();
//printf("Elapsed Cycles: %d\n", elapsed_cycles);
//return output_sample;

// Lab 3: Circular Buffer
//	tic();
//	pos--;
//	if (pos == -1) pos = 30;
//	x[pos] = ((float32_t) input_sample) * INPUT_SCALE_FACTOR;
//	float32_t yN = 0;
//	for (uint8_t i = 0; i < 31; i++) { // Calculate y[n]
//			int8_t j = (pos + i) % 31;
//			yN += b[i] * x[j];
//	}
//
//	int16_t output_sample = (int16_t) (yN * OUTPUT_SCALE_FACTOR);
//	elapsed_cycles = toc();
//	printf("Elapsed Cycles: %d\n", elapsed_cycles);
//	return output_sample;

/*
This function provides access to each individual sample that is incoming on the left channel.
The returned value will be sent to the right channel DAC.
Default behavior:
	1. Copy input to output without modification (passthrough)
	2. Estimate the number of cycles that have elapsed during the function call
*/
int16_t process_right_sample(int16_t input_sample)
{
	tic();
	int16_t output_sample;
	output_sample = input_sample;
	elapsed_cycles = toc();
	return output_sample;
}

/*
This function provides another opportunity to access the frame of data
The default behavior is to leave the buffer unchanged (passthrough)
The buffer you see here will have any changes that occurred to the signal due to:
	1. the process_input_buffer function
	2. the process_left_sample and process_right_sample functions
*/
void process_output_buffer(int16_t* output_buffer)
{
	return;
}
