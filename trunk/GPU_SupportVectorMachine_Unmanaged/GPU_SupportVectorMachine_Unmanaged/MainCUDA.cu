#include "Constants.cu"

__global__ void CUDA_global_selfProd(svm_precision* b_selfProd, svm_precision* b_inputData);
__global__ void CUDA_global_test(svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData, svm_precision* b_class, svm_precision* b_alpha);
__global__ void CUDA_global_errorCacheUpdate(svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData);
__global__ void CUDA_global_SVMOutput(svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData, svm_precision* b_class, svm_precision* b_alpha);
__host__ void CUDA_host_updateConstantBuffer(constantBuffer* src);

extern "C" void CUDA_updateConstantBuffer(void* src){
	CUDA_host_updateConstantBuffer((constantBuffer*)src);
}

extern "C" void CUDA_selfProd(unsigned int num_threads,svm_precision* b_selfProd, svm_precision* b_inputData){
	dim3 grid(int(svm_precision(num_threads)/thread_group_size)+1, 1, 1);
    dim3 threads(thread_group_size, 1, 1);

    CUDA_global_selfProd<<< grid, threads >>>(b_selfProd,b_inputData);
}

extern "C" void CUDA_testInstances(unsigned int num_threads, svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData, svm_precision* b_class, svm_precision* b_alpha){
	dim3 grid(int(svm_precision(num_threads)/thread_group_size)+1, 1, 1);
    dim3 threads(thread_group_size, 1, 1);

	CUDA_global_test<<< grid, threads >>>(b_output,b_selfProd,b_inputData,b_class,b_alpha);
}

extern "C" void CUDA_lagrangeUpdate(unsigned int num_threads, svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData){
	dim3 grid(int(svm_precision(num_threads)/thread_group_size)+1, 1, 1);
    dim3 threads(thread_group_size, 1, 1);

	CUDA_global_errorCacheUpdate<<< grid, threads >>>(b_output,b_selfProd,b_inputData);
}

extern "C" void CUDA_SVMOutput(unsigned int num_threads, svm_precision* b_output, svm_precision* b_selfProd, svm_precision* b_inputData, svm_precision* b_class, svm_precision* b_alpha){
	dim3 grid(int(svm_precision(num_threads)/thread_group_size)+1, 1, 1);
    dim3 threads(thread_group_size, 1, 1);

	CUDA_global_SVMOutput<<< grid, threads >>>(b_output,b_selfProd,b_inputData,b_class,b_alpha);
}