/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include <gtest/gtest.h>

#include "sound/sound_file.hpp"
#include "sound/sound_manager.hpp"
#include "sound/sound_source.hpp"

TEST(SoundSourceTest, duration)
{
  SoundManager mgr;

  auto wav_file = SoundFile::from_file("data/sound.wav");
  auto ogg_file = SoundFile::from_file("data/sound.ogg");
  auto opus_file = SoundFile::from_file("data/sound.opus");

  auto wav_static = mgr.sound().prepare("data/sound.wav", SoundSourceType::STATIC);
  auto ogg_static = mgr.sound().prepare("data/sound.ogg", SoundSourceType::STATIC);
  auto opus_static = mgr.sound().prepare("data/sound.opus", SoundSourceType::STATIC);

  auto wav_stream = mgr.sound().prepare("data/sound.wav", SoundSourceType::STREAM);
  auto ogg_stream = mgr.sound().prepare("data/sound.ogg", SoundSourceType::STREAM);
  auto opus_stream = mgr.sound().prepare("data/sound.opus", SoundSourceType::STREAM);

  EXPECT_EQ(wav_file->get_duration(), wav_static->get_duration());
  EXPECT_EQ(ogg_file->get_duration(), ogg_static->get_duration());
  EXPECT_EQ(opus_file->get_duration(), opus_static->get_duration());

  EXPECT_EQ(wav_file->get_duration(), wav_stream->get_duration());
  EXPECT_EQ(ogg_file->get_duration(), ogg_stream->get_duration());
  EXPECT_EQ(opus_file->get_duration(), opus_stream->get_duration());
}

/* EOF */
