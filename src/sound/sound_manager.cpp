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

#include <assert.h>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include "sound/openal_system.hpp"
#include "sound/sound_file.hpp"
#include "sound/sound_manager.hpp"
#include "sound/static_sound_source.hpp"
#include "sound/stream_sound_source.hpp"

SoundManager::SoundManager() :
  m_openal(),
  m_channels(),
  m_buffers()
{
  m_channels.emplace_back(std::make_unique<SoundChannel>(*this));
  m_channels.emplace_back(std::make_unique<SoundChannel>(*this));
  m_channels.emplace_back(std::make_unique<SoundChannel>(*this));
}

SoundManager::~SoundManager()
{
  for(std::unique_ptr<SoundChannel>& channel : m_channels) {
    channel->clear();
  }

  for(auto const& it : m_buffers)
  {
    alDeleteBuffers(1, &it.second);
  }
}

ALuint
SoundManager::load_file_into_buffer(std::filesystem::path const& filename)
{
  // open sound file
  std::unique_ptr<SoundFile> file(SoundFile::from_file(filename));

  ALenum format = OpenALSystem::get_sample_format(file.get());
  ALuint buffer;
  alGenBuffers(1, &buffer);
  OpenALSystem::check_al_error("Couldn't create audio buffer: ");

  try
  {
    std::vector<char> samples(file->get_size());

    file->read(samples.data(), file->get_size());

    alBufferData(buffer, format, samples.data(),
                 static_cast<ALsizei>(file->get_size()),
                 file->get_rate());

    OpenALSystem::check_al_error("Couldn't fill audio buffer: ");
  }
  catch(...)
  {
    throw;
  }

  return buffer;
}

SoundSourcePtr
SoundManager::create_sound_source(std::filesystem::path const& filename, SoundChannel& channel,
                                  SoundSourceType type)
{
  if (!m_openal.sound_enabled())
  {
    return SoundSourcePtr();
  }
  else
  {
    switch(type)
    {
      case SoundSourceType::STATIC:
      {
        ALuint buffer;

        // reuse an existing static sound buffer
        auto it = m_buffers.find(filename);
        if (it != m_buffers.end()) {
          buffer = it->second;
        } else {
          buffer = load_file_into_buffer(filename);
          m_buffers.insert(std::make_pair(filename, buffer));
        }

        return SoundSourcePtr(new StaticSoundSource(channel, buffer));
      }
      break;

      case SoundSourceType::STREAM:
      {
        std::unique_ptr<SoundFile> sound_file = SoundFile::from_file(filename);
        return SoundSourcePtr(new StreamSoundSource(channel, std::move(sound_file)));
      }
      break;

      default:
        assert(false && "never reached");
        return SoundSourcePtr();
    }
  }
}

void
SoundManager::set_listener_position(float x, float y, float z)
{
  alListener3f(AL_POSITION, x, y, z);
}

void
SoundManager::set_listener_velocity(float x, float y, float z)
{
  alListener3f(AL_VELOCITY, x, y, z);
}

void
SoundManager::set_gain(float gain)
{
  alListenerf(AL_GAIN, gain);
}

void
SoundManager::update(float delta)
{
  for(std::unique_ptr<SoundChannel>& channel : m_channels) {
    channel->update(delta);
  }

  m_openal.update();
}

/* EOF */
