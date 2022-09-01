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

#pragma once

#include "FloatType.h"
#include "IO/NodeSerializer.h"

#include <vecmath/forward.h>

#include <array>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <ostream>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushNode;
        class BrushFace;
        class EntityProperty;
        class Node;
    }

    namespace IO {
        class VmfSerializer : public NodeSerializer {
        public:
            explicit VmfSerializer(std::ostream& vmfStream);
        private:
			// NodeSerializer overrides
		
            void doBeginFile(const std::vector<const Model::Node*>& rootNodes) override;
            void doEndFile() override;
            void doBeginEntity(const Model::Node* node) override;
            void doEndEntity(const Model::Node* node) override;
            void doEntityProperty(const Model::EntityProperty& property) override;
            void doBrush(const Model::BrushNode* brush) override;
            void doPatch(const Model::PatchNode* patchNode) override;
            void doBrushFace(const TrenchBroom::Model::BrushFace&) override;


            void writeBrushFace(const Model::BrushFace& face);
        protected:
            std::ostream& m_stream;
            int m_id = 0;
        };
    }
}

