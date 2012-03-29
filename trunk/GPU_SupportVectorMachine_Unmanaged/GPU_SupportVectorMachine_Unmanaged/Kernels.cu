#include <cuda_runtime.h>
#include "Constants.cu"

__constant__ constantBuffer cb_constants;

//texture<svm_precision,1> t_inputDataTexture;
//texture<svm_precision,1> t_classTexture;
//texture<svm_precision,1> t_alphaTexture;
texture<unsigned int,1> t_trainingIndsTexture;
texture<unsigned int,1> t_testingIndsTexture;
texture<unsigned int,1> t_inputIndsTexture;

//surface<void,1> s_outputSurface;
//surface<void,1> s_selfProdSurface;

__host__ void CUDA_host_updateConstantBuffer(constantBuffer* src){
	cudaMemcpyToSymbol(cb_constants,src,sizeof(constantBuffer));
}

__device__ svm_precision CUDA_device_dotProd(unsigned int i1,unsigned int i2, svm_precision* b_inputData){
	svm_precision result = 0;

	for(unsigned int i=0; i<cb_constants.cb_instanceLength; i++){
		if(i != cb_constants.cb_classIndex){
			result += b_inputData[(cb_constants.cb_instanceLength*i1)+i]*b_inputData[(cb_constants.cb_instanceLength*i2)+i];
		}
	}

	return result;
}

__device__ svm_precision CUDA_device_pukKernel(svm_precision dp, int i1, int i2, svm_precision* b_selfProd){
	svm_precision squaredDifference = -2.0 * dp + b_selfProd[i1] + b_selfProd[i2];
	svm_precision intermediate = cb_constants.cb_kernelParam1 * sqrt(squaredDifference);
	return (1.0 / powf(1.0 + intermediate * intermediate,cb_constants.cb_kernelParam2));
}

__device__ svm_precision CUDA_device_rbfKernel(svm_precision dp, int i1, int i2, svm_precision* b_selfProd){
	return (cb_constants.cb_kernelParam1 * (2. * dp - b_selfProd[i1] - b_selfProd[i2]));
}

__device__ svm_precision CUDA_device_evaluateKernel(svm_precision dp, int i1, int i2, svm_precision* b_selfProd){
	if(i1 == i2)
		return 1.0;

	if(cb_constants.cb_kernel == 0)
		return CUDA_device_pukKernel(dp,i1,i2,b_selfProd);
	else if(cb_constants.cb_kernel == 1)
		return CUDA_device_rbfKernel(dp,i1,i2,b_selfProd);
	else
		return 1.0;
}

__global__ void CUDA_global_selfProd(svm_precision* b_selfProd, svm_precision* b_inputData){
	int tid = blockIdx.x*thread_group_size+threadIdx.x;
	if(tid < cb_constants.cb_instanceCount)
		b_selfProd[tid] = CUDA_device_dotProd(tid,tid,b_inputData);
}

__shared__ evalStruct sdata[thread_group_size];
__global__ void CUDA_global_SVMOutput(svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData, svm_precision* b_class, svm_precision* b_alpha){
	int tid = blockIdx.x*thread_group_size+threadIdx.x;

	if(tid >= cb_constants.cb_instanceCount)
		return;

	unsigned int n = cb_constants.cb_instanceLength;
	unsigned int outputInd = tex1Dfetch(t_inputIndsTexture,tid);

	sdata[threadIdx.x].ind1 = tex1Dfetch(t_trainingIndsTexture,cb_constants.cb_ind1) * n;
	sdata[threadIdx.x].ind2 = tex1Dfetch(t_trainingIndsTexture,outputInd) * n;
	
	svm_precision result1 = 0;
	unsigned int i = 0;
	do{
		if(i != cb_constants.cb_classIndex)
			result1 += b_inputData[i+sdata[threadIdx.x].ind1] * b_inputData[i+sdata[threadIdx.x].ind2];
		i++;
	}while (i < n);

	b_output[tid] = CUDA_device_evaluateKernel(result1,sdata[threadIdx.x].ind1/n,sdata[threadIdx.x].ind2/n,b_selfProd);
	//b_output[tid] = CUDA_device_evaluateKernel(result1,sdata[threadIdx.x].ind1/n,sdata[threadIdx.x].ind2/n,b_selfProd) * b_class[outputInd] * b_alpha[outputInd];
}

__global__ void CUDA_global_errorCacheUpdate(svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData){
	int tid = blockIdx.x*thread_group_size+threadIdx.x;

	if(tid >= cb_constants.cb_instanceCount)
		return;

	unsigned int n = cb_constants.cb_instanceLength;
	unsigned int outputInd = tex1Dfetch(t_inputIndsTexture,tid);

	sdata[threadIdx.x].ind1 = tex1Dfetch(t_trainingIndsTexture,cb_constants.cb_ind1) * n;
	sdata[threadIdx.x].ind2 = tex1Dfetch(t_trainingIndsTexture,cb_constants.cb_ind2) * n;
	sdata[threadIdx.x].ind3 = tex1Dfetch(t_trainingIndsTexture,outputInd) * n;
	
	svm_precision result1 = 0, result2 = 0;
	unsigned int i = 0;
	do{
		if(i != cb_constants.cb_classIndex)
			result1 += b_inputData[i+sdata[threadIdx.x].ind1] * b_inputData[i+sdata[threadIdx.x].ind3];
		i++;
	}while (i < n);

	i = 0;
	do{
		if(i != cb_constants.cb_classIndex)
			result2 += b_inputData[i+sdata[threadIdx.x].ind2] * b_inputData[i+sdata[threadIdx.x].ind3];
		i++;
	}while (i < n);


	b_output[tid*2] = CUDA_device_evaluateKernel(result1,sdata[threadIdx.x].ind1/n,sdata[threadIdx.x].ind3/n,b_selfProd);
	b_output[(tid*2)+1] = CUDA_device_evaluateKernel(result2,sdata[threadIdx.x].ind2/n,sdata[threadIdx.x].ind3/n,b_selfProd);
	//b_output[tid] =	cb_constants.cb_param1 * CUDA_device_evaluateKernel(result1,sdata[threadIdx.x].ind1/n,sdata[threadIdx.x].ind3/n,b_selfProd) + 
	//				cb_constants.cb_param2 * CUDA_device_evaluateKernel(result2,sdata[threadIdx.x].ind2/n,sdata[threadIdx.x].ind3/n,b_selfProd);
}

__shared__ svm_precision sdataTest[thread_group_size];
__global__ void CUDA_global_test(svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData, svm_precision* b_class, svm_precision* b_alpha){
	int tid = blockIdx.x*thread_group_size+threadIdx.x;
	int groupIndex = threadIdx.x;

	if(tid >= cb_constants.cb_instanceCount)
		return;

	unsigned int sVecInd = tex1Dfetch(t_inputIndsTexture,tid);

	sdata[groupIndex].ind1 = tex1Dfetch(t_testingIndsTexture,cb_constants.cb_ind1) * cb_constants.cb_instanceLength;
	sdata[groupIndex].ind2 = tex1Dfetch(t_trainingIndsTexture,sVecInd) * cb_constants.cb_instanceLength;
	
	svm_precision result1 = 0;
	unsigned int i = 0;
	do{
		if(i != cb_constants.cb_classIndex)
			result1 += b_inputData[i+sdata[groupIndex].ind1] * b_inputData[i+sdata[groupIndex].ind2];
		i++;
	}while (i < cb_constants.cb_instanceLength);

	b_output[tid] = CUDA_device_evaluateKernel(result1,sdata[groupIndex].ind1/cb_constants.cb_instanceLength,sdata[groupIndex].ind2/cb_constants.cb_instanceLength,b_selfProd) * b_class[sVecInd] * b_alpha[sVecInd];
}