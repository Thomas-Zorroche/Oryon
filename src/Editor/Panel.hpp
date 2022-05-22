#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Node.hpp"

namespace oryon {


	class Panel
	{
	public:
		Panel(const std::string& label, const std::vector<Node>& nodes);
		~Panel() = default;

		void render();

	private:
		std::string _label;
		std::vector<Node> _nodes;
	};
}