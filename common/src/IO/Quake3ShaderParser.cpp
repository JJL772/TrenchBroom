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

#include "Quake3ShaderParser.h"

#include "Assets/Quake3Shader.h"

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderTokenizer::Quake3ShaderTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "", '\\') {}

        Quake3ShaderTokenizer::Quake3ShaderTokenizer(const String &str) :
        Tokenizer(str, "", '\\') {}

        Tokenizer<unsigned int>::Token Quake3ShaderTokenizer::emitToken() {
            while (!eof()) {
                const auto startLine = line();
                const auto startColumn = column();
                const auto* c = curPos();
                switch (*c) {
                    case '{':
                        advance();
                        return Token(Quake3ShaderToken::OBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(Quake3ShaderToken::CBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '\r':
                        if (lookAhead() == '\n') {
                            advance();
                        }
                        // handle carriage return without consecutive linefeed
                        // by falling through into the line feed case
                        switchFallthrough();
                    case '\n':
                        discardWhile(Whitespace()); // handle empty lines and such
                        return Token(Quake3ShaderToken::Eol, c, c + 1, offset(c), startLine, startColumn);
                    case ' ':
                    case '\t':
                        advance();
                        break;
                    case '$': {
                        const auto* e = readUntil(Whitespace());
                        if (e == nullptr) {
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        }
                        return Token(Quake3ShaderToken::Variable, c, e, offset(c), startLine, startColumn);
                    }
                    case '/':
                        if (lookAhead() == '/') {
                            // parse single line comment starting with //
                            advance(2);
                            discardUntil("\n\r");
                            // do not discard the terminating line break since it might be semantically relevant
                            // e.g. for terminating a block entry
                            break;
                        } else if (lookAhead() == '*') {
                            // parse multiline comment delimited by /* and */
                            advance(2);
                            while (curChar() != '*' || lookAhead() != '/') {
                                errorIfEof();
                                advance();
                            }
                            advance(2);
                            break;
                        }
                        // fall through into the default case to parse a string that starts with '/'
                        switchFallthrough();
                    default:
                        auto* e = readDecimal(Whitespace());
                        if (e != nullptr) {
                            return Token(Quake3ShaderToken::Number, c, e, offset(c), startLine, startColumn);
                        }

                        e = readUntil(Whitespace());
                        if (e == nullptr) {
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        }
                        return Token(Quake3ShaderToken::String, c, e, offset(c), startLine, startColumn);
                }
            }
            return Token(Quake3ShaderToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        Quake3ShaderParser::Quake3ShaderParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}

        Quake3ShaderParser::Quake3ShaderParser(const String& str) :
        m_tokenizer(str) {}

        std::vector<Assets::Quake3Shader> Quake3ShaderParser::parse() {
            std::vector<Assets::Quake3Shader> result;
            while (!m_tokenizer.peekToken(Quake3ShaderToken::Eol).hasType(Quake3ShaderToken::Eof)) {
                Assets::Quake3Shader shader;
                parseTexture(shader);
                parseBody(shader);
                result.push_back(shader);
            }
            return result;
        }

        void Quake3ShaderParser::parseBody(Assets::Quake3Shader& shader) {
            expect(Quake3ShaderToken::OBrace, m_tokenizer.nextToken(Quake3ShaderToken::Eol));
            auto token = m_tokenizer.peekToken(Quake3ShaderToken::Eol);
            expect(Quake3ShaderToken::CBrace | Quake3ShaderToken::OBrace | Quake3ShaderToken::String, token);

            while (!token.hasType(Quake3ShaderToken::CBrace)) {
                if (token.hasType(Quake3ShaderToken::OBrace)) {
                    parseStage(shader);
                } else {
                    parseBodyEntry(shader);
                }
                token = m_tokenizer.peekToken(Quake3ShaderToken::Eol);
            }
            expect(Quake3ShaderToken::CBrace, m_tokenizer.nextToken(Quake3ShaderToken::Eol));
        }

        void Quake3ShaderParser::parseStage(Assets::Quake3Shader& shader) {
            expect(Quake3ShaderToken::OBrace, m_tokenizer.nextToken(Quake3ShaderToken::Eol));
            auto token = m_tokenizer.peekToken(Quake3ShaderToken::Eol);
            expect(Quake3ShaderToken::CBrace | Quake3ShaderToken::OBrace | Quake3ShaderToken::String, token);

            auto& stage = shader.addStage();
            while (!token.hasType(Quake3ShaderToken::CBrace)) {
                parseStageEntry(stage);
                token = m_tokenizer.peekToken(Quake3ShaderToken::Eol);
            }
            expect(Quake3ShaderToken::CBrace, m_tokenizer.nextToken(Quake3ShaderToken::Eol));
        }

        void Quake3ShaderParser::parseTexture(Assets::Quake3Shader& shader) {
            const auto token = expect(Quake3ShaderToken::String, m_tokenizer.nextToken(Quake3ShaderToken::Eol));
            shader.shaderPath = Path(token.data());
        }

        void Quake3ShaderParser::parseBodyEntry(Assets::Quake3Shader& shader) {
            auto token = m_tokenizer.nextToken(Quake3ShaderToken::Eol);
            expect(Quake3ShaderToken::String, token);
            const auto key = token.data();
            if (key == "qer_editorimage") {
                token = expect(Quake3ShaderToken::String, m_tokenizer.nextToken());
                shader.editorImage = Path(token.data());
            } else if (key == "q3map_lightimage") {
                token = expect(Quake3ShaderToken::String, m_tokenizer.nextToken());
                shader.lightImage = Path(token.data());
            } else if (key == "surfaceparm") {
                token = expect(Quake3ShaderToken::String, m_tokenizer.nextToken());
                shader.surfaceParms.insert(token.data());
            } else if (key == "cull") {
                token = expect(Quake3ShaderToken::String, m_tokenizer.nextToken());
                const auto value = token.data();
                if (value == "front") {
                    shader.culling = Assets::Quake3Shader::Culling::Front;
                } else if (value == "back") {
                    shader.culling = Assets::Quake3Shader::Culling::Back;
                } else if (value == "none" || value == "disable") {
                    shader.culling = Assets::Quake3Shader::Culling::None;
                }
            } else {
                while (!m_tokenizer.nextToken().hasType(Quake3ShaderToken::Eol));
            }
        }

        void Quake3ShaderParser::parseStageEntry(Assets::Quake3ShaderStage& stage) {
            auto token = m_tokenizer.nextToken(Quake3ShaderToken::Eol);
            expect(Quake3ShaderToken::String, token);
            const auto key = token.data();
            if (key == "map") {
                token = expect(Quake3ShaderToken::String | Quake3ShaderToken::Variable, m_tokenizer.nextToken());
                stage.map = Path(token.data());
            } else if (key == "blendFunc") {
                token = expect(Quake3ShaderToken::String, m_tokenizer.nextToken());
                const auto param1 = token.data();
                if (m_tokenizer.peekToken().hasType(Quake3ShaderToken::String)) {
                    token = m_tokenizer.nextToken();
                    auto param2 = token.data();
                    stage.blendFunc.srcFactor = param1;
                    stage.blendFunc.destFactor = param2;
                } else {
                    if (param1 == "add") {
                        stage.blendFunc.srcFactor = Assets::Quake3ShaderStage::BlendFunc::One;
                        stage.blendFunc.destFactor = Assets::Quake3ShaderStage::BlendFunc::One;
                    } else if (param1 == "filter") {
                        stage.blendFunc.srcFactor = Assets::Quake3ShaderStage::BlendFunc::DestColor;
                        stage.blendFunc.destFactor = Assets::Quake3ShaderStage::BlendFunc::Zero;
                    } else if (param1 == "blend") {
                        stage.blendFunc.srcFactor = Assets::Quake3ShaderStage::BlendFunc::SrcAlpha;
                        stage.blendFunc.destFactor = Assets::Quake3ShaderStage::BlendFunc::OneMinusSrcAlpha;
                    }
                }
            } else {
                while (!m_tokenizer.nextToken().hasType(Quake3ShaderToken::Eol));
            }
        }

        Quake3ShaderParser::TokenNameMap Quake3ShaderParser::tokenNames() const {
            TokenNameMap result;
            result[Quake3ShaderToken::Number]   = "number";
            result[Quake3ShaderToken::String]   = "string";
            result[Quake3ShaderToken::Variable] = "variable";
            result[Quake3ShaderToken::OBrace]   = "'{'";
            result[Quake3ShaderToken::CBrace]   = "'}'";
            result[Quake3ShaderToken::Comment]  = "comment";
            result[Quake3ShaderToken::Eol]      = "end of line";
            result[Quake3ShaderToken::Eof]      = "end of file";
            return result;
        }
    }
}