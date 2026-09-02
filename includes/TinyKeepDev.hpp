/**
 * @file TeenyKeepDev.hpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-15
 *
 * @addtogroup procedurals
 * @{
 */

#include "Deterministic.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <stdio.h>
#include "Type.hpp"
#include "graph.hpp"

class TinyKeepDev {
    private:
        static bool circumcircleContains(const Trianglef& tri, const Vector2f& p) {
            //p become the origin (0,0)
            Vector2f a = {tri.p1.x - p.x, tri.p1.y - p.y};
            Vector2f b = {tri.p2.x - p.x, tri.p2.y - p.y};
            Vector2f c = {tri.p3.x - p.x, tri.p3.y - p.y};

            //geometric condition (squared distances from P to each vertex)
            double a2 = a.square_magnitude(); // ||A'||^2
            double b2 = b.square_magnitude(); // ||B'||^2
            double c2 = c.square_magnitude(); // ||C'||^2

            double term1 = a.x * (b.y * c2 - c.y * b2); // contribution de la coordonnée x de A
            double term2 = - a.y * (b.x * c2 - c.x * b2); // contribution de la coordonnée y de A
            double term3 = a2 * b.cross(c); // contribution de la distance au carré de A (z) combinée à B et C // B' × C'
            double det = term1 + term2 + term3; // La somme (det) nous dit si P est au-dessus ou en dessous du plan → donc à l’intérieur ou à l’extérieur du cercle.

            return det < epsilonf;
        }
 
        static bool isCollidingOrClose(const Rectf &a, const Rectf &b, float margin = 0.0f) {
            return !(a.x + a.w/2 + margin < b.x - b.w/2 ||
                    a.x - a.w/2 - margin > b.x + b.w/2 ||
                    a.y + a.h/2 + margin < b.y - b.h/2 ||
                    a.y - a.h/2 - margin > b.y + b.h/2);
        }
        //mst
        struct EqV2 {
            bool operator()(const Vector2f& a, const Vector2f& b) const {
                return a.same(b);
            }
        };
        struct DistV2 {
            double operator()(const Vector2f& a, const Vector2f& b) const {
                double dx = a.x - b.x, dy = a.y - b.y;
                return std::sqrt(dx * dx + dy * dy);
            }
        };


    public:

        /**
         * @brief 
         * 
         * @tparam nbRects 
         * @param radius 
         * @param maxRectRatio \in ]0,1]
         * @param radiusRatios \in ]0,2]
         * @param seed 
         * @return Rects
         */
        template<int nbRects>
        static std::vector<Rectf> generateRects(float radius, float maxRectRatio = 0.5, std::pair<float, float> radiusRatios = {0.1f, 1.f} , uint64_t seed = 0) {
            std::vector<Rectf> result;

            // 1. Ratio largeur/hauteur
            float denominator = 1 / maxRectRatio; //maxRectRatio = 0.5, inv.. = 2; 

            // 2. Taille (diagonale)
            float diagMin = radius * radiusRatios.first;
            float diagMax = radius * radiusRatios.second;

            uint64_t next = seed;

            for (int i = 0; i < nbRects; ++i) {

                // 1. Ratio largeur/hauteur
                float ratio = maxRectRatio + random0D(next) * (denominator - maxRectRatio);
                next = mix64(next);
                // 2. Taille (diagonale)
                float diagonal = diagMin + random0D(next) * (diagMax - diagMin);
                next = mix64(next);

                // 3. Largeur / Hauteur
                float w = diagonal * ratio / std::sqrt(1.0f + ratio * ratio);
                float h = diagonal / std::sqrt(1.0f + ratio * ratio);

                // 4. Rayon réduit pour que le rectangle reste dans le cercle
                float reducedRadius = radius - diagonal / 2.0f;

                // 5. Position aléatoire uniforme dans le disque
                float theta = random0D(next) * 2.0f * M_PI;
                next = mix64(next);
                float r = reducedRadius * std::sqrt(random0D(next));
                next = mix64(next);

                float x = r * std::cos(theta);
                float y = r * std::sin(theta);

                // Convertir en coin supérieur gauche
                //x -= w * 0.5f;
                //y -= h * 0.5f;

                // 6. Enregistrement du rectangle
                result.push_back({x, y, w, h});

                //next seed
            }
            return result;
        }

        template<int its = 50>
        static void relaxRects(std::vector<Rectf>& rects) {

            for (int i = 0; i < its; ++i) {
                for (size_t i = 0; i < rects.size(); ++i) {
                    for (size_t j = i + 1; j < rects.size(); ++j) {

                        if (isCollidingOrClose(rects[i], rects[j])) {
                            float dx = rects[j].x - rects[i].x;
                            float dy = rects[j].y - rects[i].y;
                            float dist = sqrtf(dx*dx + dy*dy) + 0.001f; // éviter division par 0
                            float nx = dx / dist;
                            float ny = dy / dist;

                            float overlapX = (rects[i].w + rects[j].w)/2 - fabs(dx);
                            float overlapY = (rects[i].h + rects[j].h)/2 - fabs(dy);
                            float overlap = fminf(overlapX, overlapY);

                            float attractionRange = 50.0f; // distance max pour attraction
                            if (overlap > 0) {
                                // Repulsion
                                rects[i].x -= nx * overlap * 0.5f;
                                rects[i].y -= ny * overlap * 0.5f;
                                rects[j].x += nx * overlap * 0.5f;
                                rects[j].y += ny * overlap * 0.5f;
                            } else if (dist < attractionRange) {
                                // Attraction proportionnelle à la distance
                                float attractionStrength = (attractionRange - dist) * 0.05f; // ajustable
                                rects[i].x += nx * attractionStrength;
                                rects[i].y += ny * attractionStrength;
                                rects[j].x -= nx * attractionStrength;
                                rects[j].y -= ny * attractionStrength;
                            }
                        }
                    }
                }
            }
        }

        // --------------------------------------------------
        // Algorithme de Bowyer–Watson
        // --------------------------------------------------
        static std::vector<Linef> bowyerWatson(const std::vector<Rectf>& rects) {
            //1. Points
            std::vector<Vector2f> points(rects.size());
            points.reserve(rects.size());

            //2. Super Triangle
            double minX = rects[0].x, maxX = rects[0].x;
            double minY = rects[0].y, maxY = rects[0].y;
            for (auto& r : rects) {
                minX = std::min(minX, r.x);
                maxX = std::max(maxX, r.x);
                minY = std::min(minY, r.y);
                maxY = std::max(maxY, r.y);
            }

            double dx = maxX - minX;
            double dy = maxY - minY;
            double deltaMax = std::max(dx, dy) * 10.0f;

            Vector2f p1 = {minX - deltaMax, minY - deltaMax};
            Vector2f p2 = {minX + 0.5f * dx, maxY + deltaMax};
            Vector2f p3 = {maxX + deltaMax, minY - deltaMax};

            std::vector<Trianglef> triangles;
            triangles.push_back({p1, p2, p3});

            //3. Insert points 1 by 1:
                
            for (auto &p: rects) {
                std::vector<Trianglef> badTriangles; //.a
                std::vector<Linef> polygonEdges; //.b

                points.push_back({p.x, p.y});

                //  a. Identifier les mauvais triangles
                //    •	Pour chaque triangle t dans triangles :
                //          •   Si p est à l’intérieur du cercle circonscrit de t → ajoute t à badTriangles.
                for (auto t: triangles) {
                    if (circumcircleContains(t, {p.x, p.y})) {
                        badTriangles.push_back(t);
                    }
                }

                //  b. Construire le polygone de la cavité
                //    •   Pour chaque triangle bt dans badTriangles :
                //        •   Pour chaque arête e de bt :
                //            •   Si aucun autre triangle dans badTriangles ne partage e → ajoute e à polygonEdges.
                //    •	Résultat : polygonEdges = contour extérieur des triangles à supprimer.
                for (auto bt: badTriangles) {
                    std::vector<Linef> btedges = {
                        { bt.p1, bt.p2 },
                        { bt.p2, bt.p3 },
                        { bt.p3, bt.p1 }
                    };

                    for (const Linef& e : btedges) {
                        bool shared = false;

                        // Check if this edge is shared with another bad triangle
                        for (const Trianglef other : badTriangles) {
                            if (bt.same(other)) continue; // skip self
                            std::vector<Linef> otherEdges = {
                                { other.p1, other.p2 },
                                { other.p2, other.p3 },
                                { other.p3, other.p1 }
                            };

                            for (const Linef& oe : otherEdges) {
                                if (e.same(oe)) {
                                    shared = true;
                                    break;
                                }
                            }
                            if (shared) break;
                        }

                        // If no other triangle shares this edge, it belongs to the polygon boundary
                        if (!shared) {
                            polygonEdges.push_back(e);
                        }
                    }
                }

                //  c. Suprime les mauvais triangles
                //    • Retire tous les triangles de badTriangles de la liste triangles.
                for (const Trianglef& bad : badTriangles) {
                    auto it = std::remove_if(triangles.begin(), triangles.end(),
                        [&](const Trianglef& t) {
                            return (t.same(bad));
                        });
                    triangles.erase(it, triangles.end());
                }

                //  d. – Ajouter de nouveaux triangles
                //    •   Pour chaque arête e dans polygonEdges :
                //      •   Crée un triangle avec p et e → newTriangle = (e.a, e.b, p)
                //      •   Ajoute newTriangle à triangles.
                for (auto& e : polygonEdges) {
                        triangles.push_back({e.p1, e.p2, {p.x, p.y}});
                }
            }

            //4. Nettoyage final
            //  a.	Supprime tous les triangles qui contiennent un sommet du super-triangle → ces triangles sont artificiels.
            triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
                    [&](const Trianglef& t) {
                        auto usesSuperVertex = [&](const Vector2f& v) {
                            return (fabs(v.x - p1.x) < 1e-12 && fabs(v.y - p1.y) < 1e-12) ||
                                (fabs(v.x - p2.x) < 1e-12 && fabs(v.y - p2.y) < 1e-12) ||
                                (fabs(v.x - p3.x) < 1e-12 && fabs(v.y - p3.y) < 1e-12);
                        };
                        return usesSuperVertex(t.p1) || usesSuperVertex(t.p2) || usesSuperVertex(t.p3);
                    }),
                triangles.end()
            );

            //  b.	Les triangles restants sont ta triangulation Delaunay.

            //  c.	Extrais les arêtes finales pour relier les rectangles :
            //    •   Pour chaque triangle, ajoute (p1,p2), (p2,p3), (p3,p1) à ta liste edges.
            std::vector<Linef> edges;
            for (auto& t : triangles) {
                edges.push_back({t.p1, t.p2});
                edges.push_back({t.p2, t.p3});
                edges.push_back({t.p3, t.p1});
            }

            return edges;
        }

    // mst
        static std::vector<Linef> mst(const std::vector<Linef>& edges) {
            std::vector<std::pair<Vector2f, Vector2f>> pairs;
            pairs.reserve(edges.size());
            for (const auto& e : edges) pairs.emplace_back(e.p1, e.p2);

            auto g   = Graph<Vector2f, EqV2>::fromPairs<DistV2>(pairs);
            auto mst = g.mst();

            std::vector<Linef> result;
            result.reserve(mst.edgeCount());
            for (const auto& link : mst.edges())
                result.push_back({mst.node(link.from), mst.node(link.to)});
            return result;
        }

    static std::vector<Linef> shortenLine(const std::vector<Rectf>& rects,
                                            const std::vector<Linef>& edges) {
        auto findRect = [&](const Vector2f& p) -> const Rectf* {
            for (const auto& r : rects) {
                Vector2f c{r.x, r.y};
                if (c.same(p)) return &r;
            }
            return nullptr;
        };

        auto exitPoint = [](const Rectf& r, const Vector2f& other) -> Vector2f {
            double dx = other.x - r.x;
            double dy = other.y - r.y;
            constexpr double INF = std::numeric_limits<double>::infinity();
            double tx = (std::abs(dx) > epsilond) ? (r.w * 0.5) / std::abs(dx) : INF;
            double ty = (std::abs(dy) > epsilond) ? (r.h * 0.5) / std::abs(dy) : INF;
            double t  = std::min(tx, ty);
            return {r.x + t * dx, r.y + t * dy};
        };

        std::vector<Linef> result;
        result.reserve(edges.size());

        for (const auto& e : edges) {
            const Rectf* ra = findRect(e.p1);
            const Rectf* rb = findRect(e.p2);
            if (!ra || !rb) { result.push_back(e); continue; }   // can't match → keep as-is

            Vector2f a = exitPoint(*ra, e.p2);   // exit ra toward p2
            Vector2f b = exitPoint(*rb, e.p1);   // exit rb toward p1
            result.push_back({a, b});
        }
        return result;
    }
};
/** @} */

