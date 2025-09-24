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
uint32_t elapsed_cycles;
// Lab 2 - phase accumulation
float32_t current_phase = 0;
// Lab 2 - Difference equation / Impulse Response
float32_t w0;
float32_t sin_w0;
float32_t cos_w0;
float32_t x[3] = {1,0,0}; // x[0] is the signal x[n], x[1] is the signal x[n-1], etc
float32_t y[3] = {0,0,0}; // Same for y ^
// Lab 2 - Lookup table: sample-by-sample
//uint32_t table_len = 16;	// For 1kHz
uint32_t table_len = 400;	// For 440Hz
//int16_t table[16];	// For 1kHz
int16_t table[400];		// For 440Hz
uint32_t i_table = 0;

/*
This function will be called once before beginning the main program loop.
This is the best place to build a lookup table.
*/
void lab_init(int16_t* output_buffer)
{
//	w0 = M_PI / 8; // for f0 = 1kHz, fs = 16kHz
//	w0 = 15 * M_PI / 8; // for f0 = 15kHz, fs = 16kHz
	w0 = (2.0 * M_PI) * (440.0 / 16000.0); // for f0 = 440Hz, fs = 16kHz
	float32_t amplitude;
	for (uint32_t n = 0; n < table_len; n+=1)
	{
	    amplitude = arm_sin_f32(n * w0); // sin amplitude as a float within [-1, 1]
	    table[n] = (int16_t) (OUTPUT_SCALE_FACTOR * amplitude);
	}

	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE; i_sample+=1)
	{
	    i_table = (i_sample/2) % table_len;
	    output_buffer[i_sample] = table[i_table]; //left
	    i_sample += 1;
	    output_buffer[i_sample] = 0; //right
	}
	return;
}
// Lab 2 - Lookup table: sample-by-sample
//void lab_init(int16_t* output_buffer)
//{
//	w0 = M_PI / 8; // for f0 = 1kHz, fs = 16kHz
////w0 = 15 * M_PI / 8; // for f0 = 15kHz, fs = 16kHz
//	float32_t amplitude;
//	for (uint32_t n = 0; n < 16; n+=1)
//	{
//	    amplitude = arm_sin_f32(n * w0); // sin amplitude as a float within [-1, 1]
//	    table[n] = (int16_t) (OUTPUT_SCALE_FACTOR * amplitude);
//	}
//	return;
//}

// Lab 2 - Lookup table : DMA
//void lab_init(int16_t* output_buffer)
//{
////	w0 = M_PI / 8; // for f0 = 1kHz, fs = 16kHz
////	w0 = 15 * M_PI / 8; // for f0 = 15kHz, fs = 16kHz
//	w0 = (2.0 * M_PI) * (440.0 / 16000.0); // for f0 = 440Hz, fs = 16kHz
//	float32_t amplitude;
//	for (uint32_t n = 0; n < table_len; n+=1)
//	{
//	    amplitude = arm_sin_f32(n * w0); // sin amplitude as a float within [-1, 1]
//	    table[n] = (int16_t) (OUTPUT_SCALE_FACTOR * amplitude);
//	}
//
//	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE; i_sample+=1)
//	{
//	    i_table = (i_sample/2) % table_len;
//	    output_buffer[i_sample] = table[i_table]; //left
//	    i_sample += 1;
//	    output_buffer[i_sample] = 0; //right
//	}
//	return;
//}


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
    int16_t output_sample;

    output_sample = table[i_table];
    i_table += 1;
    if (i_table == 16)
    {
    	i_table = 0;
    }

	elapsed_cycles = toc();
	printf("Elapsed Cycles: %d\n", elapsed_cycles);
	return output_sample;
}

// Lab 1
//int16_t process_left_sample(int16_t input_sample)
//{
//	tic();
//
//	float scaled_input = input_sample * INPUT_SCALE_FACTOR;
//	scaled_input *= scaled_input;
//  output_sample = (int16_t)(scaled_input * OUTPUT_SCALE_FACTOR);
//  output_sample = input_sample;
//
//	elapsed_cycles = toc();
//	printf("Elapsed Cycles: %d\n", elapsed_cycles);
//	return output_sample;
//}

// Lab 2 - phase accumulation
//int16_t process_left_sample(int16_t input_sample)
//{
//	tic();
//
//  output_sample = OUTPUT_SCALE_FACTOR * arm_sin_f32(current_phase);
//	current_phase += w0;
//	if (current_phase > 2 * M_PI) current_phase -= 2 * M_PI;
//
//	elapsed_cycles = toc();
//	printf("Elapsed Cycles: %d\n", elapsed_cycles);
//	return output_sample;
//}

// Lab 2 - Difference equation / Impulse Response
//int16_t process_left_sample(int16_t input_sample)
//{
//	tic();
//	float32_t output_sample;
//
//	y[0] = 2 * cos_w0 * y[1] - y[2] + sin_w0 * x[1]; // Calculate y[n]
//
//	// Update y[n-2, n-1, n] and x[n-2. n-1, n]
//	y[2] = y[1]; y[1] = y[0];
//	x[2] = x[1]; x[1] = x[0]; x[0] = 0;
//
//	output_sample = y[0] * OUTPUT_SCALE_FACTOR;
//
//	elapsed_cycles = toc();
//	printf("Elapsed Cycles: %d\n", elapsed_cycles);
//	return (int16_t) output_sample;
//}

// Lab 2 - Lookup table: sample-by-sample
//int16_t process_left_sample(int16_t input_sample)
//{
//	tic();
//  int16_t output_sample;
//
//  output_sample = table[i_table];
//	i_table += 1;
//	if (i_table == 16)
//	{
//		i_table = 0;
//	}
//
//	elapsed_cycles = toc();
//	printf("Elapsed Cycles: %d\n", elapsed_cycles);
//	return output_sample;
//}

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
