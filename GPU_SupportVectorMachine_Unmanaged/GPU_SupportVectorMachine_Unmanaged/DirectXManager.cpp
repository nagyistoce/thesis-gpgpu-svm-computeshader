#include "stdafx.h"
#include "DirectXManager.h"
#include "ResourceManager.h"

namespace SVM_Framework{
	DirectXManager::DirectXManager(){
		
	}

	DirectXManager::~DirectXManager(){
		for(unsigned int i=0; i<m_adapters.size(); i++){
			m_adapters[i].m_context->Release();
			m_adapters[i].m_deviceHandle->Release();
		}
	}

	void DirectXManager::initialize(){
		UINT deviceFlags = 0;
		HRESULT result;
		IDXGIFactory* factory;
		IDXGIAdapter* adapter;
		IDXGIOutput* adapterOutput;
		DXGI_OUTPUT_DESC outputDesc;

#ifdef _DEBUG
		deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif
		// Create a DirectX graphics interface factory.
		result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		if(FAILED(result)){
			return;
		}

		unsigned int	adapterId = 0,
						adapterOutputId = 0;
		while(factory->EnumAdapters(adapterId, &adapter) == S_OK){
			adapterOutputId = 0;
			while(adapter->EnumOutputs(adapterOutputId,&adapterOutput) == S_OK){
				if(adapterOutput->GetDesc(&outputDesc) == S_OK){
					std::wstring name = outputDesc.DeviceName;
				}
				adapterOutput->Release();
				adapterOutputId++;
			}

			adapter->Release();
			adapterId++;
		}
		factory->Release();

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
									&m_adapters.back().m_featureLevel,
									&m_adapters.back().m_context))){
			// Error handling
			assert(0);
			return;
		}
	}

	void DirectXManager::launchComputation(int x, int y, int z){
		m_adapters.back().m_context->Dispatch(x,y,z);
	}

	void DirectXManager::setComputeShader(ID3D11ComputeShader* shader){
		m_adapters.back().m_context->CSSetShader(shader,NULL,0);
	}
	
	void DirectXManager::setComputeShaderConstantBuffers(std::vector<ID3D11Buffer*> &buffers){
		m_adapters.back().m_context->CSSetConstantBuffers(0,buffers.size(),&buffers[0]);
	}
	
	void DirectXManager::setComputeShaderUnorderedAccessViews(std::vector<ID3D11UnorderedAccessView*> &views){
		unsigned int viewInt = -1;
		m_adapters.back().m_context->CSSetUnorderedAccessViews(0,views.size(),&views[0],&viewInt);
	}

	HRESULT DirectXManager::createComputeShader(std::string filename, ID3D11ComputeShader** ppShader){
		boost::filesystem::path path = ResourceManager::findFilePath(filename);
		std::ifstream input(path.generic_string(),std::ios_base::binary);
		unsigned int size = boost::filesystem::file_size(path);

		char* buffer = new char[size];
		input.read(buffer,size);

		HRESULT hr = m_adapters.back().m_deviceHandle->CreateComputeShader(buffer,size,NULL,ppShader);
		delete[] buffer;
		input.close();

		return hr;
	}
}