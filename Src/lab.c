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
#define FilterLen 7

uint32_t elapsed_cycles;
// Lab 3 week 2
const float32_t fore[FilterLen] = {0.0692585012840081, -0.00562043209908955, -0.188713717395346, 0.0, 0.188713717395346, 0.00562043209908955, -0.0692585012840081};
const float32_t back[FilterLen] = {0.0, -1.33160257778053, 1.73605396252057, -1.46788691694819, 1.43145085327469, -0.696108745296826, 0.383105162659562};
float32_t x[FilterLen] = {0.0};
float32_t y[FilterLen] = {0.0};
// Lab 3 week 3
//float32_t B[3][3] = {
//		{1.000000,1.889818,1.000000},
//		{1.000000,-1.970969,1.000000},
//		{1.000000,0.000000,-1.000000}
//};
//float32_t A[3][3] = {
//		{1.000000,-1.447046,0.862778},
//		{1.000000,0.622267,0.802149},
//		{1.000000,-0.506824,0.553559}
//};
//float32_t G[4] = {0.340877,0.340877,0.596044,1.000000};
//float32_t Y[3][3] = {0};
//float32_t X[3][3] = {0};

/*
This function will be called once before beginning the main program loop.
This is the best place to build a lookup table.
*/
void lab_init(int16_t* output_buffer)
{
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
// Circular buffer code
typedef struct {
    float32_t *buffer;   // pointer to array
    int32_t length;      // number of valid elements currently stored
    int32_t size;        // total capacity of the buffer
    int32_t pos;         // current position index
} CircularBuffer;

int32_t append(CircularBuffer *cb, float32_t new_val) {
    cb->pos -= 1;
    if (cb->pos < 0) {
        cb->pos = cb->size - 1;
    }
    cb->buffer[cb->pos] = new_val;

    // update length (cannot exceed size)
    if (cb->length < cb->size) {
        cb->length++;
    }
    return cb->pos;  // return updated position
}

void pop(CircularBuffer *cb) {
    if (cb->length <= 0) return;  // nothing to delete

    cb->pos += 1;
    if (cb->pos >= cb->size) {
        cb->pos = 0;
    }
    cb->length -= 1;
}

float32_t read(CircularBuffer *cb, int32_t i) {
    if ((cb->pos < 0) || (cb->pos >= cb->size)) {
        while (1) { ; } // safety trap
    }

    int32_t pos_r = cb->pos + i;
    if (pos_r >= cb->size) {
        pos_r -= cb->size;
    }
    return cb->buffer[pos_r];
}

CircularBuffer circX = {x, 0, FilterLen, FilterLen - 1};
CircularBuffer circY = {y, 0, FilterLen, FilterLen - 1};

// Lab 3 week 3: Circular buffer of literal IIR implementations
int16_t process_left_sample(int16_t input_sample)
{
	tic();
	int16_t output_sample;

	// Get new input
	append(&circX, input_sample * INPUT_SCALE_FACTOR);
	append(&circY, 0.0);
	// Calculate new output
	float32_t temp = 0.0f;
	for (int i = 0; i < FilterLen; i++)
		temp += read(&circX, i) * fore[i] - read(&circY, i) * back[i];

	pop(&circY);
	append(&circY, temp);

	output_sample = temp * OUTPUT_SCALE_FACTOR;

	elapsed_cycles = toc();
	return output_sample;
}
// Lab 3 week 2
//int16_t process_left_sample(int16_t input_sample)
//{
//	tic();
//	int16_t output_sample;
//
//	// Shift y and x
//	for (int i = FilterLen - 1; i >= 1; i--)
//		y[i] = y[i - 1];
//	for (int i = FilterLen - 1; i >= 1; i--)
//		x[i] = x[i - 1];
//
//	// Put in new input, zero init output
//	x[0] = input_sample * INPUT_SCALE_FACTOR;
//	y[0] = 0.0;
//
//	// Calculate new output
//	float32_t temp = 0.0f;
//	for (int i = 0; i < FilterLen; i++)
//		temp += x[i] * fore[i] - y[i] * back[i];
//
//	y[0] = temp;
//
//	output_sample = temp * OUTPUT_SCALE_FACTOR;
//
//	elapsed_cycles = toc();
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
