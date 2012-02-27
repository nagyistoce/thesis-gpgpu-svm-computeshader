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
	uint n2 = cb_ind1;

	sdata[groupIndex].ind1 = b_testingIndices[dispatchThreadID.x] * n;

	uint j = 0;
	float result = 0;
	do{
		uint outputInd = b_kernelEvalBuffer[j];
		sdata[groupIndex].ind2 = b_trainingIndices[outputInd] * n;

		float dot = 0;
		unsigned int i = 0;
		do{
			if(i != cb_classIndex)
				dot += b_inputBuffer[i+sdata[groupIndex].ind1] * b_inputBuffer[i+sdata[groupIndex].ind2];
			i++;
		}while (i < n);

		result += evaluateKernel(dot,sdata[groupIndex].ind1/n,sdata[groupIndex].ind2/n) * b_class[outputInd] * b_alpha[outputInd];
		j++;
	}while(j < n2);

	b_outputBuffer[dispatchThreadID.x] = result - cb_param1;
}