#include "../Includes/Kernel.hlsl"

// group size
#define thread_group_size_x 64
#define thread_group_size_y 1

struct evalStruct{
	int ind1;
	int ind2;
};

groupshared evalStruct sdata[thread_group_size_x];
[numthreads( thread_group_size_x, thread_group_size_y, 1 )]
void cs_entry(	uint3	threadIDInGroup : SV_GroupThreadID, 
				uint3	groupID : SV_GroupID, 
				uint	groupIndex : SV_GroupIndex, 
				uint3	dispatchThreadID : SV_DispatchThreadID ){
	if(dispatchThreadID.x >= cb_instanceCount)
		return;

	unsigned int sVecInd = b_kernelEvalBuffer[dispatchThreadID.x];

	sdata[groupIndex].ind1 = b_testingIndices[cb_ind1] * cb_instanceLength;
	sdata[groupIndex].ind2 = b_trainingIndices[sVecInd] * cb_instanceLength;
	
	svm_precision result1 = 0;
	unsigned int i = 0;
	do{
		if(i != cb_classIndex)
			result1 += b_inputBuffer[i+sdata[groupIndex].ind1] * b_inputBuffer[i+sdata[groupIndex].ind2];
		i++;
	}while (i < cb_instanceLength);

	b_outputBuffer[dispatchThreadID.x] = evaluateKernel(result1,sdata[groupIndex].ind1/cb_instanceLength,sdata[groupIndex].ind2/cb_instanceLength) * b_class[sVecInd] * b_alpha[sVecInd];
}