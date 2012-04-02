#include "SMO_Include.cl"
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__attribute__((reqd_work_group_size(thread_group_size, 1, 1)))
__kernel void Test(	__global svm_precision* b_outputBuffer,__global svm_precision* b_inputBuffer, 
					__global unsigned int *b_testingIndices,__global unsigned int* b_trainingIndices, 
					__global unsigned int* b_kernelEvalBuffer, __global svm_precision* b_class, 
					__global svm_precision* b_alpha, __global svm_precision* b_selfProd, 
					__constant SharedBuffer* constants){
	__local evalStruct sdata[thread_group_size];
	unsigned int tid = get_global_id(0);
	unsigned int groupIndex = get_local_id(0);

	if(tid >= constants->cb_instanceCount)
		return;

	unsigned int sVecInd = b_kernelEvalBuffer[tid];

	sdata[groupIndex].ind1 = b_testingIndices[constants->cb_ind1] * constants->cb_instanceLength;
	sdata[groupIndex].ind2 = b_trainingIndices[sVecInd] * constants->cb_instanceLength;
	
	svm_precision result1 = 0;
	unsigned int i = 0;
	do{
		if(i != constants->cb_classIndex)
			result1 += b_inputBuffer[i+sdata[groupIndex].ind1] * b_inputBuffer[i+sdata[groupIndex].ind2];
		i++;
	}while (i < constants->cb_instanceLength);

	b_outputBuffer[tid] = evaluateKernel(result1,sdata[groupIndex].ind1/constants->cb_instanceLength,sdata[groupIndex].ind2/constants->cb_instanceLength,b_selfProd,constants) * b_class[sVecInd] * b_alpha[sVecInd];
}