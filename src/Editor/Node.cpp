#include "Node.hpp"

#include "ImBridge/Parameter.hpp"
#include "imgui/imgui.h"

namespace oryon {

	Node::Node(const std::string& label, const std::shared_ptr<ImBridge::Bridge>& bridge)
		: _label(label), _bridge(bridge)
	{
		
	}

	void Node::render()
	{
		if (ImGui::TreeNodeEx(_label.c_str(), _openAtStart ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			auto iter = _bridge->getParametersBegin();

			while (iter != _bridge->getParametersEnd()) {
				iter->second->render();
				++iter;
			}

			ImGui::TreePop();
		}
	}
}