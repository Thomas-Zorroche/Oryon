#include "Node.hpp"

#include "ImBridge/Parameter.hpp"

namespace oryon {

	Node::Node(const std::string& label, const std::shared_ptr<ImBridge::Bridge>& bridge)
		: _label(label), _bridge(bridge)
	{
		
	}

	void Node::render()
	{
		for (auto& parameter : _bridge->getParameters())
		{
			parameter->render();
		}
	}
}