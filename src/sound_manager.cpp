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

#include "openal_system.hpp"
#include "sound_file.hpp"
#include "sound_manager.hpp"
#include "dummy_sound_source.hpp"
#include "static_sound_source.hpp"
#include "stream_sound_source.hpp"

SoundManager::SoundManager() :
  m_openal(),
  m_channels(),
  m_buffer_cache()
{
  m_channels.emplace_back(std::make_unique<SoundChannel>(*this));
  m_channels.emplace_back(std::make_unique<SoundChannel>(*this));
  m_channels.emplace_back(std::make_unique<SoundChannel>(*this));
}

SoundManager::~SoundManager()
{
}

ALuint
SoundManager::load_file_into_buffer(std::filesystem::path const& filename)
{
  std::unique_ptr<SoundFile> file(SoundFile::from_file(filename));

  std::vector<char> samples(file->get_size());
  size_t total_bytesread = 0;
  while (true) {
    size_t bytesread = file->read(samples.data() + total_bytesread, 1024 * 64);
    if (bytesread == 0) {
      break;
    }
    total_bytesread += bytesread;

    assert(total_bytesread <= file->get_size());
  }

  return m_openal.create_buffer(OpenALSystem::get_sample_format(file.get()),
                                samples.data(),
                                static_cast<ALsizei>(total_bytesread),
                                file->get_rate());
}

SoundSourcePtr
SoundManager::create_sound_source(std::unique_ptr<SoundFile> sound_file,
                                  SoundChannel& channel,
                                  SoundSourceType type)
{
  if (m_openal.is_dummy())
  {
    return SoundSourcePtr(new DummySoundSource());
  }
  else
  {
    switch(type)
    {
      case SoundSourceType::STATIC:
        assert(false && "not supported");
        return SoundSourcePtr(new DummySoundSource());

      case SoundSourceType::STREAM:
        return SoundSourcePtr(new StreamSoundSource(channel, std::move(sound_file)));

      default:
        assert(false && "never reached");
        return SoundSourcePtr();
    }
  }
}

SoundSourcePtr
SoundManager::create_sound_source(std::filesystem::path const& filename, SoundChannel& channel,
                                  SoundSourceType type)
{
  if (m_openal.is_dummy())
  {
    return SoundSourcePtr(new DummySoundSource());
  }
  else
  {
    switch(type)
    {
      case SoundSourceType::STATIC:
      {
        ALuint buffer;

        // reuse an existing static sound buffer
        auto it = m_buffer_cache.find(filename);
        if (it != m_buffer_cache.end()) {
          buffer = it->second;
        } else {
          buffer = load_file_into_buffer(filename);
          m_buffer_cache.insert(std::make_pair(filename, buffer));
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
