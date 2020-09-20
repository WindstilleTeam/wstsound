/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
**                2020 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_WINDSTILLE_SOUND_OPENAL_SYSTEM_HPP
#define HEADER_WINDSTILLE_SOUND_OPENAL_SYSTEM_HPP

#include <map>
#include <vector>

#include <alc.h>
#include <al.h>

class SoundFile;

class OpenALSystem
{
public:
  static void check_al_error(const char* message);
  static ALenum get_sample_format(SoundFile* file);

public:
  OpenALSystem();
  ~OpenALSystem();

  ALCdevice*  device() { return m_device; }
  ALCcontext* context() { return m_context; }

  void print_openal_version();
  void check_alc_error(char const* message);

  bool sound_enabled() const { return m_sound_enabled; }

  /** Create an OpenAL buffer, the returned handle is held by
      OpenALSystem and must not be deleted */
  ALuint create_buffer(ALenum format, ALvoid const* data, ALsizei size, ALsizei freq);
  void update();

private:
  ALCdevice*  m_device;
  ALCcontext* m_context;
  bool m_sound_enabled;
  std::vector<ALuint> m_buffers;

public:
  OpenALSystem(const OpenALSystem&) = delete;
  OpenALSystem& operator=(const OpenALSystem&) = delete;
};

#endif

/* EOF */
