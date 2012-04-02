#include "SMO_Include.cl"
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__attribute__((reqd_work_group_size(thread_group_size, 1, 1)))
__kernel void SelfProd(__global svm_precision *b_inputBuffer, __global svm_precision *b_selfProd, __constant SharedBuffer* constants){
    // load shared mem
    unsigned int tid = get_global_id(0);
	if(tid < constants->cb_instanceCount){
		svm_precision result = 0;
	
		for(unsigned int i=0; i<constants->cb_instanceLength; i++){
			if(i != constants->cb_classIndex)
				result += b_inputBuffer[(constants->cb_instanceLength*tid)+i]*b_inputBuffer[(constants->cb_instanceLength*tid)+i];
		}

		b_selfProd[tid] = result;
	}
}