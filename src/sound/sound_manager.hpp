/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEADER_WINDSTILLE_SOUND_SOUND_MANAGER_HPP
#define HEADER_WINDSTILLE_SOUND_SOUND_MANAGER_HPP

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include <alc.h>
#include <al.h>
#include <glm/glm.hpp>

#include "sound/sound_channel.hpp"

class SoundFile;
class SoundSource;
class StreamSoundSource;

class SoundManager
{
public:
  SoundManager();
  ~SoundManager();

  void enable_sound(bool sound_enabled);
  void enable_music(bool music_enabled);

  void set_listener_position(const glm::vec2& position);
  void set_listener_velocity(const glm::vec2& velocity);

  // master volume is not clamped
  void set_gain(float gain);

  SoundChannel& voice() { return m_voice_channel; }
  SoundChannel& sound() { return m_sound_channel; }
  SoundChannel& music() { return m_music_channel; }

  /**
   * Convenience function to simply play a sound at a given position.
   */
  SoundSourcePtr play(std::filesystem::path const& filename, const glm::vec2& pos = glm::vec2(-1, -1));

  void play_music(std::filesystem::path const& filename, bool fade = true);
  void stop_music(bool fade = true);

  void update(float delta);

  /**
   * Creates a new sound source object which plays the specified soundfile.
   * You are responsible for deleting the sound source later (this will stop the
   * sound).
   * This function might throw exceptions. It returns 0 if no audio device is
   * available.
   */
  SoundSourcePtr create_sound_source(std::filesystem::path const& filename,
                                     SoundChannel& channel,
                                     OpenALSoundSourceType type);

public:
  static void check_al_error(const char* message);
  static ALenum get_sample_format(SoundFile* file);

private:
  static ALuint load_file_into_buffer(std::filesystem::path const& filename);

  void print_openal_version();
  void check_alc_error(const char* message);

  ALCdevice*  m_device;
  ALCcontext* m_context;
  bool m_sound_enabled;

  SoundChannel m_voice_channel;
  SoundChannel m_sound_channel;
  SoundChannel m_music_channel;

  std::map<std::filesystem::path, ALuint> m_buffers;
  std::vector<SoundSourcePtr> m_sources;

private:
  SoundManager(const SoundManager&);
  SoundManager& operator=(const SoundManager&);
};

#endif

/* EOF */
