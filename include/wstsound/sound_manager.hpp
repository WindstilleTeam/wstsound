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

#include "openal_system.hpp"
#include "sound_channel.hpp"

namespace wstsound {

class SoundFile;
class SoundSource;
class StreamSoundSource;

class SoundManager
{
public:
  SoundManager(std::unique_ptr<OpenALSystem> openal);
  SoundManager();
  ~SoundManager();

  bool is_dummy() const { return !m_openal; }

  void set_listener_position(float x, float y, float z);
  void set_listener_velocity(float x, float y, float z);
  void set_listener_orientation(float at_x, float at_y, float at_z,
                                float up_x, float up_y, float up_z);

  // master volume is not clamped
  void set_gain(float gain);

  SoundChannel& sound() { return *m_channels[0]; }
  SoundChannel& music() { return *m_channels[1]; }
  SoundChannel& voice() { return *m_channels[2]; }

  void update(float delta);

  void preload(std::filesystem::path const& filename);

  /**
   * Creates a new sound source object which plays the specified soundfile.
   * You are responsible for deleting the sound source later (this will stop the
   * sound).
   * This function might throw exceptions. It returns 0 if no audio device is
   * available.
   */
  SoundSourcePtr create_sound_source(std::filesystem::path const& filename,
                                     SoundChannel& channel,
                                     SoundSourceType type);

  SoundSourcePtr create_sound_source(std::unique_ptr<SoundFile> sound_file,
                                     SoundChannel& channel,
                                     SoundSourceType type);

  EffectSlotPtr create_effect_slot();
  EffectPtr create_effect(ALuint effect_type);
  FilterPtr create_filter(ALuint filter_type);

private:
  OpenALBuffer load_file_into_buffer(std::unique_ptr<SoundFile> file);

private:
  std::unique_ptr<OpenALSystem> m_openal;
  std::vector<std::unique_ptr<SoundChannel> > m_channels;
  std::map<std::filesystem::path, OpenALBuffer> m_buffer_cache;

public:
  SoundManager(const SoundManager&);
  SoundManager& operator=(const SoundManager&);
};

} // namespace wstsound

#endif

/* EOF */
