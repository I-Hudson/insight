/*
Copyright(c) 2016-2022 Panos Karabelas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Graphics/Frustum.h"

#include "Core/Profiler.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Insight
{
    namespace Graphics
    {
        Plane::Plane(const Maths::Vector4& normal)
        {
            this->normal = normal;
            this->d = 0.0f;
        }
        Plane::Plane(const Maths::Vector4& normal, float d)
        {
            this->normal = normal;
            this->d = d;
        }

        Plane Plane::Normalize()
        {
            float lengthSq = normal.LengthSquared();
            // Prevent divide by zero
            if (lengthSq > 0.0f)
            {
                lengthSq = 1.0f / lengthSq;
            }
            normal = Maths::Vector4(
                normal[0] * lengthSq,
                normal[1] * lengthSq,
                normal[2] * lengthSq,
                normal[3] * lengthSq);

            return *this;
        }


        float Plane::Dot(const Maths::Vector4& v) const
        {
            return (normal.x * v.x) + (normal.y * v.y) + (normal.z * v.z);
        }

        float Plane::Dot(const Plane& p, const Maths::Vector4& v)
        {
            const Plane& newPlane = p;
            return newPlane.Dot(v);
        }

        //// <summary>
        //// Frustum
        //// </summary>
        //// <param name="view"></param>
        //// <param name="projection"></param>
        //// <param name="screen_depth"></param>
        Frustum::Frustum(const glm::mat4& view, const glm::mat4& projection, float screen_depth)
            : m_view(view)
            , m_projection(projection)
        {
            IS_PROFILE_FUNCTION();
            // Calculate the minimum Z distance in the frustum.
            const float z_min = -projection[3][2] / projection[2][2];
            const float r = screen_depth / (screen_depth - z_min);
            glm::mat4 projection_updated = projection;
            projection_updated[2][2] = r;
            projection_updated[3][2] = -r * z_min;

            // Create the frustum matrix from the view matrix and updated projection matrix.
            const glm::mat4 projection_view = projection_updated * glm::inverse(view);

            // Calculate near plane of frustum.
            m_planes[0].normal.x = projection_view[0][3] + projection_view[0][2];
            m_planes[0].normal.y = projection_view[1][3] + projection_view[1][2];
            m_planes[0].normal.z = projection_view[2][3] + projection_view[2][2];
            m_planes[0].d = projection_view[3][3] + projection_view[3][2];
            m_planes[0].Normalize();

            // Calculate far plane of frustum.
            m_planes[1].normal.x = projection_view[0][3] - projection_view[0][2];
            m_planes[1].normal.y = projection_view[1][3] - projection_view[1][2];
            m_planes[1].normal.z = projection_view[2][3] - projection_view[2][2];
            m_planes[1].d = projection_view[3][3] - projection_view[3][2];
            m_planes[1].Normalize();

            // Calculate left plane of frustum.
            m_planes[2].normal.x = projection_view[0][3] + projection_view[0][0];
            m_planes[2].normal.y = projection_view[1][3] + projection_view[1][0];
            m_planes[2].normal.z = projection_view[2][3] + projection_view[2][0];
            m_planes[2].d = projection_view[3][3] + projection_view[3][0];
            m_planes[2].Normalize();

            // Calculate right plane of frustum.
            m_planes[3].normal.x = projection_view[0][3] - projection_view[0][0];
            m_planes[3].normal.y = projection_view[1][3] - projection_view[1][0];
            m_planes[3].normal.z = projection_view[2][3] - projection_view[2][0];
            m_planes[3].d = projection_view[3][3] - projection_view[3][0];
            m_planes[3].Normalize();

            // Calculate top plane of frustum.
            m_planes[4].normal.x = projection_view[0][3] - projection_view[0][1];
            m_planes[4].normal.y = projection_view[1][3] - projection_view[1][1];
            m_planes[4].normal.z = projection_view[2][3] - projection_view[2][1];
            m_planes[4].d = projection_view[3][3] - projection_view[3][1];
            m_planes[4].Normalize();

            // Calculate bottom plane of frustum.
            m_planes[5].normal.x = projection_view[0][3] + projection_view[0][1];
            m_planes[5].normal.y = projection_view[1][3] + projection_view[1][1];
            m_planes[5].normal.z = projection_view[2][3] + projection_view[2][1];
            m_planes[5].d = projection_view[3][3] + projection_view[3][1];
            m_planes[5].Normalize();

            // Calculate near plane of frustum.
            //m_planes[0].normal.x = projection_view[3][0] + projection_view[2][0];
            //m_planes[0].normal.y = projection_view[3][1] + projection_view[2][1];
            //m_planes[0].normal.z = projection_view[3][2] + projection_view[2][2];
            //m_planes[0].d = projection_view[3][3] + projection_view[2][3];
            //m_planes[0].Normalize();
            //
            //// Calculate far plane of frustum.
            //m_planes[1].normal.x = projection_view[3][0] - projection_view[2][0];
            //m_planes[1].normal.y = projection_view[3][1] - projection_view[2][1];
            //m_planes[1].normal.z = projection_view[3][1] - projection_view[2][2];
            //m_planes[1].d = projection_view[3][3] - projection_view[2][3];
            //m_planes[1].Normalize();
            //
            //// Calculate left plane of frustum.
            //m_planes[2].normal.x = projection_view[3][1] + projection_view[0][1];
            //m_planes[2].normal.y = projection_view[3][2] + projection_view[0][2];
            //m_planes[2].normal.z = projection_view[3][3] + projection_view[0][3];
            //m_planes[2].d = projection_view[3][3] + projection_view[0][3];
            //m_planes[2].Normalize();
            //
            //// Calculate right plane of frustum.
            //m_planes[3].normal.x = projection_view[3][0] - projection_view[0][0];
            //m_planes[3].normal.y = projection_view[3][1] - projection_view[0][1];
            //m_planes[3].normal.z = projection_view[3][2] - projection_view[0][2];
            //m_planes[3].d = projection_view[3][3] - projection_view[0][3];
            //m_planes[3].Normalize();
            //
            //// Calculate top plane of frustum.
            //m_planes[4].normal.x = projection_view[3][0] - projection_view[1][0];
            //m_planes[4].normal.y = projection_view[3][1] - projection_view[1][1];
            //m_planes[4].normal.z = projection_view[3][2] - projection_view[1][2];
            //m_planes[4].d = projection_view[3][3] - projection_view[1][3];
            //m_planes[4].Normalize();
            //
            //// Calculate bottom plane of frustum.
            //m_planes[5].normal.x = projection_view[3][0] + projection_view[1][0];
            //m_planes[5].normal.y = projection_view[3][1] + projection_view[1][1];
            //m_planes[5].normal.z = projection_view[3][2] + projection_view[1][2];
            //m_planes[5].d = projection_view[3][3] + projection_view[1][3];
            //m_planes[5].Normalize();
        }

        Frustum::Frustum(const Maths::Matrix4& view, const Maths::Matrix4& projection, float screenDepth)
        {
            IS_PROFILE_FUNCTION();
            // Calculate the minimum Z distance in the frustum.
            const float z_min = -projection[3][2] / projection[2][2];
            const float r = screenDepth / (screenDepth - z_min);
            Maths::Matrix4 projection_updated = projection;
            projection_updated[2][2] = r;
            projection_updated[3][2] = -r * z_min;

            // Create the frustum matrix from the view matrix and updated projection matrix.
            const Maths::Matrix4 projection_view = projection_updated * view.Inversed();

            // Calculate near plane of frustum.
            m_planes[0].normal.x = projection_view[0][3] + projection_view[0][2];
            m_planes[0].normal.y = projection_view[1][3] + projection_view[1][2];
            m_planes[0].normal.z = projection_view[2][3] + projection_view[2][2];
            m_planes[0].d = projection_view[3][3] + projection_view[3][2];
            m_planes[0].Normalize();

            // Calculate far plane of frustum.
            m_planes[1].normal.x = projection_view[0][3] - projection_view[0][2];
            m_planes[1].normal.y = projection_view[1][3] - projection_view[1][2];
            m_planes[1].normal.z = projection_view[2][3] - projection_view[2][2];
            m_planes[1].d = projection_view[3][3] - projection_view[3][2];
            m_planes[1].Normalize();

            // Calculate left plane of frustum.
            m_planes[2].normal.x = projection_view[0][3] + projection_view[0][0];
            m_planes[2].normal.y = projection_view[1][3] + projection_view[1][0];
            m_planes[2].normal.z = projection_view[2][3] + projection_view[2][0];
            m_planes[2].d = projection_view[3][3] + projection_view[3][0];
            m_planes[2].Normalize();

            // Calculate right plane of frustum.
            m_planes[3].normal.x = projection_view[0][3] - projection_view[0][0];
            m_planes[3].normal.y = projection_view[1][3] - projection_view[1][0];
            m_planes[3].normal.z = projection_view[2][3] - projection_view[2][0];
            m_planes[3].d = projection_view[3][3] - projection_view[3][0];
            m_planes[3].Normalize();

            // Calculate top plane of frustum.
            m_planes[4].normal.x = projection_view[0][3] - projection_view[0][1];
            m_planes[4].normal.y = projection_view[1][3] - projection_view[1][1];
            m_planes[4].normal.z = projection_view[2][3] - projection_view[2][1];
            m_planes[4].d = projection_view[3][3] - projection_view[3][1];
            m_planes[4].Normalize();

            // Calculate bottom plane of frustum.
            m_planes[5].normal.x = projection_view[0][3] + projection_view[0][1];
            m_planes[5].normal.y = projection_view[1][3] + projection_view[1][1];
            m_planes[5].normal.z = projection_view[2][3] + projection_view[2][1];
            m_planes[5].d = projection_view[3][3] + projection_view[3][1];
            m_planes[5].Normalize();
        }

        Frustum::Frustum(const Maths::Matrix4& viewProjection)
        {
            IS_PROFILE_FUNCTION();
            // We are interested in columns of the matrix, so transpose because we can access only rows:
            const Maths::Matrix4 mat = viewProjection.Inversed();

            // Near plane:
            m_planes[0] = mat[2];
            m_planes[0].Normalize();

            // Far plane:
            m_planes[1] = mat[3] - mat[2];
            m_planes[1].Normalize();

            // Left plane:
            m_planes[2] = mat[3] + mat[0];
            m_planes[2].Normalize();

            // Right plane:
            m_planes[3] = mat[3] - mat[0];
            m_planes[3].Normalize();

            // Top plane:
            m_planes[4] = mat[3] - mat[1];
            m_planes[4].Normalize();

            // Bottom plane:
            m_planes[5] = mat[3] + mat[1];
            m_planes[5].Normalize();
        }


        bool Frustum::IsVisible(const glm::vec3& center, const glm::vec3& extent, bool ignore_near_plane /*= false*/) const
        {
            IS_PROFILE_FUNCTION();
            float radius = 0.0f;

            if (!ignore_near_plane)
            {
                radius = glm::max(extent.x, glm::max(extent.y, extent.z));
            }
            else
            {
                constexpr float z = std::numeric_limits<float>::infinity(); /// reverse-z only (but I must read form Renderer)
                radius = glm::max(extent.x, glm::max(extent.y, z));
            }
            radius = std::abs(radius);

            /// Check sphere first as it's cheaper
            if (CheckSphere(center, radius) != Intersection::Outside)
                return true;

            if (CheckCube(center, extent) != Intersection::Outside)
                return true;

            return false;
        }

        bool Frustum::IsVisible(const Graphics::BoundingBox& boundingbox) const
        {
            return IsVisible(boundingbox.GetCenter(), boundingbox.GetExtents());
        }

        std::array<glm::vec3, 8> Frustum::GetWorldPoints() const
        {
            glm::mat4 projViewMatrix = m_projection * glm::inverse(m_view);
            glm::mat4 invProjViewMatrix = glm::inverse(projViewMatrix);

            glm::mat4 invProjection = glm::inverse(m_projection);
            glm::mat4 invView = glm::inverse(m_view);
            glm::mat4 world = invProjection * invView;

            std::array<glm::vec3, 8> points =
            {
                glm::vec3(-1,  1, -1),
                glm::vec3( 1,  1, -1),
                glm::vec3( 1, -1, -1),
                glm::vec3(-1, -1, -1),

                glm::vec3(-1,  1, 1),
                glm::vec3( 1,  1, 1),
                glm::vec3( 1, -1, 1),
                glm::vec3(-1, -1, 1),
            };

            for (glm::vec3& point : points)
            {
                glm::vec4 invPoint = invProjViewMatrix * glm::vec4(point, 1.0f);
                point = invPoint / invPoint.w;
            }
            return points;
        }

        Intersection Frustum::CheckCube(const glm::vec3& center, const glm::vec3& extent) const
        {
            IS_PROFILE_FUNCTION();
            Intersection result = Intersection::Inside;
            Plane plane_abs;

            // Check if any one point of the cube is in the view frustum.

            const Maths::Vector4 max(Maths::Vector3(center) + Maths::Vector3(extent));
            const Maths::Vector4 min(Maths::Vector3(center) - Maths::Vector3(extent));
            u8 boundingBoxPlanesWithIn = 0;

            for (const Plane& plane : m_planes)
            {
                const Maths::Vector4 lt(
                    plane.normal[0] < Maths::Vector4::Zero[0] ? 1 : 0,
                    plane.normal[1] < Maths::Vector4::Zero[1] ? 1 : 0,
                    plane.normal[2] < Maths::Vector4::Zero[2] ? 1 : 0,
                    plane.normal[3] < Maths::Vector4::Zero[3] ? 1 : 0
                );

                const Maths::Vector4 furthestFromPlane(
                    lt[0] == 0 ? max[0] : min[0],
                    lt[1] == 0 ? max[1] : min[1],
                    lt[2] == 0 ? max[2] : min[2],
                    lt[3] == 0 ? max[3] : min[3]);

                const float planeDot = plane.Dot(furthestFromPlane);
                if (planeDot < 0.0f)
                {
                    return Intersection::Outside;
                }
                //++boundingBoxPlanesWithIn;

                //plane_abs.normal = glm::abs(plane.normal);
                //plane_abs.d = plane.d;
                //
                //const float d = center.x * plane.normal.x + center.y * plane.normal.y + center.z * plane.normal.z;
                //const float r = extent.x * plane_abs.normal.x + extent.y * plane_abs.normal.y + extent.z * plane_abs.normal.z;
                //
                //const float d_p_r = d + r;
                //const float d_m_r = d - r;
                //
                //if (d_p_r < -plane.d)
                //{
                //    result = Intersection::Outside;
                //    break;
                //}
                //
                //if (d_m_r < -plane.d)
                //{
                //    result = Intersection::Intersects;
                //}
            }

            //if (boundingBoxPlanesWithIn == 0)
            //{
            //    return Intersection::Outside;
            //}
            //else if (boundingBoxPlanesWithIn == 0 < 6)
            //{
            //    return Intersection::Intersects;
            //}
            //else
            //{
            //    return Intersection::Inside;
            //}

            return result;
        }

        Intersection Frustum::CheckSphere(const glm::vec3& center, float radius) const
        {
            IS_PROFILE_FUNCTION();
            return Intersection::Outside;
            // calculate our distances to each of the planes
            //for (const auto& plane : m_planes)
            //{
            //    // find the distance to this plane
            //    const float distance = glm::dot(plane.normal, center) + plane.d;
            //
            //    // if this distance is < -sphere.radius, we are outside
            //    if (distance < -radius)
            //    {
            //        return Intersection::Outside;
            //    }
            //
            //    // else if the distance is between +- radius, then we intersect
            //    if (static_cast<float>(glm::abs(distance)) < radius)
            //    {
            //        return Intersection::Intersects;
            //    }
            //}
            //
            //// otherwise we are fully in view
            //return Intersection::Inside;
        }
    }
}