#include "Panel.hpp"

#include "../imgui/imgui.h"


namespace oryon {

	Panel::Panel(const std::string& label, const std::vector<Node>& nodes)
        : _label(label), _nodes(nodes)
	{

	}

    void Panel::render()
    {
        if (ImGui::Begin(_label.c_str()))
        {
            for (auto& node : _nodes)
            {
                node.render();
            }
        }
        ImGui::End(); 
    }
}