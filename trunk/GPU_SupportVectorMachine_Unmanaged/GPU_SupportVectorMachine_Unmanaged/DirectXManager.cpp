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

		//D3D11_FEATURE_DATA_DOUBLES doubleSupport;
		//m_adapters.back().m_deviceHandle->CheckFeatureSupport(D3D11_FEATURE_DOUBLES,&doubleSupport,sizeof(doubleSupport));
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

	void DirectXManager::setComputeShaderResourceViews(std::vector<ID3D11ShaderResourceView*> &views){
		m_adapters.back().m_context->CSSetShaderResources(0,views.size(),&views[0]);
	}
	
	void DirectXManager::setComputeShaderUnorderedAccessViews(std::vector<ID3D11UnorderedAccessView*> &views){
		unsigned int viewInt = -1;
		m_adapters.back().m_context->CSSetUnorderedAccessViews(0,views.size(),&views[0],&viewInt);
	}

	void DirectXManager::setComputeShaderConstantBuffers(std::map<int,ID3D11Buffer*> &buffers){
		std::vector<ID3D11Buffer*> setVec;
		std::map<int,ID3D11Buffer*>::iterator itr = buffers.begin();

		while(itr != buffers.end()){
			setVec.push_back(itr->second);
			itr++;
		}

		setComputeShaderConstantBuffers(setVec);
	}
	
	void DirectXManager::setComputeShaderResourceViews(std::map<int,ID3D11ShaderResourceView*> &views){
		std::vector<ID3D11ShaderResourceView*> setVec;
		std::map<int,ID3D11ShaderResourceView*>::iterator itr = views.begin();

		while(itr != views.end()){
			setVec.push_back(itr->second);
			itr++;
		}

		setComputeShaderResourceViews(setVec);
	}

	void DirectXManager::setComputeShaderUnorderedAccessViews(std::map<int,ID3D11UnorderedAccessView*> &views){
		std::vector<ID3D11UnorderedAccessView*> setVec;
		std::map<int,ID3D11UnorderedAccessView*>::iterator itr = views.begin();

		while(itr != views.end()){
			setVec.push_back(itr->second);
			itr++;
		}

		setComputeShaderUnorderedAccessViews(setVec);
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