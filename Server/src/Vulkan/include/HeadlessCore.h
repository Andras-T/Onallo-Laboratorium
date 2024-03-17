#pragma once
#include "Core.h"

namespace Server {

	class HeadlessCore : public Core {
	public:

		virtual void init();

	protected:

		void send() override { logger->LogWarning("HeadlessCore::Send Not implemented yet!"); }
		void cleanUp() override;
		void mainLoop() override;
		void createRenderPass(VkFormat imageformat) override;
	};
}