////////////////////////////////////////////////////////////////////////////////
//
// Space Monkey Mafia - Procedural Jungle Generation
//
// URL: https://www.doc.gold.ac.uk/~mas01at/proj2013/?page_id=4
//
// Artemis Tsouflidou     [ma303at]
// Brian Gatt             [ma301bg]
// Stefana Ovesia         [ma301so]
// Gustavo Arcanjo Silva  [io301gas]
//
////////////////////////////////////////////////////////////////////////////////

namespace octet {

  class RainManager {
  private:
    octet::ref<octet::scene_node> _node;
    octet::ref<octet::mesh_particle_system> _system;
    octet::ref<octet::material> _material;
    octet::ref<octet::shader> _shader;

    camera_instance* _camera;

    float _width;
    float _length;

    bool _enabled;
    float _genRate;

    octet::vec3 _gravity;

    octet::vec3 getPosition() const {
      float x = octet::randf(&octet::LinearCongruential::getInstance()) * _width;
      float z = octet::randf(&octet::LinearCongruential::getInstance()) * _length;

      return octet::vec3(x, 0.0f, z) * _node->access_nodeToParent();
      //return octet::vec3(x, 0.0f, z);
    };
    
    octet::vec3 getVelocity() const {
      float y = octet::randf(&octet::LinearCongruential::getInstance(), 0.0f, 1.5f);

      return octet::vec3(0.0f, -y, 0.0f);
    };

    uint32_t getLifetime() const {
      return 150;
    };

    void addRainParticle() {
      addRainParticle(getPosition(), getVelocity(), getLifetime());
    };

    // Reference: example_particles.h
    void addRainParticle(const octet::vec3& position, const octet::vec3& velocity, uint32_t lifetime) {
      mesh_particle_system::billboard_particle particle;

      particle.pos = position;
      particle.size = vec2p(1.0f, 1.0f);

      particle.uv_bottom_left = vec2p(0.0f, 0.0f);
      particle.uv_top_right = vec2p(1.0f, 1.0f);
      particle.enabled = true;

      int particleID = _system->add_billboard_particle(particle);

      mesh_particle_system::particle_animator animator;
	  
      animator.link = particleID;
      animator.acceleration = _gravity;
      animator.vel = velocity;
      animator.lifetime = lifetime;
      
      _system->add_particle_animator(animator);
    };

  public:
    RainManager() :
      _node(NULL),
      _system(NULL),
      _material(NULL),
      _shader(NULL),
      _camera(NULL),
      _width(10.f),
      _length(10.f),
      _enabled(false),
      _genRate(100.0f),
      _gravity(0.0f, -9.8f, 0.0f) {
    };

    void reset(octet::visual_scene* scene, const Bounds<octet::vec2>& jungleBounds, float yOffset) {
      reset(
        scene,
        // width
        jungleBounds.max.x() - jungleBounds.min.x(),
        // length
        jungleBounds.max.y() - jungleBounds.min.y(),
        // position
        //octet::vec3(jungleBounds.min.x(), yOffset, jungleBounds.min.y())
        octet::vec3(0.0f, yOffset, 0.0f)
      );
    };
    
    void reset(octet::visual_scene* scene, float width, float length, const octet::vec3& position) {
       reset(scene, scene->get_camera_instance(0), width, length, position);
    };
    
    void reset(octet::visual_scene* scene, camera_instance* camera, float width, float length, const octet::vec3& position) {
      _width = width;
      _length = length;

      _camera = camera;

      _node = scene->add_scene_node();
      _node->translate(position);
      
      _system = new octet::mesh_particle_system(octet::aabb(octet::vec3(0.0f), octet::vec3(_width / 2.0f, _length / 2.0f, 0.0f)), 512, 0, 512);

      if (!_material) {
        _material = new octet::material(new octet::image("assets/jungle/droplet.gif"));
      }

      if (!_shader) {
        _shader = new octet::alpha_texture_shader();
        _shader->init();
      }

      octet::mesh_instance* instance = new octet::mesh_instance(
        _node,
        _system,
        _material,
        _shader
      );

      scene->add_mesh_instance(instance);
    };
    
    float getGenerationRate() const {
      return _genRate;
    };

    void setGenerationRate(float genRate) {
      _genRate = genRate;
    };
    
    octet::vec3 getGravity() const {
      return _gravity;
    };

    void setGravity(const octet::vec3& gravity) {
      _gravity = gravity;
    };

    bool isEnabled() const {
      return _enabled;
    };

    void setEnabled(bool enabled) {
      _enabled = enabled;
    };

    void update(float deltaTime) {
      // Always update underlying particle system irregardles of enable state
      _system->set_cameraToWorld(_camera->get_node()->calcModelToWorld());
	    _system->animate(deltaTime);
      _system->update();

      // The enabled flag toggles generation of new rain particles
      if (_enabled) {
        float particlesToGenerate = _genRate * deltaTime;
        size_t particlesToGenerateFully = (size_t) ::floor(particlesToGenerate);

        if (randf(&octet::LinearCongruential::getInstance()) < (particlesToGenerate - particlesToGenerateFully)) {
          ++particlesToGenerateFully;
        }

        for (size_t i = 0; i < particlesToGenerateFully; ++i) {
          addRainParticle();
        }
      }
    };
  };

} // namespace octet