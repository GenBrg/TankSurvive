#pragma once

#include <glm/glm.hpp>

#include <vector>

using ConvexPolygon = std::vector<glm::vec2>;

class CollisionDetection {
public:
	static float Static2DSAT(const ConvexPolygon& cp1, const ConvexPolygon& cp2);
	static bool IsLineOverlap(const glm::vec2& line1, const glm::vec2& line2);

	// Check if p is to the left of line p1-p2
	static bool ToLeft(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p);
	static bool PointInConvexPolygon(const glm::vec2& point, const ConvexPolygon& polygon);
};
