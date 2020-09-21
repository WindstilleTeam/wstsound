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

#include "openal_system.hpp"

#include <iostream>
#include <stdexcept>

#include "sound_file.hpp"

OpenALSystem::OpenALSystem() :
  m_device(nullptr),
  m_context(nullptr),
  m_buffers()
{
  try
  {
    m_device = alcOpenDevice(nullptr);

    if (!m_device)
    {
      print_openal_version(std::cerr);
      throw std::runtime_error("Couldn't open audio device.");
    }
    else
    {
      m_context = alcCreateContext(m_device, nullptr);
      check_alc_error("Couldn't create audio context: ");

      alcMakeContextCurrent(m_context);
      check_alc_error("Couldn't select audio context: ");

      check_al_error("Audio error after init: ");
    }
  }
  catch(std::exception& e)
  { // disable sound
    m_device  = nullptr;
    m_context = nullptr;

    std::cerr << "Couldn't initialize audio device:" << e.what() << "\n";
    print_openal_version(std::cerr);

    std::cerr << "Disabling sound\n";
  }

}

OpenALSystem::~OpenALSystem()
{
  alDeleteBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data());
  m_buffers.clear();

  if (m_context) {
    alcDestroyContext(m_context);
  }

  if (m_device) {
    alcCloseDevice(m_device);
  }
}

ALuint
OpenALSystem::create_buffer(ALenum format,
                            ALvoid const* data,
                            ALsizei size,
                            ALsizei freq)
{
  ALuint buffer;
  alGenBuffers(1, &buffer);
  OpenALSystem::check_al_error("Couldn't create audio buffer: ");

  alBufferData(buffer, format, data, size, freq);

  OpenALSystem::check_al_error("Couldn't fill audio buffer: ");

  m_buffers.push_back(buffer);

  return buffer;
}

void
OpenALSystem::update()
{
  if (m_context) {
    alcProcessContext(m_context);
    check_alc_error("Error while processing audio context: ");
  }
}

ALenum
OpenALSystem::get_sample_format(SoundFile* file)
{
  if (file->get_channels() == 2)
  {
    if (file->get_bits_per_sample() == 16)
    {
      return AL_FORMAT_STEREO16;
    }
    else if (file->get_bits_per_sample() == 8)
    {
      return AL_FORMAT_STEREO8;
    }
    else
    {
      throw std::runtime_error("Only 16 and 8 bit samples supported");
    }
  }
  else if (file->get_channels() == 1)
  {
    if (file->get_bits_per_sample() == 16)
    {
      return AL_FORMAT_MONO16;
    }
    else if (file->get_bits_per_sample() == 8)
    {
      return AL_FORMAT_MONO8;
    }
    else
    {
      throw std::runtime_error("Only 16 and 8 bit samples supported");
    }
  }
  else
  {
    throw std::runtime_error("Only 1 and 2 channel samples supported");
  }
}

void
OpenALSystem::print_openal_version(std::ostream& out)
{
  out << "OpenAL Vendor: " << alGetString(AL_VENDOR) << "\n"
      << "OpenAL Version: " << alGetString(AL_VERSION) << "\n"
      << "OpenAL Renderer: " << alGetString(AL_RENDERER) << "\n"
      << "OpenAL Extensions: " << alGetString(AL_EXTENSIONS) << "\n";
}

void
OpenALSystem::check_alc_error(const char* message)
{
  int err = alcGetError(m_device);

  if (err != ALC_NO_ERROR)
  {
    std::stringstream msg;
    msg << message << alcGetString(m_device, err);
    throw std::runtime_error(msg.str());
  }
}

void
OpenALSystem::check_al_error(const char* message)
{
  int err = alGetError();

  if (err != AL_NO_ERROR)
  {
    std::stringstream msg;
    msg << message << alGetString(err);
    throw std::runtime_error(msg.str());
  }
}

/* EOF */
