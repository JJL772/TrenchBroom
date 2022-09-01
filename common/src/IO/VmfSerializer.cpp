/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "VmfSerializer.h"

#include "Ensure.h"
#include "Assets/Texture.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/PatchNode.h"
#include "Model/Polyhedron.h"
#include "Model/LayerNode.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/parallel.h>

#include <fmt/format.h>

#include <iostream>
#include <fstream>

namespace TrenchBroom {
	namespace IO {
		
		VmfSerializer::VmfSerializer(std::ostream& vmfStream) :
			m_stream(vmfStream) {
			
		}
		
        void VmfSerializer::doBeginFile(const std::vector<const Model::Node*>& rootNodes) {
#if 0
            ensure(m_nodeToPrecomputedString.empty(), "MapFileSerializer may not be reused");
            // collect nodes
            std::vector<std::variant<const Model::BrushNode*, const Model::PatchNode*>> nodesToSerialize;
            nodesToSerialize.reserve(rootNodes.size());

            Model::Node::visitAll(rootNodes, kdl::overload(
                [](auto&& thisLambda, const Model::WorldNode* world) { world->visitChildren(thisLambda); },
                [](auto&& thisLambda, const Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [](auto&& thisLambda, const Model::GroupNode* group) { group->visitChildren(thisLambda); },
                [](auto&& thisLambda, const Model::EntityNode* entity) { entity->visitChildren(thisLambda); },
                [&](const Model::BrushNode* brush) {
                    nodesToSerialize.push_back(brush);
                },
                [&](const Model::PatchNode* patchNode) {
                    nodesToSerialize.push_back(patchNode);
                }
            ));

            // serialize brushes to strings in parallel
            using Entry = std::pair<const Model::Node*, PrecomputedString>;
            std::vector<Entry> result = kdl::vec_parallel_transform(std::move(nodesToSerialize),
                [&](const auto& node) {
                    return std::visit(kdl::overload(
                        [&](const Model::BrushNode* brushNode) {
                            return Entry{brushNode, writeBrushFaces(brushNode->brush())};
                        },
                        [&](const Model::PatchNode* patchNode) {
                            return Entry{patchNode, writePatch(patchNode->patch())};
                        }
                    ), node);
                });
#endif
		}
        
		void VmfSerializer::doEndFile() {
		}

        void VmfSerializer::doBeginEntity(const Model::Node* node) {
			
		}
        
		void VmfSerializer::doEndEntity(const Model::Node* node) {
			
		}
        
		void VmfSerializer::doEntityProperty(const Model::EntityProperty& property) {
			
		}

        void VmfSerializer::doBrush(const Model::BrushNode* brush) {
			m_stream << "solid\n\t{\n\t\t" << "\"id\" \"" << ++m_id << "\"\n";
			
			// Write out all faces
			for (auto& face : brush->brush().faces()) {
				writeBrushFace(face);
			}
			
			m_stream << "\teditor\n\t{\n";
			m_stream << "\t\t\"color\" \"255 255 255\"\n";
			m_stream << "\t\t\"visgroupshown\" \"1\"\n";
			m_stream << "\t\t\"visgroupautoshown\" \"1\"\n\t}\n}";
		}
        
		void VmfSerializer::writeBrushFace(const Model::BrushFace& face) {
			m_stream << "side\n{\n";
			
			// ID
			m_stream << "\t\t\t\"id\" \"" << ++m_id << "\"\n";
			
			// Plane
			auto &points = face.points();
            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "\t\t\t\"plane\" \"( {} {} {} ) ( {} {} {} ) ( {} {} {} )\"",
                           points[0].x(),
                           points[0].y(),
                           points[0].z(),
                           points[1].x(),
                           points[1].y(),
                           points[1].z(),
                           points[2].x(),
                           points[2].y(),
                           points[2].z());
						   
			// Material
			m_stream << "\t\t\t\"material\" \"" << face.texture()->name() << "\"\n";
			
			// uAxis transforms
            const vm::vec3 uAxis = face.textureXAxis();
			fmt::format_to(std::ostream_iterator<char>(m_stream), "\t\t\t\"uaxis\" \"[ {} {} {} {} ] {}\n",
				uAxis.x(), uAxis.y(), uAxis.z(), face.attributes().xOffset(), face.attributes().xScale());
			
			// vAxis transforms 
            const vm::vec3 vAxis = face.textureYAxis();
			fmt::format_to(std::ostream_iterator<char>(m_stream), "\t\t\t\"vaxis\" \"[ {} {} {} {} ] {}\n",
				vAxis.x(), vAxis.y(), vAxis.z(), face.attributes().yOffset(), face.attributes().yScale());
				
			// Texture rotation 
			fmt::format_to(std::ostream_iterator<char>(m_stream), "\t\t\t\"rotation\" \"{}\"\n",
				face.attributes().rotation());
			
			m_stream << "\t\t\t\"lightmapscale\" \"16\"\n"; // @TODO: Properly set this
			m_stream << "\t\t\t\"smoothing_groups\" \"0\"\n\t\t}"; // @TODO: Properly set this
		}

        void VmfSerializer::doPatch(const Model::PatchNode* patchNode) {
			// @TODO: UNSUPPORTED FOR NOW
		}
		
		void VmfSerializer::doBrushFace(const TrenchBroom::Model::BrushFace&) {
			// Intentionally empty
		}
	}
}