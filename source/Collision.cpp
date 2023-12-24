#include "Collision.hpp"
#include "Math.hpp"

#include <cmath>

namespace Collision
{
    static bool isIntersect(const Vec3D &normal, const Triangle &tri, const Line &line)
    {
        Vec3D v1 = line.start - tri.vtx[0];
        Vec3D v2 = line.end - tri.vtx[0];
        float d1 = Math::dot(v1, normal);
        float d2 = Math::dot(v2, normal);
        if (d1 * d2 <= 0) {//←平面と交差している
            //平面と線分が交差している点の座標ip
            d1 = std::abs(d1);
            d2 = std::abs(d2);
            float m = d1 / (d1 + d2);
            Vec3D ip = line.start * (1 - m) + line.end * m;
            //ipが三角形に含まれているか
            int containFlag = 1;//とりあえず含まれていることにする
            for (int i = 0; i < 3; i++) {
                Vec3D side = tri.vtx[(i + 1) % 3] - tri.vtx[i];//三角形の一辺のベクトル
                Vec3D vip = ip - tri.vtx[i];//三角形の１つの頂点から交差点までのベクトル
                Vec3D c = Math::cross(side, vip);

                float d = Math::dot(normal, c);
                if (d < 0) {
                    containFlag = 0;//含まれていない
                }
            }
            if (containFlag == 1) {
                return true;
            }
        }

        return false;
    }

    bool checkCollision(const Object &obj, const Position &oldPos, const Position &newPos)
    {
        Line moveVec = {oldPos, newPos};
        const Vertex *vertices = obj.getVertices();

        for(u32 i = 0; i < obj.getNumVertices(); i += 3)
        {
            const Vec3D *normal = (Vec3D *)(&vertices[i].normal[0]);
            const auto &vtx1 = vertices[i].position;
            const auto &vtx2 = vertices[i + 1].position;
            const auto &vtx3 = vertices[i + 2].position;

            if(isIntersect(*normal, Triangle{{vtx1, vtx2, vtx3}}, moveVec))
            {
                return false;
            }
        }

        return true;
    }
} // namespace Collision
