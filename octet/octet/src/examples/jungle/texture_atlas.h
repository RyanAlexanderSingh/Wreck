namespace octet {

  class TextureInfo {
  private:
    octet::string _name;

    unsigned int _x;
    unsigned int _y;
    
    unsigned int _width;
    unsigned int _height;

  public:
    TextureInfo() :
      _name(),
      _x(0),
      _y(0),
      _width(0),
      _height(0) {
    };

    ~TextureInfo() {
    };

    const octet::string& getName() const {
      return _name;
    };
    
    octet::string& getName() {
      return _name;
    };

    unsigned int getX() const {
      return _x;
    };
    
    void setX(unsigned int x) {
      _x = x;
    };

    unsigned int getY() const {
      return _y;
    };
    
    void setY(unsigned int y) {
      _y = y;
    };

    unsigned int getWidth() const {
      return _width;
    };
    
    void setWidth(unsigned int width) {
      _width = width;
    };

    unsigned int getHeight() const {
      return _height;
    };
    
    void setHeight(unsigned int height) {
      _height = height;
    };
  };

  class TextureAtlasInfo {
  private:
    octet::string _imagePath;

    unsigned int _width;
    unsigned int _height;

    dynarray<TextureInfo> _textures;

  public:
    TextureAtlasInfo() :
      _imagePath(),
      _width(0),
      _height(0) {
    };

    ~TextureAtlasInfo() {
    };
    
    const octet::string& getPath() const {
      return _imagePath;
    };
    
    octet::string& getPath() {
      return _imagePath;
    };
    
    unsigned int getWidth() const {
      return _width;
    };
    
    void setWidth(unsigned int width) {
      _width = width;
    };

    unsigned int getHeight() const {
      return _height;
    };
    
    void setHeight(unsigned int height) {
      _height = height;
    };

    const dynarray<TextureInfo>& getTextures() const {
      return _textures;
    };
    
    dynarray<TextureInfo>& getTextures() {
      return _textures;
    };

    const TextureInfo* getTextureByName(const char* name) const {
      for (size_t i = 0; i < getTextures().size(); ++i) {
        if (::strcmp(name, getTextures()[i].getName().c_str()) == 0) {
          return &getTextures()[i];
        }
      }

      return NULL;
    };
  };
  
  Bounds<octet::math::vec2> getUV(const TextureInfo& tex, const TextureAtlasInfo& atlas) {
    return Bounds<octet::math::vec2>(
      octet::math::vec2(((float) tex.getX() / (float) atlas.getWidth()), ((float) tex.getY() / (float) atlas.getHeight())),
      octet::math::vec2(((float) (tex.getX() + tex.getWidth()) / (float) atlas.getWidth()), ((float) (tex.getY() + tex.getHeight())  / (float) atlas.getHeight()))
    );
  };
  
  Bounds<octet::math::vec2> getUV(const TextureInfo* tex, const TextureAtlasInfo& atlas) {
    return (tex == NULL ? Bounds<octet::math::vec2>() : getUV(*tex, atlas));
  };

  class TextureAtlasInfoXMLParser {
  private:
    
    void parseSpriteAttributes(const TiXmlElement& element, TextureInfo& texInfo) const {
      texInfo.getName() = element.Attribute("n");
      
      int temp = 0;
      element.Attribute("x", &temp);
      texInfo.setX(temp);

      temp = 0;
      element.Attribute("y", &temp);
      texInfo.setY(temp);
      
      temp = 0;
      element.Attribute("w", &temp);
      texInfo.setWidth(temp);

      temp = 0;
      element.Attribute("h", &temp);
      texInfo.setHeight(temp);
    };

    void parseSprite(const TiXmlElement& element, TextureAtlasInfo& atlasInfo) const {
      if (strcmp(element.Value(), "sprite") == 0) {
        TextureInfo info;
        parseSpriteAttributes(element, info);
        atlasInfo.getTextures().push_back(info);
      }
    };
    
    void parseAtlasAttributes(const TiXmlElement& element, TextureAtlasInfo& atlasInfo) const {
      atlasInfo.getPath() = element.Attribute("imagePath");
      
      int temp = 0;
      element.Attribute("width", &temp);
      atlasInfo.setWidth(temp);

      temp = 0;
      element.Attribute("height", &temp);
      atlasInfo.setHeight(temp);
    };

    TextureAtlasInfo parse(const TiXmlDocument& xml) const {
      TextureAtlasInfo info;
    
      const TiXmlElement* root = xml.FirstChildElement();
      if (root != NULL && strcmp(root->Value(), "TextureAtlas") == 0) {
        parseAtlasAttributes(*root, info);

        const TiXmlElement* child = root->FirstChildElement();
        for (; child != NULL; child = child->NextSiblingElement()) {
          parseSprite(*child, info);
        };
      }

      return info;
    };

  public:
    TextureAtlasInfoXMLParser() {
    };

    TextureAtlasInfo parse(const char* path) const {
      TiXmlDocument doc;
      if (doc.LoadFile(path)) {
        return parse(doc);
      }

      return TextureAtlasInfo();
    };
  };

  class TextureAtlasInfoResource : public octet::resource {
  private:
    mutable TextureAtlasInfo _info;

    octet::string _path;
    bool _loaded;

  public:
    TextureAtlasInfoResource(const char* path) :
      _path(app_utils::get_path(path)),
      _loaded(false) {
    };

    void load() {
      TextureAtlasInfoXMLParser parser;
      _info = parser.parse(_path);
      _loaded = true;
    };

    const TextureAtlasInfo& get() const {
      if (!_loaded) {
        const_cast<TextureAtlasInfoResource*>(this)->load();
      }

      return _info;
    };
  };

} // namespace octet