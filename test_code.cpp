/*************************************************************
Filename: test_code.cpp

Description: Unit tests for the edge enhancer code for the 
	     SDPT & PAO project

Author: Alexandru-Iulian Constantinescu
**************************************************************/

#include <gtest/gtest.h>
#include "code.h"
#include <stdlib.h>

#define TEST01_v_process_png_file TEST_F

extern short int kernel[3][3];

class TEST_code : virtual public ::testing::Test{
public:
	virtual void SetUp();
	virtual void TearDown();
};


TEST01_v_process_png_file(TEST_code, process_png_file_v1){
	DETAILS("This test case verifies the functionality of process_png_file function");

	short int test_kernel[3][3] = {
       		{0 , -1, 0},
       		{-1, 5, -1},
       		{0, -1, 0}
   	};

	TESTCASE;
	DETAILS("checking if kernel matrix is initialized as expected");
	ASSERT_EQ(test_kernel, kernel);	
}
