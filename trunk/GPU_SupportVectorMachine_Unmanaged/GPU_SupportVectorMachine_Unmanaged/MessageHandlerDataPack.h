#pragma once

namespace SVM_Framework{
	class MessageHandlerDataPack{
	public:
		GraphicsManagerPtr m_gfxMgr;
		ResourceManagerPtr m_recMgr;
	};
}

typedef boost::shared_ptr<SVM_Framework::MessageHandlerDataPack> MessageHandlerDataPackPtr;