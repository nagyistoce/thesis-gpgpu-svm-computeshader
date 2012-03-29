#pragma once
#include "Value.h"
#include "IAlgorithm.h"
#include "MainFramework.h"
#include "ISVM.h"

namespace SVM_Framework{
	class SearchScheme{
	public:
		SearchScheme(MainFrameworkPtr framework);

		void run();
		void stop();
	protected:
		virtual bool callbackProcessing(IAlgorithmPtr algorithm) = 0;
		virtual std::string printProcess() = 0;
		virtual void runProcess() = 0;

		void printResults();
		void callback(IAlgorithmPtr algorithm);

		bool	m_stop;
		MainFrameworkPtr m_framework;

		std::wstringstream m_output;
		std::vector<ISVM::ResultsPack> m_resPacks;
	};
}