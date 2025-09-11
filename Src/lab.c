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
float32_t phase;
float32_t x[3] = {1,0,0};
float32_t y[3] = {0,0,0};

float32_t w0;
float32_t coeff1;
float32_t coeff2;
/*
This function will be called once before beginning the main program loop.
This is the best place to build a lookup table.
*/
void lab_init(int16_t* output_buffer)
{
	w0 = M_PI / 8.0;
	coeff1 = 2.0 * arm_cos_f32(w0);
	coeff2 = arm_sin_f32(w0);
//	coeff2 = (arm_cos_f32(w0));
	return;
}

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
// Lab 2 Week 1: Math libarary & phase accumulation
//int16_t process_left_sample(int16_t input_sample)
//{
//	tic();
//	int16_t output_sample;
//	if (phase > 2 * M_PI)
//		phase -= 2 * M_PI;
//	output_sample = (int16_t) (OUTPUT_SCALE_FACTOR * arm_sin_f32(phase));
//	phase += M_PI / 8.0;
//	elapsed_cycles = toc();
//	return output_sample;
//}

// Lab 2 Week 1: Difference equation
int16_t process_left_sample(int16_t input_sample)
{
	tic();
	int16_t output_sample;

	output_sample = y[0] * OUTPUT_SCALE_FACTOR;

	float32_t y_new = coeff1 * y[1] - y[2] + coeff2 * x[1];
//	float32_t y_new = coeff1 * y[1] - y[2] + x[0] - coeff2 * x[1];
	float32_t x_new = 0.0;

	//y = {y_new, y[0], y[1]};
	y[2] = y[1];
	y[1] = y_new;
	y[0] = y_new;
	//x = {x_new, x[0], x[1]};
	x[2] = x[1];
	x[1] = x_new;
	x[0] = x_new;

	elapsed_cycles = toc();
	return output_sample;
}

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
