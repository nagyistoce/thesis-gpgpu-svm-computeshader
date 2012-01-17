#include "stdafx.h"
#include "DirectXManager.h"
#include "ResourceManager.h"

namespace SVM_Framework{
	DirectXManager::DirectXManager(){
		
	}

	DirectXManager::~DirectXManager(){
		std::map<std::string,ID3D11ComputeShader*>::iterator itr = m_computeShaders.begin();
		while(itr != m_computeShaders.end()){
			itr->second->Release();
			itr++;
		}
		m_accesView->Release();
		m_gfxBuffer->Release();
		for(unsigned int i=0; i<m_adapters.size(); i++){
			m_adapters[i].m_context->Release();
			m_adapters[i].m_deviceHandle->Release();
		}
	}

	void DirectXManager::initialize(){
		UINT deviceFlags = 0;
#ifdef _DEBUG
		deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

		// Create devices
		m_adapters.push_back(dxAdapter());
		if(FAILED(D3D11CreateDevice(NULL,
									D3D_DRIVER_TYPE_HARDWARE,
									NULL,
									deviceFlags,
									NULL,
									0,
									D3D11_SDK_VERSION,
									&m_adapters.back().m_deviceHandle,
									&m_featureLevel,
									&m_adapters.back().m_context))){
			// Error handling
			assert(0);
		}

		createShaders();
	}

	void DirectXManager::launchComputation(std::string shader, int x, int y, int z){
		std::map<std::string,ID3D11ComputeShader*>::iterator itr;
		if((itr = m_computeShaders.find(shader)) != m_computeShaders.end()){
			UINT views = -1;
			m_adapters.back().m_context->CSSetShader(itr->second,NULL,0);
			m_adapters.back().m_context->CSSetUnorderedAccessViews(0,1,&m_accesView,&views);
			m_adapters.back().m_context->Dispatch(x,y,z);
		}
	}

	void DirectXManager::createShaders(){
		// Create buffers
		if(FAILED(createStructuredBuffer<bufferStruct>(10,&m_gfxBuffer,&m_accesView))){
			assert(0);
		}

		// Create compute shaders
		std::vector<boost::filesystem::path> paths = ResourceManager::getFilesInFolder(ResourceManager::getResourcePath()+"Shaders");
		for(unsigned int i=0; i<paths.size(); i++){
			std::ifstream input(paths[i].generic_string(),std::ios_base::binary);
			unsigned int size = boost::filesystem::file_size(paths[i]);
			ID3D11ComputeShader* compShader;

			char* buffer = new char[size];
			input.read(buffer,size);

			HRESULT hr = m_adapters.back().m_deviceHandle->CreateComputeShader(buffer,size,NULL,&compShader);
			if(!FAILED(hr)){
				m_computeShaders[paths[i].filename().generic_string()] = compShader;
			}

			delete[] buffer;
			input.close();
		}
	}

	template <class T>
	HRESULT DirectXManager::createStructuredBuffer(UINT iNumElements, ID3D11Buffer** ppBuffer, ID3D11UnorderedAccessView** ppUAV, const T* pInitialData){
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
}