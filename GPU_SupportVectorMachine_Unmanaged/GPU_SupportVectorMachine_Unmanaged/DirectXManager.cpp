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

		bool multiCard = false;
		unsigned int	adapterId = 0,
						adapterOutputId = 0;
		while(factory->EnumAdapters(adapterId, &adapter) == S_OK){
			// Create devices
			m_adapters.push_back(dxAdapter());
			adapterOutputId = 0;
			while(adapter->EnumOutputs(adapterOutputId,&adapterOutput) == S_OK){
				if(adapterOutput->GetDesc(&outputDesc) == S_OK){
					m_adapters.back().m_name = outputDesc.DeviceName;
				}
				adapterOutput->Release();
				adapterOutputId++;
			}

			if(FAILED( result =	D3D11CreateDevice(adapter,
										D3D_DRIVER_TYPE_UNKNOWN,
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

			adapter->Release();
			adapterId++;
			if(!multiCard)
				break;
		}
		factory->Release();

		//D3D11_FEATURE_DATA_DOUBLES doubleSupport;
		//m_adapters.back().m_deviceHandle->CheckFeatureSupport(D3D11_FEATURE_DOUBLES,&doubleSupport,sizeof(doubleSupport));
	}

	void DirectXManager::launchComputation(int devId, int x, int y, int z){
		m_adapters[devId].m_context->Dispatch(x,y,z);
	}

	void DirectXManager::setComputeShader(int devId, ID3D11ComputeShader* shader){
		m_adapters[devId].m_context->CSSetShader(shader,NULL,0);
	}
	
	void DirectXManager::setComputeShaderConstantBuffers(int devId, std::vector<ID3D11Buffer*> &buffers){
		m_adapters[devId].m_context->CSSetConstantBuffers(0,buffers.size(),&buffers[0]);
	}

	void DirectXManager::setComputeShaderResourceViews(int devId, std::vector<ID3D11ShaderResourceView*> &views){
		m_adapters[devId].m_context->CSSetShaderResources(0,views.size(),&views[0]);
	}
	
	void DirectXManager::setComputeShaderUnorderedAccessViews(int devId, std::vector<ID3D11UnorderedAccessView*> &views){
		unsigned int viewInt = -1;
		m_adapters[devId].m_context->CSSetUnorderedAccessViews(0,views.size(),&views[0],&viewInt);
	}

	void DirectXManager::setComputeShaderConstantBuffers(int devId, std::map<int,ID3D11Buffer*> &buffers){
		std::vector<ID3D11Buffer*> setVec;
		std::map<int,ID3D11Buffer*>::iterator itr = buffers.begin();

		while(itr != buffers.end()){
			setVec.push_back(itr->second);
			itr++;
		}

		setComputeShaderConstantBuffers(devId,setVec);
	}
	
	void DirectXManager::setComputeShaderResourceViews(int devId, std::map<int,ID3D11ShaderResourceView*> &views){
		std::vector<ID3D11ShaderResourceView*> setVec;
		std::map<int,ID3D11ShaderResourceView*>::iterator itr = views.begin();

		while(itr != views.end()){
			setVec.push_back(itr->second);
			itr++;
		}

		setComputeShaderResourceViews(devId,setVec);
	}

	void DirectXManager::setComputeShaderUnorderedAccessViews(int devId, std::map<int,ID3D11UnorderedAccessView*> &views){
		std::vector<ID3D11UnorderedAccessView*> setVec;
		std::map<int,ID3D11UnorderedAccessView*>::iterator itr = views.begin();

		while(itr != views.end()){
			setVec.push_back(itr->second);
			itr++;
		}

		setComputeShaderUnorderedAccessViews(devId,setVec);
	}

	HRESULT DirectXManager::createComputeShader(int devId, std::string filename, ID3D11ComputeShader** ppShader){
		boost::filesystem::path path = ResourceManager::findFilePath(filename);
		std::ifstream input(path.generic_string(),std::ios_base::binary);
		unsigned int size = boost::filesystem::file_size(path);

		char* buffer = new char[size];
		input.read(buffer,size);

		HRESULT hr = m_adapters[devId].m_deviceHandle->CreateComputeShader(buffer,size,NULL,ppShader);
		delete[] buffer;
		input.close();

		return hr;
	}
}