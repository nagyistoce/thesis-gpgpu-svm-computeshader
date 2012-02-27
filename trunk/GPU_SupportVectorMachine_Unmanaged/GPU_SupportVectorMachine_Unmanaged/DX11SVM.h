#pragma once
#include "ISVM.h"
#include "DirectXManager.h"

namespace SVM_Framework{
	class DX11SVM : public ISVM{
	public:
		DX11SVM(GraphicsManagerPtr dxMgr);
		~DX11SVM();

		enum GBuffers	{	
							GB_TrainingIndices,
							GB_TestingIndices,
							GB_OutputBuffer,
							GB_ErrorBuffer,
							GB_SelfProdBuffer,
							GB_ClassBuffer,
							GB_AlphaBuffer,
							GB_DataBuffer,
							GB_InputInds
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
	private:
		void initGPUResources();

		std::vector<ID3D11ComputeShader*> m_shaders;

		std::vector<ID3D11ShaderResourceView*> m_resourceViews;
		std::vector<ID3D11UnorderedAccessView*> m_accessViews;
		std::map<GBuffers,ID3D11Buffer*> m_buffers;
		std::vector<ID3D11Buffer*> m_constantBuffers;

		std::vector<SharedBuffer> m_sharedBuffer;

		DirectXManagerPtr m_dxMgr;
	};
}