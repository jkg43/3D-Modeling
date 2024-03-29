#include "Constants.h"
#include <float.h>
using namespace glm;

void arcballRotation(const vec2& start, const vec2& end, quat& orientation)
{
    // Calculate the rotation axis and angle
    vec3 startVec = getArcballVector(start);
    vec3 endVec = getArcballVector(end);
    vec3 rotAxis = cross(startVec, endVec);
    float rotAngle = acos(dot(startVec, endVec));

    // Update the orientation quaternion
    orientation = normalize(angleAxis(rotAngle, rotAxis) * orientation);
}

vec3 getArcballVector(const vec2& point)
{
    // Scale the point to the range [-1, 1]
    vec3 p(point.x / WIDTH * 2 - 1, point.y / HEIGHT * 2 - 1, 0);

    // Flip the y-axis
    p.y *= -1;

    // Calculate the length of the vector
    float length = glm::length(p);

    // If the point is outside the arcball, set its z-coordinate to zero
    if (length > 1) {
        p = normalize(p);
    }
    else {
        p.z = sqrt(1 - length * length);
    }

    return p;
}

vec4 convertScreenVectorToWorldVector(vec4 vec)
{
    vec = inverse(vg.ubo.projXview) * vec;

    return vec / vec.w;

}

vec3 getRayFromScreenPos(vec2 pos)
{
    mat4 inverse = glm::inverse(vg.ubo.projXview);

    int halfW = vg.swapChainExtent.width / 2, halfH = vg.swapChainExtent.height / 2;

    vec4 near = vec4((pos.x - halfW) / halfW,
        (pos.y - halfH) / halfH,
        0, 1.0);
    vec4 far = vec4((pos.x - halfW) / halfW,
        (pos.y - halfH) / halfH,
        1, 1.0);

    vec4 nearResult = inverse * near;
    vec4 farResult = inverse * far;

    nearResult /= nearResult.w;
    farResult /= farResult.w;


    vec3 result = vec3(farResult - nearResult);

    return normalize(result);

}

vec3 rayCollideTrianglePoint(vec3 rayPos, vec3 rayDir, vec3 p1, vec3 p2, vec3 p3)
{

    if (p1 == p2 || p2 == p3 || p3 == p1)
    {
        return vec3(FLT_MAX);
    }

    vec3 s1 = p2 - p1, s2 = p3 - p1;


    vec3 intersect = findIntersectionPointOfRayAndPlane(rayPos, rayDir, p1, s1, s2);

    if (intersect.x == FLT_MAX)
    {
        return vec3(FLT_MAX);
    }

    vec3 p = intersect - p1;

    float c1, c2;


    if (s1.x != 0)
    {
        if (s2.y != 0)
        {
            c2 = (p.y * s1.x - s1.y * p.x)
                / (s1.x * s2.y - s1.y * s2.x);
        }
        else if (s2.z != 0)
        {
            c2 = (p.z * s1.x - s1.z * p.x)
                / (s1.x * s2.z - s1.z * s2.x);
        }
        else
        {
            return vec3(FLT_MAX);
        }
        c1 = (p.x - s2.x * c2) / s1.x;
    }
    else if (s1.y != 0)
    {
        if (s2.x != 0)
        {
            c2 = (p.x * s1.y - s1.x * p.y)
                / (s1.y * s2.x - s1.x * s2.y);
        }
        else if (s2.z != 0)
        {
            c2 = (p.z * s1.y - s1.z * p.y)
                / (s1.y * s2.z - s1.z * s2.y);
        }
        else
        {
            return vec3(FLT_MAX);
        }
        c1 = (p.y - s2.y * c2) / s1.y;
    }
    else if (s1.z != 0)
    {
        if (s2.x != 0)
        {
            c2 = (p.x * s1.z - s1.x * p.z)
                / (s1.z * s2.x - s1.x * s2.z);
        }
        else if (s2.y != 0)
        {
            c2 = (p.y * s1.z - s1.y * p.z)
                / (s1.z * s2.y - s1.y * s2.z);
        }
        else
        {
            return vec3(FLT_MAX);
        }
        c1 = (p.z - s2.z * c2) / s1.z;
    }
    else
    {
        return vec3(FLT_MAX);
    }

    if (c1 + c2 <= 1 && c1 >= 0 && c2 >= 0)
    {
        return intersect;
    }

    return vec3(FLT_MAX);

}

bool rayCollideTriangle(vec3 rayPos, vec3 rayDir, vec3 p1, vec3 p2, vec3 p3)
{
    return rayCollideTrianglePoint(rayPos, rayDir, p1, p2, p3).x != FLT_MAX;
}

bool rayCollideRenderObject(RenderObject &o, vec3 rayPos, vec3 rayDir)
{
    if (!o.isVisible)
    {
        return false;
    }

    size_t length = o.indices.size() / 3;


    for (int i = 0; i < length; i++)
    {

        if (rayCollideTrianglePoint(rayPos - o.transform.position, rayDir, o.vertices[o.indices[3 * i]].pos,
            o.vertices[o.indices[3 * i + 1]].pos, o.vertices[o.indices[3 * i + 2]].pos).x != FLT_MAX)
        {
            return true;
        }
    }

    return false;
}

bool rayCollideModelObject(ModelObject &o, vec3 rayPos, vec3 rayDir)
{
    RenderObject *ro = o.ro;

    if (!ro->isVisible)
    {
        return false;
    }

    size_t length = ro->indices.size() / 3;

    bool anyCollisions = false;

    for (int i = 0; i < length; i++)
    {
        Vertex *v1 = &ro->vertices[ro->indices[3 * i]],
            *v2 = &ro->vertices[ro->indices[3 * i + 1]],
            *v3 = &ro->vertices[ro->indices[3 * i + 2]];
        vec3 collisionPoint = rayCollideTrianglePoint(rayPos - ro->transform.position, rayDir, v1->pos, v2->pos, v3->pos);

        if (collisionPoint.x == FLT_MAX)
        {
            continue;
        }
        anyCollisions = true;

        float distSquared = distSqr(rayPos, collisionPoint);

        if (distSquared < vg.engine.distToHoveredTriSquared)
        {
            vg.engine.distToHoveredTriSquared = distSquared;
            vg.engine.hoveredTri = TriSelection(v1,v2,v3,&o);
        }
    }

    return anyCollisions;
}

bool rayCollideLine2D(vec2 p, vec2 p1, vec2 p2)
{
    //p1 inclusive, p2 exclusive

    if (p1.x == p2.x)
    {
        if (p.x == p1.x && (p.y < p2.y || p.y <= p1.y))
        {
            return true;
        }
    }

    if (p.x == p1.x && p.y <= p1.y)
    {
        return true;
    }

    if (p.x == p2.x && p.y < p2.y)
    {
        return false;
    }

    //check if x is between x1 and x2, not inclusive - same x is handled above
    if (abs(2 * p.x - p1.x - p2.x) < abs(p2.x - p1.x))
    {
        //if x2 is less than x1, need to use the reverse operation
        //probably a way to simplify this, not bothering with it rn
        if (p2.x < p1.x)
        {
            //include on the line
            return(p.y - p1.y) * (p2.x - p1.x) >= (p2.y - p1.y) * (p.x - p1.x);
        }
        else
        {
            return(p.y - p1.y) * (p2.x - p1.x) <= (p2.y - p1.y) * (p.x - p1.x);
        }
    }



    return false;
}

bool checkIfPointInTriangle2D(vec2 p, vec2 p1, vec2 p2, vec2 p3)
{
    int count = 0;

    if (rayCollideLine2D(p, p1, p2))
    {
        count++;
    }
    if (rayCollideLine2D(p, p2, p3))
    {
        count++;
    }
    if (rayCollideLine2D(p, p3, p1))
    {
        count++;
    }

    return count % 2 == 1;
}

vec3 findIntersectionPointOfRayAndPlane(vec3 rayOri, vec3 rayDir, vec3 planeOri, vec3 v1, vec3 v2)
{
    if (rayOri == planeOri)
    {
        return planeOri;
    }
    //ray parallel to plane
    if (dot(cross(v1, v2), rayDir) == 0)
    {
        return vec3(FLT_MAX);
    }

    vec3 cv = vec3(
        v1.y * v2.z - v1.z * v2.y, 
        v1.z * v2.x - v1.x * v2.z, 
        v1.x * v2.y - v1.y * v2.x
    );

    float l = dot(cv, planeOri - rayOri) / dot(cv, rayDir);

    if (l < 0)
    {
        return vec3(FLT_MAX);
    }

    return rayOri + rayDir * l;
}

vec3 findIntersectionPointOfRayAndPlane(vec3 rayOri, vec3 rayDir, vec3 planeOri, vec3 norm)
{
    vec3 v1,v2;

    if (norm.y == 0 && norm.z == 0)//x vec
    {
        v1 = vec3(0, 1, 0);
        v2 = vec3(0, 0, 1);
    }
    else if (norm.x == 0 && norm.z == 0)//y vec
    {
        v1 = vec3(1, 0, 0);
        v2 = vec3(0, 0, 1);
    }
    else
    {
        v1 = cross(norm, vec3(1, 0, 0));
        v2 = cross(norm, vec3(0, 1, 0));
    }

    return findIntersectionPointOfRayAndPlane(rayOri, rayDir, planeOri, v1, v2);
}

vec3 movePointAlongLineFromRay(vec3 lineOrigin, vec3 lineDir, vec3 axis, vec3 rayOrigin, vec3 ray1, vec3 ray2)
{
    vec3 proj1 = projectRayOntoLineInDirection(rayOrigin, ray1, lineOrigin, lineDir, axis);
    if (proj1.x == FLT_MAX)
    {
        return vec3(0);
    }
    vec3 proj2 = projectRayOntoLineInDirection(rayOrigin, ray2, lineOrigin, lineDir, axis);
    if (proj2.x == FLT_MAX)
    {
        return vec3(0);
    }


    return proj2 - proj1;
}

vec3 projectRayOntoLineInDirection(vec3 rayOrigin, vec3 rayDir, vec3 lineOrigin, vec3 lineDir, vec3 axis)
{
    if (rayOrigin == lineOrigin)
    {
        return rayOrigin;
    }
    float c3, c4, x, y;
    vec3 dp = lineOrigin - rayOrigin;
    if (rayDir.x != 0)
    {
        x = axis.y - axis.x * rayDir.y / rayDir.x;
        y = axis.z - axis.x * rayDir.z / rayDir.x;
        c4 = rayDir.y * y - rayDir.z * x;
        c3 = (rayDir.x * (dp.z * x - dp.y * y) + dp.x * c4) 
            / (rayDir.x * (lineDir.y * y - lineDir.z * x) - lineDir.x * c4);
    }
    else if (rayDir.y != 0)
    {
        x = axis.x - axis.y * rayDir.x / rayDir.y;
        y = axis.z - axis.y * rayDir.z / rayDir.y;
        c4 = rayDir.x * y - rayDir.z * x;
        c3 = (rayDir.y * (dp.z * x - dp.x * y) + dp.y * c4)
            / (rayDir.y * (lineDir.x * y - lineDir.z * x) - lineDir.y * c4);
    }
    else if (rayDir.z != 0)
    {
        x = axis.y - axis.z * rayDir.y / rayDir.z;
        y = axis.x - axis.z * rayDir.x / rayDir.z;
        c4 = rayDir.y * y - rayDir.x * x;
        c3 = (rayDir.z * (dp.x * x - dp.y * y) + dp.z * c4)
            / (rayDir.z * (lineDir.y * y - lineDir.x * x) - lineDir.z * c4);
    }
    else
    {
        return vec3(FLT_MAX);
    }

    if (isnan(c3))
    {
        c3 = c3;
    }

    return lineOrigin + lineDir * c3;
}

float distSqr(vec3 v1, vec3 v2)
{
    float dx = v2.x - v1.x, dy = v2.y - v1.y, dz = v2.z - v1.z;
    return dx * dx + dy * dy + dz * dz;
}

float distance(vec3 v1, vec3 v2)
{
    return sqrt(distSqr(v1, v2));
}

vec3 rayCollideCircle(vec3 rayOri, vec3 rayDir, Circle *circle)
{
    vec3 collision = findIntersectionPointOfRayAndPlane(rayOri, rayDir, circle->pos, circle->normal);

    if (collision.x == FLT_MAX)
    {
        return vec3(FLT_MAX);
    }

    float dist = distance(circle->pos, collision);

    if (dist <= circle->radius)
    {
        return collision;
    }

    return vec3(FLT_MAX);
}



