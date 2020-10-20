#include "CollisionDetection.hpp"

#include <limits>
#include <algorithm>

float CollisionDetection::Static2DSAT(const ConvexPolygon& cp1, const ConvexPolygon& cp2)
{
	assert(cp1.size() >= 3 && cp2.size() >= 3);

	float min_overlap = std::numeric_limits<float>::max();

	auto get_line_normal = [](const glm::vec2 x1, const glm::vec2 x2) {
		glm::vec2 line = x2 - x1;
		return glm::normalize(glm::vec2(-line.y, line.x));
	};

	auto get_projected_line = [](const glm::vec2& normal, const ConvexPolygon& cp) {
		float min_r = std::numeric_limits<float>::max();
		float max_r = -std::numeric_limits<float>::max();

		for (int i = 0; i < cp.size(); ++i) {
			float projected = glm::dot(cp[i], normal);
			min_r = std::min(min_r, projected);
			max_r = std::max(max_r, projected);
		}

		return glm::vec2(min_r, max_r);
	};

	auto get_line_overlap_amount = [](const glm::vec2& line1, const glm::vec2& line2) {
		return std::min(line1.y, line2.y) - std::max(line1.x, line2.x);
	};

	for (int k = 0; k < 2; ++k) {
		const ConvexPolygon* pcp1 = (k == 0) ? &cp1 : &cp2;
		const ConvexPolygon* pcp2 = (k == 0) ? &cp2 : &cp1;

		for (int i = 0; i < pcp1->size(); ++i) {
			glm::vec2 normal = get_line_normal((*pcp1)[i], (*pcp1)[(i + 1) % pcp1->size()]);
			glm::vec2 cp1_line = get_projected_line(normal, *pcp1);
			glm::vec2 cp2_line = get_projected_line(normal, *pcp2);

			if (IsLineOverlap(cp1_line, cp2_line)) {
				min_overlap = std::min(min_overlap, get_line_overlap_amount(cp1_line, cp2_line));
			} else {
				return 0.0f;
			}
		}
	}

	return min_overlap;
}

bool CollisionDetection::IsLineOverlap(const glm::vec2& line1, const glm::vec2& line2)
{
	return line1.y > line2.x && line1.x < line2.y;
}

bool CollisionDetection::ToLeft(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p)
{
	return glm::determinant(glm::transpose(glm::mat3(glm::vec3(p1, 1.0f), glm::vec3(p2, 1.0f), glm::vec3(p, 1.0f)))) > 0;
}

bool CollisionDetection::PointInConvexPolygon(const glm::vec2& point, const ConvexPolygon& polygon)
{
	assert(polygon.size() >= 3);

	bool to_left = ToLeft(polygon[0], polygon[1], point);

	for (int i = 1; i < polygon.size(); ++i) {
		if (to_left != ToLeft(polygon[i], polygon[(i + 1) % polygon.size()], point)) {
			return false;
		}
	}

	return true;
}
