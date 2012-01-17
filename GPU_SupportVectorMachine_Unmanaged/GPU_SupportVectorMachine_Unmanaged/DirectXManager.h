#pragma once
#include "GraphicsManager.h"

namespace SVM_Framework{
	class DirectXManager : public GraphicsManager{
	public:
		DirectXManager();
		~DirectXManager();

		void initialize();
		void launchComputation(std::string shader, int x, int y, int z);
	private:
		void createShaders();
		template <class T>
		HRESULT createStructuredBuffer(UINT iNumElements, ID3D11Buffer** ppBuffer, ID3D11UnorderedAccessView** ppUAV, const T* pInitialData = NULL);

		struct dxAdapter{
			ID3D11Device			*m_deviceHandle;
			ID3D11DeviceContext		*m_context;
		};

		struct bufferStruct{
			float value;
		};

		std::vector<dxAdapter>	m_adapters;

		D3D_FEATURE_LEVEL		m_featureLevel;

		std::map<std::string,ID3D11ComputeShader*> m_computeShaders;
		ID3D11UnorderedAccessView* m_accesView;
		ID3D11Buffer* m_gfxBuffer;

		HWND m_hwnd;
	};
}

typedef boost::shared_ptr<SVM_Framework::DirectXManager> DirectXManagerPtr;