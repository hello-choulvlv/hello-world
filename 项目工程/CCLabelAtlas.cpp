/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "2d/CCLabelAtlas.h"
#include "renderer/CCTextureAtlas.h"
#include "renderer/CCRenderer.h"
#include "platform/CCFileUtils.h"
#include "base/CCDirector.h"
#include "base/ccUTF8.h"
#include "renderer/CCTextureCache.h"
#include "2d/CCSpriteFrameCache.h"

#if CC_LABELATLAS_DEBUG_DRAW
#include "renderer/CCRenderer.h"
#endif

NS_CC_BEGIN

//CCLabelAtlas - Creation & Init

LabelAtlas* LabelAtlas::create()
{
    LabelAtlas* ret = new (std::nothrow) LabelAtlas();
    if (ret)
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_RELEASE_NULL(ret);
    }
    
    return ret;
}

LabelAtlas* LabelAtlas::create(const std::string& string, const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap)
{
    LabelAtlas* ret = new (std::nothrow) LabelAtlas();
    if(ret && ret->initWithString(string, charMapFile, itemWidth, itemHeight, startCharMap))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LabelAtlas::initWithString(const std::string& string, const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap)
{
    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(charMapFile);
    return initWithString(string, texture, itemWidth, itemHeight, startCharMap);
}

bool LabelAtlas::initWithString(const std::string& string, Texture2D* texture, int itemWidth, int itemHeight, int startCharMap)
{
    if (AtlasNode::initWithTexture(texture, itemWidth, itemHeight, static_cast<int>(string.size())))
    {
        _mapStartChar = startCharMap;
        this->setString(string);
        return true;
    }
    return false;
}

LabelAtlas* LabelAtlas::create(const std::string& string, const std::string& fntFile)
{    
    LabelAtlas *ret = new (std::nothrow) LabelAtlas();
    if (ret)
    {
        if (ret->initWithString(string, fntFile))
        {
            ret->autorelease();
        }
        else 
        {
            CC_SAFE_RELEASE_NULL(ret);
        }
    }
    
    return ret;
}

bool LabelAtlas::initWithString(const std::string& theString, const std::string& fntFile)
{
    std::string pathStr = FileUtils::getInstance()->fullPathForFilename(fntFile);
    std::string relPathStr = pathStr.substr(0, pathStr.find_last_of("/"))+"/";
    
    ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(pathStr);

    CCASSERT(dict["version"].asInt() == 1, "Unsupported version. Upgrade cocos2d version");

    std::string textureFilename = relPathStr + dict["textureFilename"].asString();

    unsigned int width = dict["itemWidth"].asInt() / CC_CONTENT_SCALE_FACTOR();
    unsigned int height = dict["itemHeight"].asInt() / CC_CONTENT_SCALE_FACTOR();
    unsigned int startChar = dict["firstChar"].asInt();


    this->initWithString(theString, textureFilename, width, height, startChar);

    return true;
}

//CCLabelAtlas - Atlas generation
void LabelAtlas::updateAtlasValues()
{
    if(_itemsPerRow == 0)
    {
        return;
    }

    ssize_t n = _string.length();

    const unsigned char *s = (unsigned char*)_string.c_str();

    Texture2D *texture = _textureAtlas->getTexture();
    float textureWide = (float) texture->getPixelsWide();
    float textureHigh = (float) texture->getPixelsHigh();
    float itemWidthInPixels = _itemWidth * CC_CONTENT_SCALE_FACTOR();
    float itemHeightInPixels = _itemHeight * CC_CONTENT_SCALE_FACTOR();
    if (_ignoreContentScaleFactor)
    {
        itemWidthInPixels = _itemWidth;
        itemHeightInPixels = _itemHeight;
    }

    CCASSERT(n <= _textureAtlas->getCapacity(), "updateAtlasValues: Invalid String length");
    V3F_C4B_T2F_Quad* quads = _textureAtlas->getQuads();
    for(ssize_t i = 0; i < n; i++) {

        unsigned char a = s[i] - _mapStartChar;
        float row = (float) (a % _itemsPerRow);
        float col = (float) (a / _itemsPerRow);

#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
        // Issue #938. Don't use texStepX & texStepY
        float left        = (2 * row * itemWidthInPixels + 1) / (2 * textureWide);
        float right        = left + (itemWidthInPixels * 2 - 2) / (2 * textureWide);
        float top        = (2 * col * itemHeightInPixels + 1) / (2 * textureHigh);
        float bottom    = top + (itemHeightInPixels * 2 - 2) / (2 * textureHigh);
#else
        float left        = row * itemWidthInPixels / textureWide;
        float right        = left + itemWidthInPixels / textureWide;
        float top        = col * itemHeightInPixels / textureHigh;
        float bottom    = top + itemHeightInPixels / textureHigh;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL

        quads[i].tl.texCoords.u = left;
        quads[i].tl.texCoords.v = top;
        quads[i].tr.texCoords.u = right;
        quads[i].tr.texCoords.v = top;
        quads[i].bl.texCoords.u = left;
        quads[i].bl.texCoords.v = bottom;
        quads[i].br.texCoords.u = right;
        quads[i].br.texCoords.v = bottom;

        quads[i].bl.vertices.x = (float) (i * _itemWidth);
        quads[i].bl.vertices.y = 0;
        quads[i].bl.vertices.z = 0.0f;
        quads[i].br.vertices.x = (float)(i * _itemWidth + _itemWidth);
        quads[i].br.vertices.y = 0;
        quads[i].br.vertices.z = 0.0f;
        quads[i].tl.vertices.x = (float)(i * _itemWidth);
        quads[i].tl.vertices.y = (float)(_itemHeight);
        quads[i].tl.vertices.z = 0.0f;
        quads[i].tr.vertices.x = (float)(i * _itemWidth + _itemWidth);
        quads[i].tr.vertices.y = (float)(_itemHeight);
        quads[i].tr.vertices.z = 0.0f;
        Color4B c(_displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity);
        quads[i].tl.colors = c;
        quads[i].tr.colors = c;
        quads[i].bl.colors = c;
        quads[i].br.colors = c;
    }
    if (n > 0 ){
        _textureAtlas->setDirty(true);
        ssize_t totalQuads = _textureAtlas->getTotalQuads();
        if (n > totalQuads) {
            _textureAtlas->increaseTotalQuadsWith(static_cast<int>(n - totalQuads));
        }
    }
}

//CCLabelAtlas - LabelProtocol
void LabelAtlas::setString(const std::string &label)
{
    ssize_t len = label.size();
    if (len > _textureAtlas->getTotalQuads())
    {
        _textureAtlas->resizeCapacity(len);
    }
    _string.clear();
    _string = label;
    this->updateAtlasValues();

    Size s = Size(len * _itemWidth, _itemHeight);

    this->setContentSize(s);

    _quadsToDraw = len;
}

const std::string& LabelAtlas::getString(void) const
{
    return _string;
}

void LabelAtlas::updateColor()
{
    if (_textureAtlas)
    {
        Color4B color4( _displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity );
        if (_isOpacityModifyRGB)
        {
            color4.r *= _displayedOpacity/255.0f;
            color4.g *= _displayedOpacity/255.0f;
            color4.b *= _displayedOpacity/255.0f;
        }
        auto quads = _textureAtlas->getQuads();
        ssize_t length = _string.length();
        for (int index = 0; index < length; index++)
        {
            quads[index].bl.colors = color4;
            quads[index].br.colors = color4;
            quads[index].tl.colors = color4;
            quads[index].tr.colors = color4;
            _textureAtlas->updateQuad(&quads[index], index);
        }
    }
}

//CCLabelAtlas - draw
#if CC_LABELATLAS_DEBUG_DRAW
void LabelAtlas::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    AtlasNode::draw(renderer, transform, _transformUpdated);

    _debugDrawNode->clear();
    auto size = getContentSize();
    Vec2 vertices[4]=
    {
        Vec2::ZERO,
        Vec2(size.width, 0),
        Vec2(size.width, size.height),
        Vec2(0, size.height)
    };
    _debugDrawNode->drawPoly(vertices, 4, true, Color4F(1.0, 1.0, 1.0, 1.0));
}
#endif

std::string LabelAtlas::getDescription() const
{
    return StringUtils::format("<LabelAtlas | Tag = %d, Label = '%s'>", _tag, _string.c_str());
}

//////////////////////////////LabelFrameAtlas///////////////////////////////////
LabelFrameAtlas::LabelFrameAtlas(const std::string &frame_name_format):_frameNameFormat(frame_name_format)
,_quads(nullptr)
,_quadCount(0)
,_quadCapacity(0)
,_isContentDirty(false)
,_isUseSecondary(false)
, _isColorDirty(false)
, _textureRef(nullptr)
, _blendFunc(BlendFunc::ALPHA_PREMULTIPLIED){
}

LabelFrameAtlas::~LabelFrameAtlas() {
	delete[] _quads;
	_quads = nullptr;
}

LabelFrameAtlas *LabelFrameAtlas::create(const std::string &frame_name_format) {
	LabelFrameAtlas *object = new LabelFrameAtlas(frame_name_format);
	if (object->init()) {
		object->autorelease();
		return object;
	}
	object->release();
	object = nullptr;
	return nullptr;
}

bool LabelFrameAtlas::init() {
	Node::init();
	setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
	return true;
}

void LabelFrameAtlas::setFrameFormat(const std::string &frame_format) {
	if (_frameNameFormat != frame_format) {
		_frameNameFormat = frame_format;
			_isContentDirty = true;
	}
}

void LabelFrameAtlas::setString(const std::string &str_content) {
	if (this->_strContent != str_content) {
		this->_strContent = str_content;
		_isContentDirty = true;
		_isUseSecondary = false;
	}
}

void LabelFrameAtlas::setString(const std::string &str_content, const std::string &secondary_format) {
	bool b2 = this->_strContent != str_content;
	bool b3 = this->_secondaryFormat != secondary_format;
	bool b4 = b2 || b3;

	if (b2)this->_strContent = str_content;
	if (b3)_secondaryFormat = secondary_format;
	

	if (b4 || !_isUseSecondary)
		_isContentDirty = true;

	_isUseSecondary = true;
}

const std::string& LabelFrameAtlas::getString()const {
	return _strContent;
}

void LabelFrameAtlas::updateQuadContent() {
	int str_length = _strContent.length();
	bool b2 = false;
	int   inc_number = 4;
	if (str_length > _quadCapacity) {
		b2 = true;
		inc_number = 16;
	}
	else if (str_length >16 && str_length < (_quadCapacity >> 2)) 
		b2 = true;
	
	if (b2) {
		delete[] _quads;
		_quads = new V3F_C4B_T2F_Quad[str_length + inc_number];
		_quadCapacity = str_length + inc_number;
	}
	_quadCount = str_length;
	Color4B	 color(_displayedColor.r, _displayedColor.g, _displayedColor.b,_displayedOpacity);

	const char *str = _strContent.c_str();
	const char *format = _isUseSecondary?_secondaryFormat.c_str():_frameNameFormat.c_str();
	char  c_array[2] = {0};
	int real_numner = 0;
	int texture_id = -1;//debug,所有的frame都必须来自于同一个纹理,否则报错
	float offset_x = 0;
	float max_height = 0.0f;
	for (int j = 0; j < str_length; ++j) {
		c_array[0] = str[j];
		char buffer[256];
		sprintf(buffer,format,c_array);
		SpriteFrame  *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(buffer);
		if (!frame) {
			CCLOG("could not find frame '%s'",buffer);
			continue;
		};
		_textureRef = frame->getTexture();
		int s2_id = _textureRef->getName();
		if (texture_id == -1)texture_id = s2_id;
		if (texture_id != s2_id) {
			CCLOG("could not render LabelFrameAtlas by diffent texture.");
			break;
		}

		real_numner += 1;
		V3F_C4B_T2F_Quad &quad = _quads[j];
		bool is_rotate = frame->isRotated();
		float width = _textureRef->getPixelsWide();
		float height = _textureRef->getPixelsHigh();
		const Rect &frame_rect = frame->getRect();
		auto &size = frame_rect.size;
		//注意cocos2d的纹理坐标与OpenGL的纹理坐标有着显著的不同
		float w = is_rotate ? size.height : size.width;
		float h = is_rotate ? size.width : size.height;

		float sw = w/width;
		float sh = h/height;

		float x0 = is_rotate?(frame_rect.origin.x + size.height)/width:frame_rect.origin.x/width;
		float y0 = frame_rect.origin.y/height;

		quad.tl.colors = color;
		quad.tl.texCoords.u = x0; quad.tl.texCoords.v = y0;
		quad.tl.vertices.x = offset_x; quad.tl.vertices.y = size.height; quad.tl.vertices.z = 0.0f;

		x0 = is_rotate ? x0 - sw:x0;
		y0 = is_rotate ? y0:y0 + sh;
		
		quad.bl.colors = color;
		quad.bl.texCoords.u = x0; quad.bl.texCoords.v = y0;
		quad.bl.vertices.x = offset_x; quad.bl.vertices.y = 0; quad.bl.vertices.z = 0.0f;

		x0 = is_rotate ? x0:x0+sw;
		y0 = is_rotate ? y0 + sh:y0;

		quad.br.colors = color;
		quad.br.texCoords.u = x0;quad.br.texCoords.v = y0;
		quad.br.vertices.x = offset_x + size.width; quad.br.vertices.y = 0.0f; quad.br.vertices.z = 0.0f;

		x0 = is_rotate ? x0 + sw:x0;
		y0 = is_rotate ? y0 : y0 - sh;

		quad.tr.colors = color;
		quad.tr.texCoords.u = x0 ;quad.tr.texCoords.v = y0;
		quad.tr.vertices.x = offset_x + size.width; quad.tr.vertices.y = size.height; quad.tr.vertices.z = 0.0f;

		offset_x += size.width;
		max_height = fmaxf(max_height,size.height);
	}
	setContentSize(Size(offset_x, max_height));
}

void LabelFrameAtlas::setColor(const Color3B& color) {
	if (color != this->_displayedColor) {
		Node::setColor(color);
		_isColorDirty = true;
	}
}

void LabelFrameAtlas::setOpacity(GLubyte opacity) {
	if (opacity != _displayedOpacity) {
		Node::setOpacity(opacity);
		_isColorDirty = true;
	}
}

void LabelFrameAtlas::updateQuadColor() {
	Color4B color(_displayedColor.r,_displayedColor.g,_displayedColor.b,_displayedOpacity);
	for (int j = 0; j < _quadCount; ++j) {
		V3F_C4B_T2F_Quad &quad = _quads[j];
		quad.tl.colors = color;
		quad.bl.colors = color;
		quad.br.colors = color;
		quad.tr.colors = color;
	}
}

void LabelFrameAtlas::setBlendFunc(const BlendFunc &blend) {
	_blendFunc = blend;
}

void LabelFrameAtlas::draw(Renderer *renderer, const Mat4& transform, uint32_t flags) {
	if (_isContentDirty) {
		updateQuadContent();
		_isContentDirty = false;
		_isColorDirty = false;
	}
	if (!_quadCount) return;
	if (_isColorDirty) {
		updateQuadColor();
		_isColorDirty = false;
	}
	bool visibility = renderer->checkVisibility(transform, _contentSize);
	if (visibility) {
		_quadCommand.init(_globalZOrder, _textureRef->getName(), _glProgramState, _blendFunc,_quads,_quadCount,transform, flags);
		renderer->addCommand(&_quadCommand);
	}
}

NS_CC_END
