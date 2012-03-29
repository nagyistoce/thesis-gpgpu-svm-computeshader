#include "SMO_Include.cl"
//#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__attribute__((reqd_work_group_size(thread_group_size, 1, 1)))
__kernel void UpdateErrorCache(	__global svm_precision *b_inputBuffer,__global unsigned int* b_kernelEvalBuffer, 
								__global unsigned int* b_trainingIndices,__global svm_precision* b_errorBuffer, 
								__global svm_precision *b_selfProd, __constant SharedBuffer* constants){
	__local evalStruct sdata[thread_group_size];

	unsigned int tid = get_global_id(0);
	unsigned int threadID = get_local_id(0); 

	if(tid >= constants->cb_instanceCount)
		return;

	unsigned int n = constants->cb_instanceLength;
	unsigned int outputInd = b_kernelEvalBuffer[tid];

	sdata[threadID].ind1 = b_trainingIndices[constants->cb_ind1] * n;
	sdata[threadID].ind2 = b_trainingIndices[constants->cb_ind2] * n;
	sdata[threadID].ind3 = b_trainingIndices[outputInd] * n;
	
	svm_precision result1 = 0, result2 = 0;
	unsigned int i = 0;
	do{
		if(i != constants->cb_classIndex)
			result1 += b_inputBuffer[i+sdata[threadID].ind1] * b_inputBuffer[i+sdata[threadID].ind3];
		i++;
	}while (i < n);

	i = 0;
	do{
		if(i != constants->cb_classIndex)
			result2 += b_inputBuffer[i+sdata[threadID].ind2] * b_inputBuffer[i+sdata[threadID].ind3];
		i++;
	}while (i < n);

	b_errorBuffer[tid*2] = evaluateKernel(result1,sdata[threadID].ind1/n,sdata[threadID].ind3/n);
	b_errorBuffer[(tid*2)+1] = evaluateKernel(result2,sdata[threadID].ind2/n,sdata[threadID].ind3/n);
	//b_errorBuffer[tid] = (constants->cb_param1 * evaluateKernel(result1,sdata[threadID].ind1/n,sdata[threadID].ind3/n,b_selfProd,constants)) + (constants->cb_param2 * evaluateKernel(result2,sdata[threadID].ind2/n,sdata[threadID].ind3/n,b_selfProd,constants));
}