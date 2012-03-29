#include "stdafx.h"
#include "SearchScheme.h"
#include "StartAlgorithmMessage.h"

namespace SVM_Framework{
	SearchScheme::SearchScheme(MainFrameworkPtr framework):m_stop(false),m_framework(framework){

	}

	void SearchScheme::run(){
		m_stop = false;

		runProcess();

		AlgorithmDataPackPtr data = AlgorithmDataPackPtr(new SVM_Framework::AlgorithmDataPack);
		data->m_algoName = m_framework->getGuiPtr()->getEditText(IDC_COMBO_ALGO);
		std::stringstream ss;
		ss << m_framework->getGuiPtr()->getEditText(IDC_EDIT_FILEPATH);
		data->m_dataResource = ss.str();
		data->m_callBack = boost::shared_ptr<boost::function<void (IAlgorithmPtr a)>>(new boost::function<void (IAlgorithmPtr a)>(
			std::bind1st(std::mem_fun(&SearchScheme::callback), this)));

		m_framework->postMessage(StartAlgorithmMessagePtr(new SVM_Framework::StartAlgorithmMessage(data)));
	}

	void SearchScheme::stop(){
		m_stop = true;
	}

	void SearchScheme::printResults(){
		std::string head = printProcess();

		if(!m_output.str().empty()){
			std::stringstream ss;
			ss << m_framework->getGuiPtr()->getEditText(IDC_EDIT_FILEPATH);
			boost::filesystem::path p = ss.str();
		
			std::ios_base::openmode mode = std::ios_base::app;
			std::wofstream out(head+std::string(p.filename().generic_string())+".txt",mode);
			out.write(m_output.str().c_str(),m_output.str().size());
			out.close();

			m_output = std::wstringstream();
		}
	}

	void SearchScheme::callback(IAlgorithmPtr algorithm){
		if(m_stop){
			printResults();
			return;
		}

		m_resPacks.push_back(algorithm->getOutputResults());
		
		if(callbackProcessing(algorithm)){
			printResults();
			run();
		}
		else{
			printResults();
		}
	}
}