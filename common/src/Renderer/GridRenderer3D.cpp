/*
 Copyright (C) 2021 Jeremy Lorelli

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GridRenderer3D.h"

#include "FloatType.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/PrimType.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"

#include <vecmath/bbox.h>
#include <vecmath/intersection.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace Renderer {

        GridRenderer3D::GridRenderer3D(const PerspectiveCamera &camera, const vm::bbox3 &worldBounds) :
            m_vertexArray(VertexArray::move(vertices(worldBounds)))
        {
        }

        void GridRenderer3D::doPrepareVertices(VboManager &vboManager) {
            m_vertexArray.prepare(vboManager);
        }

        void GridRenderer3D::doRender(RenderContext &renderContext) {
            if(renderContext.showGrid()) {
                ActiveShader shader(renderContext.shaderManager(), Shaders::Grid3DShader);
                const auto &camera = renderContext.camera();
                const auto zdist = vm::abs(camera.position().z());

                shader.set("Normal", vm::vec3f(0, 0, 1));
                shader.set("RenderGrid", renderContext.showGrid());
                shader.set("GridSize", static_cast<float>(renderContext.gridSize()));
                shader.set("GridAlpha", pref(Preferences::GridAlpha));
                shader.set("GridColor", pref(Preferences::GridColor2D));
                shader.set("CameraZDist", zdist);

                m_vertexArray.render(PrimType::Quads);
            }
        }

        std::vector<GridRenderer3D::Vertex> GridRenderer3D::vertices(const vm::bbox3 &worldBounds) {
            // Generate verticies along the XY plane
            const auto lower2d = worldBounds.min.xy();
            const auto upper2d = worldBounds.max.xy();
            return {
                    Vertex(vm::vec3f(lower2d, 0.f)),
                    Vertex(vm::vec3f(lower2d.x(), upper2d.y(), 0.f)),
                    Vertex(vm::vec3f(upper2d, 0.f)),
                    Vertex(vm::vec3f(upper2d.x(), lower2d.y(), 0.f)),
            };
        }
    }
}