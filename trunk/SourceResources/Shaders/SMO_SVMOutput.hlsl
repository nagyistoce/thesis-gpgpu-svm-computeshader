#include "../Includes/Kernel.hlsl"

// group size
#define thread_group_size_x 32
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

	unsigned int n = cb_instanceLength;
	uint outputInd = b_kernelEvalBuffer[dispatchThreadID.x];

	sdata[groupIndex].ind1 = b_trainingIndices[cb_ind1] * n;
	sdata[groupIndex].ind2 = b_trainingIndices[outputInd] * n;
	
	float result1 = 0;
	unsigned int i = 0;
	do{
		if(i != cb_classIndex)
			result1 += b_inputBuffer[i+sdata[groupIndex].ind1] * b_inputBuffer[i+sdata[groupIndex].ind2];
		i++;
	}while (i < n);

	b_outputBuffer[dispatchThreadID.x] = evaluateKernel(result1,sdata[groupIndex].ind1/n,sdata[groupIndex].ind2/n) * b_class[outputInd] * b_alpha[outputInd];
}