#pragma once
#include "ISVM.h"
#include "DirectXManager.h"

namespace SVM_Framework{
	class DX11SVM : public ISVM{
	public:
		DX11SVM(GraphicsManagerPtr dxMgr);
		~DX11SVM();

	protected:
		void execute();
		int examineExample(int i2);
		int takeStep(int i1, int i2, double F2);
		double SVMOutput(int index, InstancePtr inst);
	private:
		ID3D11ComputeShader* m_shader;
		std::vector<ID3D11UnorderedAccessView*> m_accessViews;
		std::vector<ID3D11Buffer*> m_uavBuffers;
		std::vector<ID3D11Buffer*> m_constantBuffers;

		DirectXManagerPtr m_dxMgr;
	};
}