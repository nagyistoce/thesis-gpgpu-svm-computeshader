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
		void setComputeShaderResourceViews(std::vector<ID3D11ShaderResourceView*> &views);
		void setComputeShaderUnorderedAccessViews(std::vector<ID3D11UnorderedAccessView*> &views);

		// Helpers
		template <class T>
		void copyFromGraphicsCard(ID3D11Buffer* buffer, std::vector<T> &copy){
			D3D11_BUFFER_DESC desc;
			buffer->GetDesc(&desc);

			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			ID3D11Buffer *cpuBuffer;
			m_adapters.back().m_deviceHandle->CreateBuffer(&desc,0,&cpuBuffer);
			m_adapters.back().m_context->CopyResource(cpuBuffer,buffer);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			m_adapters.back().m_context->Map(cpuBuffer,0,D3D11_MAP_READ,0,&mappedResource);
			
			std::copy((T*)mappedResource.pData,(T*)mappedResource.pData+copy.size(),copy.begin());
			
			m_adapters.back().m_context->Unmap(cpuBuffer,0);
			cpuBuffer->Release();
		}

		template <class T>
		void copyFromAppendBuffer(ID3D11Buffer* buffer, ID3D11UnorderedAccessView* uav, std::vector<T> &copy){
			unsigned int size = 0;
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ID3D11Buffer *cpuBuffer;
			D3D11_BUFFER_DESC desc;

			desc.ByteWidth = sizeof(unsigned int);
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			m_adapters.back().m_deviceHandle->CreateBuffer(&desc,0,&cpuBuffer);
			m_adapters.back().m_context->CopyStructureCount(cpuBuffer,0,uav);

			m_adapters.back().m_context->Map(cpuBuffer,0,D3D11_MAP_READ,0,&mappedResource);
			
			size = *(unsigned int*)mappedResource.pData;
			
			m_adapters.back().m_context->Unmap(cpuBuffer,0);
			cpuBuffer->Release();
			copy.assign(size,T());

			buffer->GetDesc(&desc);
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.MiscFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			m_adapters.back().m_deviceHandle->CreateBuffer(&desc,0,&cpuBuffer);
			m_adapters.back().m_context->CopyResource(cpuBuffer,buffer);
			m_adapters.back().m_context->Map(cpuBuffer,0,D3D11_MAP_READ,0,&mappedResource);
			
			std::copy((T*)mappedResource.pData,(T*)mappedResource.pData+copy.size(),copy.begin());
			
			m_adapters.back().m_context->Unmap(cpuBuffer,0);
			cpuBuffer->Release();
		}

		template <class T>
		void copyToGraphicsCard(ID3D11Buffer* pBuffer, std::vector<T> &data){
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			m_adapters.back().m_context->Map(pBuffer,0,D3D11_MAP_WRITE_DISCARD,0,&mappedResource);

			std::copy(data.begin(),data.end(),(T*)mappedResource.pData);
			
			m_adapters.back().m_context->Unmap(pBuffer,0);
		}

		template <class T>
		HRESULT createAppendConsumeBuffer(DWORD usage, DWORD access, UINT iNumElements, ID3D11Buffer** ppBuffer, ID3D11UnorderedAccessView** ppUAV, const T* pInitialData){
			HRESULT hr = S_OK;

			// Create SB
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory( &bufferDesc, sizeof(bufferDesc) );
			bufferDesc.ByteWidth = iNumElements * sizeof(T);
			bufferDesc.Usage = D3D11_USAGE(usage);
			bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = sizeof(T);
			bufferDesc.CPUAccessFlags = access;

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
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
			hr = m_adapters.back().m_deviceHandle->CreateUnorderedAccessView( *ppBuffer, &uavDesc, ppUAV );
			if(FAILED(hr))
				return hr;

			return hr;
		}

		template <class T>
		HRESULT createStructuredBufferUAV(DWORD usage, DWORD access, UINT iNumElements, ID3D11Buffer** ppBuffer, ID3D11UnorderedAccessView** ppUAV, const T* pInitialData){
			HRESULT hr = S_OK;

			// Create SB
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory( &bufferDesc, sizeof(bufferDesc) );
			bufferDesc.ByteWidth = iNumElements * sizeof(T);
			bufferDesc.Usage = D3D11_USAGE(usage);
			bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = sizeof(T);
			bufferDesc.CPUAccessFlags = access;

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
		HRESULT createStructuredBufferSRV(DWORD usage, DWORD access, UINT iNumElements, ID3D11Buffer** ppBuffer, ID3D11ShaderResourceView** ppUAV, const T* pInitialData){
			HRESULT hr = S_OK;

			// Create SB
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMemory( &bufferDesc, sizeof(bufferDesc) );
			bufferDesc.ByteWidth = iNumElements * sizeof(T);
			bufferDesc.Usage = D3D11_USAGE(usage);
			bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = sizeof(T);
			bufferDesc.CPUAccessFlags = access;

			D3D11_SUBRESOURCE_DATA bufferInitData;
			ZeroMemory( &bufferInitData, sizeof(bufferInitData) );
			bufferInitData.pSysMem = pInitialData;
			hr = m_adapters.back().m_deviceHandle->CreateBuffer( &bufferDesc, (pInitialData)? &bufferInitData : NULL, ppBuffer);
			if(FAILED(hr))
				return hr;

			// Create UAV
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory( &srvDesc, sizeof(srvDesc) );
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.NumElements = iNumElements;
			hr = m_adapters.back().m_deviceHandle->CreateShaderResourceView( *ppBuffer, &srvDesc, ppUAV );
			if(FAILED(hr))
				return hr;

			return hr;
		}

		template <class T>
		HRESULT createConstantBuffer(ID3D11Buffer** ppBuffer, const T* pInitialData){
			HRESULT hr = S_OK;

			// Fill in a buffer description.
			D3D11_BUFFER_DESC cbDesc;
			cbDesc.ByteWidth = 16*((sizeof(T)+15)/16);
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
			if(FAILED(hr))
				return hr;

			return hr;
		}

		HRESULT createComputeShader(std::string filename, ID3D11ComputeShader** ppShader);	
	private:
		void createShaders();

		std::vector<dxAdapter>	m_adapters;

		HWND m_hwnd;
	};
}

typedef boost::shared_ptr<SVM_Framework::DirectXManager> DirectXManagerPtr;