//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_WSTSOUND_OPENAL_BUFFER_HPP
#define HEADER_WSTSOUND_OPENAL_BUFFER_HPP

#include <memory>

#include <al.h>

#include <wstsound/openal_system.hpp>

namespace wstsound {

class OpenALBufferImpl
{
public:
  OpenALBufferImpl() :
    m_handle()
  {
    alGenBuffers(1, &m_handle);
    OpenALSystem::check_al_error("Couldn't create audio buffer: ");
  }

  ~OpenALBufferImpl()
  {
    alDeleteBuffers(1, &m_handle);
    OpenALSystem::warn_al_error("Couldn't delete audio buffer: ");
  }

public:
  ALuint m_handle;

private:
  OpenALBufferImpl(const OpenALBufferImpl&) = delete;
  OpenALBufferImpl& operator=(const OpenALBufferImpl&) = delete;
};

/** A refcounted wrapper around an OpenAL buffer */
class OpenALBuffer
{
public:
  OpenALBuffer() :
    m_impl(std::make_shared<OpenALBufferImpl>())
  {
  }

  OpenALBuffer(OpenALBuffer&& other) :
    m_impl(std::move(other.m_impl))
  {
  }

  OpenALBuffer(OpenALBuffer const& other) :
    m_impl(other.m_impl)
  {
  }

  OpenALBuffer& operator=(OpenALBuffer const& other)
  {
    m_impl = other.m_impl;
    return *this;
  }

  ~OpenALBuffer()
  {
  }

  ALuint get_handle() const
  {
    return m_impl->m_handle;
  }

  int get_sample_duration() const
  {
    ALint frequency;
    alGetBufferi(get_handle(), AL_FREQUENCY, &frequency);

    ALint bits;
    alGetBufferi(get_handle(), AL_BITS, &bits);

    ALint channels;
    alGetBufferi(get_handle(), AL_CHANNELS, &channels);

    ALint size;
    alGetBufferi(get_handle(), AL_SIZE, &size);

    return 8 * size / channels / bits;
  }

  float get_duration() const
  {
    ALint frequency;
    alGetBufferi(get_handle(), AL_FREQUENCY, &frequency);

    ALint bits;
    alGetBufferi(get_handle(), AL_BITS, &bits);

    ALint channels;
    alGetBufferi(get_handle(), AL_CHANNELS, &channels);

    ALint size;
    alGetBufferi(get_handle(), AL_SIZE, &size);

    return static_cast<float>(size)
      / static_cast<float>(frequency)
      / static_cast<float>(channels)
      / static_cast<float>(bits) * 8.0f;
  }

private:
  std::shared_ptr<OpenALBufferImpl> m_impl;
};

} // namespace wstsound

#endif

/* EOF */
