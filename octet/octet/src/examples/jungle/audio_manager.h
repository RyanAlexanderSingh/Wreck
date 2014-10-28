////////////////////////////////////////////////////////////////////////////////
//
// (C) Gustavo Arcanjo 2014
// (C) Space Monkey Mafia 2014
//
// Advanced Games Programming @ Goldsmiths
// Procedural Jungle Generation Project
//

namespace octet {

  // Singleton audio_manager
  class audio_manager {

    enum {
      num_sound_sources = 8,

    };

    audio_manager() {
      alGenSources(num_sound_sources, sources);
    }

    audio_manager(audio_manager const&);
    void operator=(audio_manager const&);

    // random number generator
    class random randomizer;

    dictionary<dynarray<ALuint>> dict;

    ALuint cur_source;
    ALuint sources[num_sound_sources];
    dictionary<int> dedicated_sources;

    ALuint get_sound_source() {
      ALuint source = sources[++cur_source % num_sound_sources];
      if (is_source_playing(source) && cur_source < num_sound_sources) {
        return get_sound_source();
      }
      if (cur_source >= num_sound_sources) cur_source = 0;
      return source;
    }
    ALuint get_sound_source(const char *name) {
      if (!dedicated_sources.contains(name)) {
        dedicated_sources[name] = get_sound_source();
      }
      return dedicated_sources[name];
    }

    ALint any_on(const char *name) {
      int i = randomizer.get(0, dict[name].size());
      return dict[name][i];
    }

    ALint get_sound_value(const char* name) {
      if (name == 0 || name[0] == 0) {
        return NULL;
      }

      if (!dict.contains(name)) {
        return NULL;
      } else {
        if (dict[name].empty()) {
          return NULL;
        } else {
          return any_on(name);
        }
      }
    }

    bool is_source_playing(ALuint sourceID) {
      ALint state;

      alGetSourcei(sourceID, AL_SOURCE_STATE, &state);

      return state == AL_PLAYING;
    }

    public:

    static audio_manager &get_instance() {
      static audio_manager instance;
      
      return instance;
    }

    void play(const char* group_name) {
      if (group_name && group_name[0] && dict.contains(group_name)){
        ALuint source = get_sound_source();
        ALint value = get_sound_value(group_name);
        alSourcei(source, AL_BUFFER, value);
        alSourcePlay(source);
      } else {
        DEBUG_PRINT("audio_manager/play failed, invalid group name %s\n", group_name);
      }
    }

    void play_on_silence(const char* group_name) {
      if (group_name && group_name[0] && dict.contains(group_name)){
        ALuint source = get_sound_source(group_name);
        if (!is_source_playing(source)) {
          ALint value = get_sound_value(group_name);
          alSourcei(source, AL_BUFFER, value);
          alSourcePlay(source);
        }
      } else {
        DEBUG_PRINT("audio_manager/play_on_silence failed, invalid group name %s\n", group_name);
      }
    }

    void start_on_loop(const char* group_name) {
      if (group_name && group_name[0] && dict.contains(group_name)){
        ALuint source = get_sound_source(group_name);
        if (!is_source_playing(source)) {
          ALint value = get_sound_value(group_name);
          alSourcei(source, AL_BUFFER, value);
          alSourcei(source, AL_LOOPING, true);
          alSourcePlay(source);
        }
      }
    }

    void stop_playing(const char* group_name) {
      if (group_name && group_name[0] && dict.contains(group_name)){
        ALuint source = get_sound_source(group_name);
        if (is_source_playing(source)) {
          alSourceStop(source);
        }
      }
    }

    bool add_sound_to_group(const char* path, const char* group_name) {
      if (group_name && group_name[0]) {
        ALint value = resources::resource_dict::get_sound_handle(AL_FORMAT_MONO16, path);
        dict[group_name].push_back(value);
        return true;
      }
      DEBUG_PRINT("audio_manager/add_sound_to_group failed, invalid group name\n");
      return false;
    }

    bool add_numbered_sounds_to_group(const char* prefix, unsigned int start, unsigned int end, const char* suffix, const char* group_name) {
      if (end <= start || end > 99) {
        DEBUG_PRINT("audio_manager/add_numerated_sounds_to_group failed, invalid start or end\n");
        return false;
      }
      if (prefix && prefix[0] && suffix && suffix[0]) {
        for (unsigned int i=start; i<=end; ++i) {
          string path(prefix);
          path += ('0'+i/10);
          path += ('0'+i%10);
          path += suffix;
          if (!add_sound_to_group(path.c_str(), group_name)){
            return false;
          }
        }

        return true;
      }
      DEBUG_PRINT("audio_manager/add_numerated_sounds_to_group failed, invalid prefix or suffix\n");
      return false;
    }

    bool remove_group(const char* group_name) {
      if (group_name && group_name[0] && dict.contains(group_name)) {
        dict[group_name].reset();
        return true;
      }
      DEBUG_PRINT("audio_manager/add_sound_to_group failed, invalid group name\n");
      return false;
    }
  
  };
}