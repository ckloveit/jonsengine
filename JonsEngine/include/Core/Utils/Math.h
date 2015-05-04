#pragma once

#include "include/Core/Types.h"

#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <vector>
#include <math.h>


namespace JonsEngine
{
    /*
     * Contains various math utilities
     */
     
    struct Transform : Mat4
    {
        typedef std::vector<Transform>::size_type Index;
    };

    struct AABB
    {
        typedef std::vector<AABB>::size_type Index;

        AABB(const Vec3& center, const Vec3& extent);

		AABB& operator*=(const Mat4& transform);

        Vec3 Min() const;
        Vec3 Max() const;


        Vec3 mAABBCenter;
        Vec3 mAABBExtent;
    };

    AABB operator*(const Mat4& transform, const AABB& aabb);
    AABB operator*(const AABB& aabb, const Mat4& transform);

    enum class AABBIntersection
    {
        AABB_INTERSECTION_INSIDE,
        AABB_INTERSECTION_PARTIAL,
        AABB_INTERSECTION_OUTSIDE
    };

    // note: make sure frustrum and aabb are in the same space
    AABBIntersection IsAABBInFrustum(const AABB& aabb, const Mat4& frustumMatrix);
    AABBIntersection IsAABBInSphere(const AABB& aabb, const Vec3& sphereCentre, const float sphereRadius);
    // "is target in source"
    AABBIntersection IsAABBInAABB(const AABB& target, const AABB& source);
	bool IsPointInSphere(const Vec3& point, const Vec3& sphereCentre, const float radius);
    bool IsPointInAABB(const Vec3& point, const AABB& aabb);

    Mat4 PerspectiveMatrixFov(const float fovDegrees, const float ratio, const float zNear, const float zFar);
    Mat4 OrthographicMatrix(const float left, const float right, const float top, const float bottom, const float zNear, const float zFar);

    Vec3 MinVal(const Vec3& v1, const Vec3& v2);
    Vec3 MaxVal(const Vec3& v1, const Vec3& v2);

    const Mat4 gIdentityMatrix(1.0f);
}