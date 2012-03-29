#pragma once
#include "AlgorithmDataPack.h"
#include "IEvaluation.h"

namespace SVM_Framework{
	class IAlgorithm : public boost::enable_shared_from_this<IAlgorithm>{
	public:
		typedef Value::v_precision alg_precision;

		IAlgorithm():m_stop(false){}
		virtual ~IAlgorithm(){}
		virtual void run(AlgorithmDataPackPtr data) = 0;

		struct ResultsPack{
			ResultsPack():	cl1Correct(0),
							cl1Wrong(0),
							cl2Correct(0),
							cl2Wrong(0),
							iterations(0),
							cacheHits(0),
							kernelEvals(0),
							supportVectors(0),
							totalTime(0.0),
							trainingTime(0.0),
							testingTime(0.0),
							cl1EnrichmentFactor(0.0),
							cl2EnrichmentFactor(0.0),
							balancedErrorRate(0.0),
							accuracy(0.0),
							divisor(0),
							trainingInstances(0),
							testingInstances(0)
			{}

			long	cl1Correct,
					cl1Wrong,
					cl2Correct,
					cl2Wrong,
					iterations,
					cacheHits,
					kernelEvals,
					supportVectors,
					trainingInstances,
					testingInstances;
			
			alg_precision	totalTime,
							trainingTime,
							testingTime;

			alg_precision	cl1EnrichmentFactor,
							cl2EnrichmentFactor,
							accuracy,
							balancedErrorRate;

			unsigned int divisor;
		};

		std::wstringstream& getoutputStream()	{return m_outputStream;}
		ResultsPack& getOutputResults()			{return m_outputResults;}

		void stop() { m_stop = true; }
		void start() { m_stop = false; }
	protected:
		ResultsPack m_outputResults;
		std::wstringstream m_outputStream;

		bool m_stop;
	};
}

typedef boost::shared_ptr<SVM_Framework::IAlgorithm> IAlgorithmPtr;