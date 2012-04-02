#pragma OPENCL EXTENSION cl_khr_fp64 : enable
typedef double svm_precision;
#define thread_group_size 64

typedef struct{
	svm_precision			cb_kernelParam1;
	svm_precision			cb_kernelParam2;
	unsigned int	cb_instanceLength;
	unsigned int	cb_instanceCount;
	unsigned int	cb_classIndex;
	unsigned int	cb_kernel;
	svm_precision			cb_param1;
	svm_precision			cb_param2;
	int				cb_ind1;
	int				cb_ind2;
} SharedBuffer;

typedef struct{
	svm_precision	s_bLow,
			s_bUp;
	int		s_iLow,
			s_iUp;
} algorithmParams;

typedef struct{
	int ind1;
	int ind2;
	int ind3;
} evalStruct;

svm_precision absf(svm_precision val){
	if(val < 0){
		return -val;
	}
	return val;
}

svm_precision pukKernel(svm_precision dp, int i1, int i2, __global svm_precision* b_selfProd, __constant SharedBuffer* constants){
	svm_precision squaredDifference = -2.0 * dp + b_selfProd[i1] + b_selfProd[i2];
	svm_precision intermediate = constants->cb_kernelParam1 * sqrt(squaredDifference);
	return (1.0 / pow(1.0 + intermediate * intermediate,constants->cb_kernelParam2));
}

svm_precision rbfKernel(svm_precision dp, int i1, int i2, __global svm_precision* b_selfProd, __constant SharedBuffer* constants){
	return (constants->cb_kernelParam1 * (2. * dp - b_selfProd[i1] - b_selfProd[i2]));
}

svm_precision evaluateKernel(svm_precision dp, int i1, int i2, __global svm_precision* b_selfProd, __constant SharedBuffer* constants){
	if(i1 == i2)
		return 1.0;

	if(constants->cb_kernel == 0)
		return pukKernel(dp,i1,i2,b_selfProd,constants);
	else if(constants->cb_kernel == 1)
		return rbfKernel(dp,i1,i2,b_selfProd,constants);
	else
		return 1.0;
}