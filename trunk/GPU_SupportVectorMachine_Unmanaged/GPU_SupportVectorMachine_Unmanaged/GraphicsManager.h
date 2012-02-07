#pragma once

namespace SVM_Framework{
	class GraphicsManager{
	public:
		virtual void initialize() = 0;
		virtual void launchComputation(int x, int y, int z) = 0;
	private:
	};
}

typedef boost::shared_ptr<SVM_Framework::GraphicsManager> GraphicsManagerPtr;