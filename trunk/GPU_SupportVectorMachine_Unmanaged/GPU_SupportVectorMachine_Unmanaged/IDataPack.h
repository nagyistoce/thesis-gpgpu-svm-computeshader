#pragma once
#include "GraphicsManager.h"
#include "ResourceManager.h"
#include "GUIManager.h"

namespace SVM_Framework{
	class GUIManager;
	class IAlgorithm;

	class IDataPack{
	public:
		GraphicsManagerPtr m_gfxMgr;
		ResourceManagerPtr m_recMgr;
		GUIManagerPtr m_gui;

		boost::shared_ptr<boost::function<void (boost::shared_ptr<IAlgorithm> a)>> m_callBack;
	};
}

typedef boost::shared_ptr<SVM_Framework::IDataPack> IDataPackPtr;