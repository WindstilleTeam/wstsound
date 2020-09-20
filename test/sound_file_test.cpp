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

#include "sound/ogg_sound_file.hpp"
#include "sound/opus_sound_file.hpp"
#include "sound/wav_sound_file.hpp"

TEST(SoundFileTest, wav)
{
  auto fin = std::make_unique<std::ifstream>("data/sound.wav");
  WavSoundFile sound_file(std::move(fin));

  EXPECT_EQ(sound_file.get_bits_per_sample(), 16);
  EXPECT_EQ(sound_file.get_size(), 22788);
  EXPECT_EQ(sound_file.get_rate(), 44100);
  EXPECT_EQ(sound_file.get_channels(), 1);
  EXPECT_FLOAT_EQ(sound_file.get_duration(), 0.25836736f);
}

TEST(SoundFileTest, ogg)
{
  auto fin = std::make_unique<std::ifstream>("data/sound.ogg");
  OggSoundFile sound_file(std::move(fin));

  EXPECT_EQ(sound_file.get_bits_per_sample(), 16);
  EXPECT_EQ(sound_file.get_size(), 22788);
  EXPECT_EQ(sound_file.get_rate(), 44100);
  EXPECT_EQ(sound_file.get_channels(), 1);
  EXPECT_FLOAT_EQ(sound_file.get_duration(), 0.25836736f);
}

TEST(SoundFileTest, opus)
{
  auto fin = std::make_unique<std::ifstream>("data/sound.opus");
  OpusSoundFile sound_file(std::move(fin));

  EXPECT_EQ(sound_file.get_bits_per_sample(), 16);
  EXPECT_EQ(sound_file.get_size(), 24804);
  EXPECT_EQ(sound_file.get_rate(), 48000);
  EXPECT_EQ(sound_file.get_channels(), 1);
  EXPECT_FLOAT_EQ(sound_file.get_duration(), 0.25837499f);
}

/* EOF */
