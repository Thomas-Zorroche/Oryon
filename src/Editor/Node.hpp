#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ImBridge/Bridge.hpp"

namespace oryon {

	class Node
	{
	public:
		Node(const std::string& label, const std::shared_ptr<ImBridge::Bridge>& bridge);

		void render();

	private:
		std::string _label;
		std::shared_ptr<ImBridge::Bridge> _bridge;
	};
}