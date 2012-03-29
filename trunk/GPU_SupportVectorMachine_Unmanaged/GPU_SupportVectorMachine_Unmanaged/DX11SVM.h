#pragma once
#include "ISVM.h"
#include "DirectXManager.h"

namespace SVM_Framework{
	class DX11SVM : public ISVM{
	public:
		DX11SVM(GraphicsManagerPtr dxMgr);
		~DX11SVM();

		enum GResources	{	
							GB_TrainingIndices = 0,
							GB_TestingIndices,
							GB_OutputBuffer,
							GB_SelfProdBuffer,
							GB_ClassBuffer,
							GB_AlphaBuffer,
							GB_DataBuffer,
							GB_InputInds,

							GS_SelfProd,
							GS_Testing,
							GS_SVMOutput,
							GS_UpdateErrorCache,

							CB_Shared,

							// Order dependent! Same as shader order
							UAV_SelfProd,
							UAV_Output,

							// Order dependent! Same as shader order
							SRV_Data,
							SRV_InputInds,
							SRV_TrainingIndices,
							SRV_TestingIndices,
							SRV_Class,
							SRV_Alpha
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
		void initGPUResources();
		void cleanGPUResources();

		std::vector<std::map<int,ID3D11ComputeShader*>> m_shaders;

		std::vector<std::map<int,ID3D11ShaderResourceView*>> m_resourceViews;
		std::vector<std::map<int,ID3D11UnorderedAccessView*>> m_accessViews;
		std::vector<std::map<int,ID3D11Buffer*>> m_buffers;
		std::vector<std::map<int,ID3D11Buffer*>> m_constantBuffers;

		DirectXManagerPtr m_dxMgr;
	};
}