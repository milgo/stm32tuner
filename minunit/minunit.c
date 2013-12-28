#include "minunit.h"

#ifdef __UNIT_TESTING

int tests_run = 0;
 
static char * test_ConvertToDecibels() {
	float32_t res = 0.0f;
	 
	res = ConvertToDecibels(4095.0f);
	 mu_assert("error, ConvertToDecibels(4095.0f) != 72.245078", res == 72.245078f);
	res = ConvertToDecibels(-4095.0f);
	 mu_assert("error, ConvertToDecibels(-4095.0f) != 72.245078", res == 72.245078f);
	res = ConvertToDecibels(0.0f);
	 mu_assert("error, ConvertToDecibels(0.0f) != -80", res == -80.0f);

  return 0;
}
 
static char * test_FindPeak() {
	
	arm_status status; 
  arm_rfft_instance_q15 rfft; 
  arm_cfft_radix4_instance_q15 cfft;  
	uint32_t ifftFlag = 0; 
	uint32_t doBitReverse = 1;
	double freq;
	float32_t y = 0.0f;
	int i = 0, peakInt;
	
  status = ARM_MATH_SUCCESS; 
  status = arm_rfft_init_q15(&rfft, &cfft, FFT_SIZE, ifftFlag, doBitReverse);	
	mu_assert("test_FindPeak() error, status != ARM_MATH_SUCCESS", status == ARM_MATH_SUCCESS);
	
	/* Testing min frequency*/
	freq = 20.0;
	for(i=0; i < LENGTH_SAMPLES; i++){
		y = (4096 * arm_sin_f32(freq * 2 * PI * i/LENGTH_SAMPLES)) * 3300 / 0xFFF;
		arm_float_to_q15(&y, &input[i], 1);
	}
	
	arm_rfft_q15(&rfft, input, output); 
	peakInt = FindPeak(output);
	mu_assert("error, test_FindPeak() != 20Hz", peakInt == (int)freq);
	
	/* Testing max frequency*/
	freq = 999.0;
	for(i=0; i < LENGTH_SAMPLES; i++){
		y = (4096 * arm_sin_f32(freq * 2 * PI * i/LENGTH_SAMPLES)) * 3300 / 0xFFF;
		arm_float_to_q15(&y, &input[i], 1);
	}
	
	arm_rfft_q15(&rfft, input, output); 
	peakInt = FindPeak(output);
	mu_assert("error, test_FindPeak() != 999Hz", peakInt == (int)freq);
	
	/* Testing mid frequency*/
	freq = 321.0;
	for(i=0; i < LENGTH_SAMPLES; i++){
		y = (4096 * arm_sin_f32(freq * 2 * PI * i/LENGTH_SAMPLES)) * 3300 / 0xFFF;
		arm_float_to_q15(&y, &input[i], 1);
	}
	
	arm_rfft_q15(&rfft, input, output); 
	peakInt = FindPeak(output);
	mu_assert("error, test_FindPeak() != 321Hz", peakInt == (int)freq);
  return 0;
}

static char* test_GetNoteInfo(){ //niedostateczne!!!
	uint8_t hzIndex, noteIndex, res;
	
	res = GetNoteInfo(60, &hzIndex, &noteIndex);
	mu_assert("test_GetNoteInfo() error, res != 2", res == 2);
	
	res = GetNoteInfo(65, &hzIndex, &noteIndex);
	mu_assert("test_GetNoteInfo() error, hzIndex != 0 || noteIndex != 0 || res != 0", hzIndex == 0 && noteIndex == 0 && res == 0);
	
	res = GetNoteInfo(66, &hzIndex, &noteIndex);
	mu_assert("test_GetNoteInfo() error, hzIndex != 0 || noteIndex != 0 || res != 0", hzIndex == 0 && noteIndex == 0 && res == 0);

	res = GetNoteInfo(67, &hzIndex, &noteIndex);
	mu_assert("test_GetNoteInfo() error, hzIndex != 0 || noteIndex != 0 || res != 0", hzIndex == 0 && noteIndex == 0 && res == 0);	
	
	res = GetNoteInfo(68, &hzIndex, &noteIndex);
	mu_assert("test_GetNoteInfo() error, hzIndex != 1 || noteIndex != 1 || res != 0", hzIndex == 1 && noteIndex == 1 && res == 0);	
	
	res = GetNoteInfo(1000, &hzIndex, &noteIndex);
	mu_assert("test_GetNoteInfo() error, res != 1", res == 1);
	return 0;
}
 
 static char * all_tests() {
    mu_run_test(test_ConvertToDecibels);
    mu_run_test(test_FindPeak);
		mu_run_test(test_GetNoteInfo);
    return 0;
 }
 
 int test_run(void) {
     char *result = all_tests();
     if (result != 0) {
         printf("%s\n", result);
     }
     else {
         printf("ALL TESTS PASSED\n");
     }
     printf("TESTS RUN: %d\n", tests_run);
 
     return result != 0;
 }

 #endif //__UNIT_TESTING
