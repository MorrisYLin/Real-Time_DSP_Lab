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
extern float32_t fft_in[FRAME_SIZE / 4];
extern float32_t fft_out[FRAME_SIZE / 4];
extern float32_t fft_mag[FRAME_SIZE / 8];

//declare variables local to this file
//uint32_t elapsed_cycles;
//uint32_t s = 1;
//uint32_t PN[513] = {0};
//uint32_t data_stream[513] = {0};
float32_t pulse_shaping_coeffs[64] = { 0.000000, 0.000163, -0.000000, -0.000576,
		-0.001624, -0.003164, -0.005162, -0.007522, -0.010081, -0.012608,
		-0.014806, -0.016326, -0.016777, -0.015752, -0.012850, -0.007702,
		0.000000, 0.010474, 0.023826, 0.040029, 0.058914, 0.080159, 0.103297,
		0.127730, 0.152748, 0.177557, 0.201320, 0.223195, 0.242375, 0.258132,
		0.269854, 0.277080, 0.279521, 0.277080, 0.269854, 0.258132, 0.242375,
		0.223195, 0.201320, 0.177557, 0.152748, 0.127730, 0.103297, 0.080159,
		0.058914, 0.040029, 0.023826, 0.010474, 0.000000, -0.007702, -0.012850,
		-0.015752, -0.016777, -0.016326, -0.014806, -0.012608, -0.010081,
		-0.007522, -0.005162, -0.003164, -0.001624, -0.000576, -0.000000,
		0.000163 };
//arm_fir_interpolate_instance_f32 filter_instance;
//uint8_t upsampling_factor = 16;
//uint16_t num_taps = 64;
//float32_t state[(FRAME_SIZE/4) + 4 - 1] = {0};
//float32_t filter_in[FRAME_SIZE/64] = {0};
//float32_t filter_out[FRAME_SIZE/4] = {0};
//float32_t cos_lut[4] = {1,0,-1,0};
//uint32_t i_lut = 0;
//uint32_t i_bit = 0;
//uint32_t i_word = 0;
uint32_t tree[512] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x400, 0x0, 0x0, 0x0, 0x1c600, 0x0, 0x0, 0x0, 0x1e100, 0x0, 0x0, 0x0,
		0x1e000, 0x0, 0x0, 0x0, 0x1f080, 0x0, 0x0, 0x0, 0x3880, 0x0, 0x0, 0x0,
		0x1880, 0x0, 0x0, 0x0, 0xc80, 0x0, 0x0, 0x0, 0x680, 0x0, 0x0, 0x0,
		0x680, 0x0, 0x0, 0x0, 0x4680, 0x0, 0x0, 0x0, 0x6680, 0x0, 0x0, 0x0,
		0x680, 0x0, 0x0, 0x0, 0xc0e80, 0x0, 0x0, 0x0, 0x40e80, 0x0, 0x0, 0x0,
		0x20680, 0x0, 0x0, 0x0, 0x10680, 0x0, 0x0, 0x0, 0x8680, 0x0, 0x0, 0x0,
		0x680, 0x0, 0x0, 0x0, 0x680, 0x0, 0x0, 0x0, 0x680, 0x0, 0x0, 0x0, 0x680,
		0x0, 0x0, 0x0, 0x600680, 0x0, 0x0, 0x0, 0x104680, 0x0, 0x0, 0x0,
		0x82680, 0x0, 0x0, 0x0, 0x41680, 0x0, 0x1c00000, 0x0, 0x20e80, 0x0,
		0x1e00000, 0x0, 0x10680, 0x0, 0x3e00000, 0x0, 0x8680, 0x30000000,
		0x83c00004, 0x1, 0x4680, 0x20000000, 0x8600200c, 0x1, 0x680, 0x40000000,
		0xc002008, 0x0, 0x680, 0x8f000000, 0x18404010, 0x0, 0x680, 0xff000000,
		0x1c408000, 0x0, 0x380, 0xff000000, 0x9801038c, 0x0, 0x802180,
		0x86000000, 0x78004399, 0x0, 0x4010c0, 0xc00000, 0x30004793, 0x0,
		0x200860, 0x8c00000, 0x63818fa6, 0x21, 0x100460, 0x19008000, 0x67011c2c,
		0x71, 0x80260, 0x22018c00, 0x66081838, 0xd2, 0x40160, 0xfff20e00,
		0x6c0c1f3f, 0xf4, 0x200e0, 0x21b00, 0x78021a31, 0x68, 0x810c60,
		0x80009e00, 0x7f82d030, 0x22, 0x38c08c60, 0x9c01bc00, 0x60807030, 0x62,
		0x3c004260, 0x3c626000, 0xc0383230, 0x6c, 0x3c202160, 0x3c40e040,
		0x87f83230, 0xc9, 0x3c1020e0, 0x7880c040, 0xff86260, 0x183, 0x6082060,
		0xe100cc80, 0x1c39c1ec, 0x18e, 0x3ff2060, 0xfe039900, 0xf801818d, 0x18f,
		0x1ff9060, 0x80073200, 0xf70f0303, 0x19f, 0x1c860, 0x1e60180, 0x11e0603,
		0x1b0, 0x26463, 0x20c0180, 0x300c06, 0x800001fe, 0x4363f, 0xfffffc00,
		0x7ffffff, 0xe00001c8, 0x201f9f, 0xfffffc00, 0x4fffffff, 0xf0000184,
		0x400fc3, 0x3800180, 0xd8030002, 0x38000383, 0x18000e0, 0x11fc0100,
		0x3fc1f841, 0x1c000670, 0x1801e60, 0xb007c200, 0x6200ffe0, 0x8ffffe00,
		0x21ff, 0x4003e360, 0xc1082707, 0xe0000000, 0xc01f, 0x23026110,
		0xe8fc1bff, 0xffffffff, 0x18007, 0x13b3088, 0x1c0019ff, 0x3fffffff, 0x3,
		0xfed88c, 0xc38007, 0x0, 0x600, 0x1fe4c04, 0xe0ff80c4, 0x1fffffff,
		0x100603, 0x3f180e00, 0x71ff84c4, 0xf0000000, 0x18010f, 0x7e010bc0,
		0x3be30808, 0xe3fff800, 0x400bf, 0xc41089f0, 0x9e0c1e7f, 0x87fffe0f,
		0x38073, 0x83f06478, 0xe2c46ff, 0xc000700, 0x43e0, 0x1e00236c,
		0x1760c1c0, 0x180003e0, 0x2600, 0xfe000338, 0x1bffffff, 0xb00001ff,
		0x4001c03, 0xe240010, 0xf0038007, 0xe00000e1, 0xf80181f, 0xc0000,
		0xd801c063, 0xc0000060, 0x40303f, 0xf0080000, 0x68f8fe61, 0x30,
		0x207063, 0xffffe000, 0x3100ff80, 0x18, 0x1fc060, 0x5c011000,
		0x1a313bb8, 0x1c, 0xc0060, 0x4e00c800, 0x1ff919f8, 0x1e, 0x181060,
		0x37f04600, 0x7f90cf8, 0x1b, 0x301060, 0xa3fc0600, 0x80380639, 0x19,
		0x702060, 0xe0000, 0xc601f302, 0x18, 0x3c040e0, 0x8b0000, 0xf20411c6,
		0x18, 0x7808160, 0x64cf0000, 0xf00801e0, 0x30, 0x3c010660, 0x22460000,
		0xff9f0160, 0xf0, 0x78220060, 0x19000000, 0xdcf10130, 0x1e0, 0x78c23060,
		0x800000, 0xce7f1c1e, 0x1e0, 0x78c260c0, 0x800000, 0xc731fd0f, 0x1e0,
		0x100201c0, 0x80000000, 0xf310fc89, 0x80, 0x40300, 0xc0000000,
		0xc9ce18cc, 0x0, 0x80600, 0x78000000, 0x66e04002, 0x0, 0x100e00,
		0x78000000, 0x74706003, 0x0, 0x200a00, 0x78000000, 0x30202000, 0x0,
		0x401a00, 0x30000000, 0x38000004, 0x0, 0x803a00, 0x0, 0x3fb80002, 0x0,
		0x1004a00, 0x80000000, 0x37c00001, 0x0, 0x1008a00, 0x80000000,
		0x30e70001, 0x0, 0x10a00, 0x0, 0x307f0000, 0x0, 0x20a00, 0x0,
		0x703f0000, 0x0, 0x20a00, 0x0, 0x78070000, 0x0, 0xa00, 0x0, 0x78000000,
		0x0, 0xa00, 0x0, 0x70000000, 0x0, 0x40a00, 0x0, 0x0, 0x0, 0x81a00, 0x0,
		0x0, 0x0, 0x103a00, 0x0, 0x0, 0x0, 0x304a00, 0x0, 0x0, 0x0, 0x208a00,
		0x0, 0x0, 0x0, 0x10a00, 0x0, 0x0, 0x0, 0x20a00, 0x0, 0x0, 0x0, 0x40a00,
		0x0, 0x0, 0x0, 0x80a00, 0x0, 0x0, 0x0, 0x100a40, 0x0, 0x0, 0x0,
		0x2008c0, 0x0, 0x0, 0x0, 0x601800, 0x0, 0x0, 0x0, 0xc03a00, 0x0, 0x0,
		0x0, 0x4a00, 0x0, 0x0, 0x0, 0x38a00, 0x0, 0x0, 0x0, 0x40a00, 0x0, 0x0,
		0x0, 0x80a00, 0x0, 0x0, 0x0, 0x101a00, 0x0, 0x0, 0x0, 0x303200, 0x0,
		0x0, 0x0, 0x207200, 0x0, 0x0, 0x0, 0x1e200, 0x0, 0x0, 0x0, 0x3c000, 0x0,
		0x0, 0x0, 0x3c200, 0x0, 0x0, 0x0, 0x18200, 0x0, 0x0, 0x0, 0x200, 0x0,
		0x0, 0x0, 0x400, 0x0, 0x0, 0x0, 0x1800, 0x0, 0x0, 0x0, 0x1800, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
// Lab 5 week 2
float32_t Ux[64] = { 0 };
float32_t Lx[64] = { 0 };
float32_t Uy = 0;
float32_t Ly = 0;
float32_t wc = 1.5707963267949;
float32_t theta = 0;
float32_t mu = 0.01;
float32_t baseband[FRAME_SIZE / 4] = { 0 };
uint32_t i_baseband = 0;

uint32_t data[512] = { 0 };
uint32_t i_word = 0;
uint32_t i_bit = 0;
int32_t k = 0;
/*
 This function will be called once before beginning the main program loop.
 This is the best place to build a lookup table.
 */
void lab_init(int16_t *output_buffer) {
	//display_image(tree,128,128);
//	for (uint32_t n = 0; n<16415; n+=1)
//	{
//		PN[i_word] |= (
//		                (s>>13) & 1
//		              ) << i_bit;
//		i_bit += 1;
//		if (i_bit == 32)
//		{
//			i_word += 1;
//			i_bit = 0;
//		}
//		s = ( s|(
//		          (1<<14) & (
//		                      (s<<2)^(s<<12)^(s<<13)^(s<<14)
//		                    )
//		        )
//		    ) >> 1;
//	}
//	data_stream[0] = 0x967c6ea1;
//	for (i_word = 0; i_word < 512; i_word+=1)
//	{
//	    data_stream[i_word+1] = tree[i_word]^PN[i_word]; // For scrambling
//	    //data_stream[i_word+1] = tree[i_word]; // For no scrambling
//	}
//	arm_fir_interpolate_init_f32 (&filter_instance, upsampling_factor, num_taps, pulse_shaping_coeffs, state, FRAME_SIZE/4);
	display_image(data, 128, 128);
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
void process_input_buffer(int16_t *input_buffer) {
	int16_t left_sample;
	int16_t right_sample;
	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE / 2; i_sample += 1) {
		left_sample = input_buffer[i_sample];
		i_sample += 1;
		right_sample = input_buffer[i_sample];
		fft_in[i_sample / 2] = (((float32_t) left_sample)
				+ ((float32_t) right_sample)) / 2;
	}
	arm_rfft_fast_f32(&fft_inst, fft_in, fft_out, 0);
	arm_cmplx_mag_f32(fft_out, fft_mag, FRAME_SIZE / 8);

//	for (uint32_t i_sample = 0; i_sample<FRAME_SIZE/64; i_sample+=1)
//	{
//	    filter_in[i_sample] = ( data_stream[i_word] & (1<<i_bit) ) >> i_bit;
//	    filter_in[i_sample] = (filter_in[i_sample]*2) - 1;
//
//	    i_bit += 1;
//	    if (i_bit == 32)
//	    {
//	        i_word += 1;
//	        i_bit = 0;
//	        if (i_word > 512){i_word = 0;}
//	    }
//	}

//	arm_fir_interpolate_f32 (&filter_instance, filter_in, filter_out, FRAME_SIZE/64);
//
//	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
//	{
//	     input_buffer[i_sample] = OUTPUT_SCALE_FACTOR*filter_out[i_sample/2]*cos_lut[i_lut];
//	     i_lut = (i_lut + 1) % 4;
//	     i_sample+=1;
//	     input_buffer[i_sample] = 0;
//	}
	return;
}

/*
 This function provides access to each individual sample that is incoming on the left channel.
 The returned value will be sent to the left channel DAC.
 Default behavior:
 1. Copy input to output without modification (passthrough)
 2. Estimate the number of cycles that have elapsed during the function call
 */
int16_t process_left_sample(int16_t input_sample) {
	//tic();
	int16_t output_sample;
	output_sample = input_sample;
	//elapsed_cycles = toc();
	return output_sample;
}

/*
 This function provides access to each individual sample that is incoming on the left channel.
 The returned value will be sent to the right channel DAC.
 Default behavior:
 1. Copy input to output without modification (passthrough)
 2. Estimate the number of cycles that have elapsed during the function call
 */
int16_t process_right_sample(int16_t input_sample) {
	float32_t r = input_sample * INPUT_SCALE_FACTOR;

	Ux[0] = r * arm_cos_f32(theta);
	Lx[0] = r * arm_sin_f32(theta);

	Uy = 0;
	Ly = 0;

	for (uint32_t i_sample = 0; i_sample < 64; i_sample += 1) {
		Uy += Ux[i_sample] * pulse_shaping_coeffs[i_sample];
		Ly += Lx[i_sample] * pulse_shaping_coeffs[i_sample];
	}

	baseband[i_baseband] = Uy;
	i_baseband += 1;
	if (i_baseband > FRAME_SIZE / 4) {
		i_baseband = 0;
	}

	for (uint32_t i_sample = 63; i_sample > 0; i_sample -= 1) {
		Ux[i_sample] = Ux[i_sample - 1];
		Lx[i_sample] = Lx[i_sample - 1];
	}
	theta += wc - mu * Uy * Ly;
	if (theta > 6.283185) {
		theta -= 6.283185;
	}

	if (k == 15) {
		k = 0;
		if (Uy > 0) {
			data[i_word] |= (1 << i_bit);
		}

		i_bit += 1;
		if (i_bit == 32) {
			i_word += 1;
			i_bit = 0;
		}
	} else {
		k++;
	}

	if (i_word > 511) {
		display_image(data, 128, 128);
		i_word = 0;
		i_bit = 0;
		for (size_t i = 0; i < 512; i++) {
			data[i] = 0;
		}
	}
	//display_image(data,128,128);

	return OUTPUT_SCALE_FACTOR * Uy * 0.3;

//	display_image(tree,128,128);
//	return input_sample;
}

/*
 This function provides another opportunity to access the frame of data
 The default behavior is to leave the buffer unchanged (passthrough)
 The buffer you see here will have any changes that occurred to the signal due to:
 1. the process_input_buffer function
 2. the process_left_sample and process_right_sample functions
 */
void process_output_buffer(int16_t *output_buffer) {
	return;
}
