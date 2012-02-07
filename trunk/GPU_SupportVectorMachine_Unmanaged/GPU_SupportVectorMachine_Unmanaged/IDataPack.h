#pragma once
#include "GraphicsManager.h"
#include "ResourceManager.h"

namespace SVM_Framework{
	class GUIManager;

	class IDataPack{
	public:
		GraphicsManagerPtr m_gfxMgr;
		ResourceManagerPtr m_recMgr;
		GUIManager *m_gui;
	};
}

typedef boost::shared_ptr<SVM_Framework::IDataPack> IDataPackPtr;