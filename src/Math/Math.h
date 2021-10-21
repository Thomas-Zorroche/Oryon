#pragma once

#include <glm/glm.hpp>

namespace oryon::math {

	bool decomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

}
