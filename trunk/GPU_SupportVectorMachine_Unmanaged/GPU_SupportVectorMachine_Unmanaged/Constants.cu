typedef double svm_precision;
#define thread_group_size 64

struct constantBuffer{
	svm_precision	cb_kernelParam1;
	svm_precision	cb_kernelParam2;
	unsigned int	cb_instanceLength;
	unsigned int	cb_instanceCount;
	unsigned int	cb_classIndex;

	// Run flags
	unsigned int	cb_kernel;
	svm_precision	cb_param1;
	svm_precision	cb_param2;
	int				cb_ind1;
	int				cb_ind2;
};

struct evalStruct{
	int ind1;
	int ind2;
	int ind3;
};