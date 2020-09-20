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

#include "sound/sound_channel.hpp"

class SoundFile;
class SoundSource;
class StreamSoundSource;

class SoundManager
{
public:
  SoundManager();
  ~SoundManager();

  void set_listener_position(float x, float y, float z);
  void set_listener_velocity(float x, float y, float z);

  // master volume is not clamped
  void set_gain(float gain);

  SoundChannel& sound() { return *m_channels[0]; }
  SoundChannel& music() { return *m_channels[1]; }
  SoundChannel& voice() { return *m_channels[2]; }

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
                                     SoundSourceType type);

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

  std::vector<std::unique_ptr<SoundChannel> > m_channels;

  std::map<std::filesystem::path, ALuint> m_buffers;

private:
  SoundManager(const SoundManager&);
  SoundManager& operator=(const SoundManager&);
};

#endif

/* EOF */
