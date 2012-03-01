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
							GB_ErrorBuffer,
							GB_SelfProdBuffer,
							GB_ClassBuffer,
							GB_AlphaBuffer,
							GB_DataBuffer,
							GB_InputInds,
							GB_FindBI,

							GS_ErrorUpdate,
							GS_SelfProd,
							GS_Testing,
							GS_SVMOutput,
							GS_FindBI,
							GS_UpdateErrorCache,
							GS_KernelEvaluations,

							CB_Shared,

							// Order dependent! Same as shader order
							UAV_SelfProd,
							UAV_Error,
							UAV_Output,
							UAV_FindBI,

							// Order dependent! Same as shader order
							SRV_Data,
							SRV_InputInds,
							SRV_TrainingIndices,
							SRV_TestingIndices,
							SRV_Class,
							SRV_Alpha
						};

		// Shader buffers
		struct SharedBuffer{
			float			kernelParam1;
			float			kernelParam2;
			unsigned int	instanceLength;
			unsigned int	instanceCount;
			unsigned int	classIndex;
			unsigned int	kernel;
			float			param1;
			float			param2;
			int				cb_ind1;
			int				cb_ind2;
		};

		struct algorithmParams{
			float	s_bLow,
					s_bUp;
			int		s_iLow,
					s_iUp;
		};

	protected:
		void execute();
		void lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2);
		void initializeFold(int id);
		void endFold(int id);
		void updateErrorCache(float f, int i, int id);
		double SVMOutput(int index, InstancePtr inst, int id);
		void testInstances(int id);
		void kernelEvaluations(std::vector<int> &inds, std::vector<float> &result, int id);
	private:
		void initGPUResources();
		void cleanGPUResources();
		void performTestCycle(std::vector<int> &inds);

		std::map<int,ID3D11ComputeShader*> m_shaders;

		std::map<int,ID3D11ShaderResourceView*> m_resourceViews;
		std::map<int,ID3D11UnorderedAccessView*> m_accessViews;
		std::map<int,ID3D11Buffer*> m_buffers;
		std::map<int,ID3D11Buffer*> m_constantBuffers;

		std::vector<SharedBuffer> m_sharedBuffer;

		DirectXManagerPtr m_dxMgr;
	};
}