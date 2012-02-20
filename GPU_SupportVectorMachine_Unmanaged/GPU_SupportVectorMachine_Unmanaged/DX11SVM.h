#pragma once
#include "ISVM.h"
#include "DirectXManager.h"

namespace SVM_Framework{
	class DX11SVM : public ISVM{
	public:
		DX11SVM(GraphicsManagerPtr dxMgr);
		~DX11SVM();

		struct SharedBuffer{
			float			kernelParam1;
			float			kernelParam2;
			unsigned int	instanceLength;
			unsigned int	instanceCount;
			unsigned int	classIndex;
		};

	protected:
		void execute();
		int examineExample(int i2);
		int takeStep(int i1, int i2, double F2);
		double SVMOutput(int index, InstancePtr inst);
	private:
		ID3D11ComputeShader	*m_shaderTraining,
							*m_shaderTesting;

		std::vector<ID3D11ShaderResourceView*> m_resourceViews;
		std::vector<ID3D11UnorderedAccessView*> m_accessViews;
		std::vector<ID3D11Buffer*> m_buffers;
		std::vector<ID3D11Buffer*> m_constantBuffers;

		DirectXManagerPtr m_dxMgr;
	};
}