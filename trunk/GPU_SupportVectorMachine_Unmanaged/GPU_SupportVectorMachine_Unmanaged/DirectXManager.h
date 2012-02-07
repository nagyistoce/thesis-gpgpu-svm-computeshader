#pragma once
#include "GraphicsManager.h"

namespace SVM_Framework{
	class DirectXManager : public GraphicsManager{
	public:
		DirectXManager();
		~DirectXManager();

		struct dxAdapter{
			ID3D11Device			*m_deviceHandle;
			ID3D11DeviceContext		*m_context;
			D3D_FEATURE_LEVEL		m_featureLevel;

			std::wstring m_name;
		};

		void initialize();
		void launchComputation(int x, int y, int z);
		void setComputeShader(ID3D11ComputeShader* shader);
		void setComputeShaderConstantBuffers(std::vector<ID3D11Buffer*> &buffers);
		void setComputeShaderUnorderedAccessViews(std::vector<ID3D11UnorderedAccessView*> &views);

		// Helpers
		template <class T>
		HRESULT createStructuredBuffer(UINT iNumElements, ID3D11Buffer** ppBuffer, ID3D11UnorderedAccessView** ppUAV, const T* pInitialData){
			HRESULT hr = S_OK;

			// Create SB
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory( &bufferDesc, sizeof(bufferDesc) );
			bufferDesc.ByteWidth = iNumElements * sizeof(T);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = sizeof(T);

			D3D11_SUBRESOURCE_DATA bufferInitData;
			ZeroMemory( &bufferInitData, sizeof(bufferInitData) );
			bufferInitData.pSysMem = pInitialData;
			hr = m_adapters.back().m_deviceHandle->CreateBuffer( &bufferDesc, (pInitialData)? &bufferInitData : NULL, ppBuffer);
			if(FAILED(hr))
				return hr;

			// Create UAV
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory( &uavDesc, sizeof(uavDesc) );
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = iNumElements;
			hr = m_adapters.back().m_deviceHandle->CreateUnorderedAccessView( *ppBuffer, &uavDesc, ppUAV );
			if(FAILED(hr))
				return hr;

			return hr;
		}

		template <class T>
		HRESULT createConstantBuffer(ID3D11Buffer** ppBuffer, const T* pInitialData){
			// Fill in a buffer description.
			D3D11_BUFFER_DESC cbDesc;
			cbDesc.ByteWidth = sizeof( T );
			cbDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbDesc.MiscFlags = 0;
			cbDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA bufferInitData;
			ZeroMemory( &bufferInitData, sizeof(bufferInitData) );
			bufferInitData.pSysMem = pInitialData;

			// Create the buffer.
			m_adapters.back().m_deviceHandle->CreateBuffer( &cbDesc, (pInitialData)? &bufferInitData : NULL, ppBuffer );
		}
		HRESULT createComputeShader(std::string filename, ID3D11ComputeShader** ppShader);	
	private:
		void createShaders();

		std::vector<dxAdapter>	m_adapters;

		HWND m_hwnd;
	};
}

typedef boost::shared_ptr<SVM_Framework::DirectXManager> DirectXManagerPtr;