#include "../Includes/Kernel.hlsl"

// group size
#define thread_group_size_x 64
#define thread_group_size_y 1

//groupshared svm_precision sdata[thread_group_size_x];
//[numthreads( thread_group_size_x, thread_group_size_y, 1 )]
//void cs_entry(	uint3	threadIDInGroup : SV_GroupThreadID, 
//				uint3	groupID : SV_GroupID, 
//				uint	groupIndex : SV_GroupIndex, 
//				uint3	dispatchThreadID : SV_DispatchThreadID ){
//	evalStruct evalInds;
//	evalInds = b_kernelEvalBuffer[groupID.x];
//	evalInds.ind1 = b_instanceIndices[evalInds.ind1] * cb_instanceLength;
//	evalInds.ind2 = b_instanceIndices[evalInds.ind2] * cb_instanceLength;
//	sdata[groupIndex] = 0;
//
//	// Multiply stage
//	unsigned int n = (cb_instanceLength/thread_group_size_x);
//	unsigned int offset = n*groupIndex;
//	if(groupIndex == 31){
//		n = cb_instanceLength - offset;
//	}
//
//	unsigned int i = 0;
//	do{
//		if((i+offset) != cb_classIndex)
//			sdata[groupIndex] += b_inputBuffer[i+offset+evalInds.ind1] * b_inputBuffer[i+offset+evalInds.ind2];
//		i++;
//	}while (i < n);
//    GroupMemoryBarrierWithGroupSync();
//	
//	// Sum stage
//	if(groupIndex < 16)
//		sdata[groupIndex] += sdata[groupIndex + 16];
//	if(groupIndex < 8)
//		sdata[groupIndex] += sdata[groupIndex + 8];
//	if(groupIndex < 4)
//		sdata[groupIndex] += sdata[groupIndex + 4];
//	if(groupIndex < 2)
//		sdata[groupIndex] += sdata[groupIndex + 2];
//
//	// Final store of result
//	if(groupIndex == 0)
//		b_outputBuffer[groupID.x] = evaluateKernel(sdata[0]+sdata[1],evalInds.ind1/cb_instanceLength,evalInds.ind2/cb_instanceLength);
//}

struct evalStruct{
	int ind1;
	int ind2;
	int ind3;
};

//groupshared evalStruct sdata[thread_group_size_x];
//[numthreads( thread_group_size_x, thread_group_size_y, 1 )]
//void cs_entry(	uint	groupIndex : SV_GroupIndex,
//				uint3	dispatchThreadID : SV_DispatchThreadID){
//	if(dispatchThreadID.x >= cb_instanceCount)
//		return;
//
//	unsigned int n = cb_instanceLength;
//	uint outputInd = b_kernelEvalBuffer[dispatchThreadID.x];
//
//	sdata[groupIndex].ind1 = b_trainingIndices[cb_ind1] * n;
//	sdata[groupIndex].ind2 = b_trainingIndices[cb_ind2] * n;
//	sdata[groupIndex].ind3 = b_trainingIndices[outputInd] * n;
//	
//	svm_precision result1 = 0, result2 = 0;
//	unsigned int i = 0;
//	do{
//		if(i != cb_classIndex)
//			result1 += b_inputBuffer[i+sdata[groupIndex].ind1] * b_inputBuffer[i+sdata[groupIndex].ind3];
//		i++;
//	}while (i < n);
//
//	i = 0;
//	do{
//		if(i != cb_classIndex)
//			result2 += b_inputBuffer[i+sdata[groupIndex].ind2] * b_inputBuffer[i+sdata[groupIndex].ind3];
//		i++;
//	}while (i < n);
//
//	b_outputBuffer[dispatchThreadID.x] =	cb_param1 * evaluateKernel(result1,sdata[groupIndex].ind1/n,sdata[groupIndex].ind3/n) + 
//											cb_param2 * evaluateKernel(result2,sdata[groupIndex].ind2/n,sdata[groupIndex].ind3/n);
//}

groupshared evalStruct sdata[thread_group_size_x];
[numthreads( thread_group_size_x, thread_group_size_y, 1 )]
void cs_entry(	uint	groupIndex : SV_GroupIndex,
				uint3	dispatchThreadID : SV_DispatchThreadID){
	if(dispatchThreadID.x >= cb_instanceCount)
		return;

	unsigned int n = cb_instanceLength;
	uint outputInd = b_kernelEvalBuffer[dispatchThreadID.x];

	sdata[groupIndex].ind1 = b_trainingIndices[cb_ind1] * n;
	sdata[groupIndex].ind2 = b_trainingIndices[cb_ind2] * n;
	sdata[groupIndex].ind3 = b_trainingIndices[outputInd] * n;
	
	svm_precision result1 = 0, result2 = 0;
	unsigned int i = 0;
	do{
		if(i != cb_classIndex)
			result1 += b_inputBuffer[i+sdata[groupIndex].ind1] * b_inputBuffer[i+sdata[groupIndex].ind3];
		i++;
	}while (i < n);

	i = 0;
	do{
		if(i != cb_classIndex)
			result2 += b_inputBuffer[i+sdata[groupIndex].ind2] * b_inputBuffer[i+sdata[groupIndex].ind3];
		i++;
	}while (i < n);

	b_outputBuffer[dispatchThreadID.x*2] = evaluateKernel(result1,sdata[groupIndex].ind1/n,sdata[groupIndex].ind3/n);
	b_outputBuffer[(dispatchThreadID.x*2)+1] = evaluateKernel(result2,sdata[groupIndex].ind2/n,sdata[groupIndex].ind3/n);
}