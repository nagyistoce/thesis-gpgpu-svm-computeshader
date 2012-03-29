#pragma once
#include "ISVM.h"
#include "GraphicsManager.h"
#include <CL/opencl.h>

namespace SVM_Framework{
	class OpenCLSVM : public ISVM{
	public:
		OpenCLSVM(GraphicsManagerPtr dxMgr);
		~OpenCLSVM();

		enum OpenCLEnums {
			OCLP_UpdateErrorCache=0,
			OCLP_Test,
			OCLP_SVMOutput,
			OCLP_SelfProd,

			OCLB_InputData,
			OCLB_SelfProd,
			OCLB_Output,
			OCLB_TrainingInds,
			OCLB_TestingInds,
			OCLB_InputInds,
			OCLB_Alpha,
			OCLB_Class,
			OCLB_ConstantBuffer,
			
			OCLK_UpdateErrorCache,
			OCLK_Test,
			OCLK_SVMOutput,
			OCLK_SelfProd
		};

	protected:
		void beginExecute();
		void endExecute();
		void lagrangeThresholdUpdate(svm_precision p1, svm_precision p2, int id, int i1, int i2);
		void initializeFold(int id);
		void endFold(int id);
		void updateErrorCache(svm_precision f, int i, int id);
		svm_precision SVMOutput(int index, InstancePtr inst, int id);
		void testInstances(std::vector<svm_precision> &finalResult, int id);
		void kernelEvaluations(std::vector<int> &inds, std::vector<svm_precision> &result, int id);
	private:
		void compileProgram(std::string filename, OpenCLEnums program);
		void checkError(cl_int error);

		cl_context m_context;
		cl_command_queue m_cQueue;

		std::map<OpenCLEnums,cl_program> m_programs;
		std::map<OpenCLEnums,cl_kernel> m_kernels;
		std::map<OpenCLEnums,cl_mem> m_buffers;

		unsigned int m_numDevices;
	};
}