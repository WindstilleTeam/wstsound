{
  description = "Windstille Sound Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    openal-soft_src.url = "github:kcat/openal-soft?ref=1.22.2";
    openal-soft_src.flake = false;

    libmodplug_src.url = "https://downloads.sourceforge.net/project/modplug-xmms/libmodplug/0.8.9.0/libmodplug-0.8.9.0.tar.gz";
    libmodplug_src.flake = false;

    libogg_src.url = "https://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz";
    libogg_src.flake = false;

    libvorbis_src.url = "https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz";
    libvorbis_src.flake = false;

    opus_src.url = "https://archive.mozilla.org/pub/opus/opus-1.3.1.tar.gz";
    opus_src.flake = false;

    opusfile_src.url = "https://downloads.xiph.org/releases/opus/opusfile-0.12.tar.gz";
    opusfile_src.flake = false;

    mpg123_src.url = "https://downloads.sourceforge.net/project/mpg123/mpg123/1.30.2/mpg123-1.30.2.tar.bz2";
    mpg123_src.flake = false;
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc,
              openal-soft_src,
              libmodplug_src,
              libogg_src,
              libvorbis_src,
              opus_src,
              opusfile_src,
              mpg123_src
            }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = wstsound;

          # nix build -L .#packages.x86_64-w64-mingw32.SDL2
          openal-soft = pkgs.stdenv.mkDerivation {
            pname = "openal-soft";
            version = "1.22.2";
            src = openal-soft_src;
            nativeBuildInputs = [
              pkgs.buildPackages.cmake
            ];
          };

          # nix build -L .#packages.x86_64-w64-mingw32.SDL2
          libmodplug = pkgs.stdenv.mkDerivation {
            pname = "libmodplug";
            version = "1.22.2";
            src = libmodplug_src;
            nativeBuildInputs = [
            ];
          };

          libogg = pkgs.stdenv.mkDerivation {
            pname = "libogg";
            version = "1.3.5";
            src = libogg_src;
            nativeBuildInputs = [
            ];
          };

          libvorbis = pkgs.stdenv.mkDerivation {
            pname = "libvorbis";
            version = "1.3.7";
            src = libvorbis_src;
            nativeBuildInputs = [
            ];
            buildInputs = [
              libogg
            ];
          };

          opus = pkgs.stdenv.mkDerivation {
            pname = "opus";
            version = "0.2";
            src = opus_src;
            enableParallelBuilding = true;
            configureFlags = [
            ];
            makeFlags = [
              "LDFLAGS=-lssp"
            ];
            nativeBuildInputs = [
              pkgs.buildPackages.pkgconfig
            ];
            buildInputs = [
            ];
          };

          opusfile = pkgs.stdenv.mkDerivation {
            pname = "opusfile";
            version = "0.12";
            src = opusfile_src;
            configureFlags = [
              "--disable-http"
            ];
            nativeBuildInputs = [
              pkgs.buildPackages.pkgconfig
            ];
            buildInputs = [
              libogg
              opus
            ];
          };

          mpg123 = pkgs.stdenv.mkDerivation {
            pname = "mpg123";
            version = "1.30.2";
            src = mpg123_src;
            nativeBuildInputs = [
            ];
          };

          wstsound = pkgs.callPackage ./wstsound.nix {
            tinycmmc = tinycmmc.packages.${pkgs.system}.default;
            libmodplug = if pkgs.targetPlatform.isWindows then libmodplug else pkgs.libmodplug;
            libogg = if pkgs.targetPlatform.isWindows then libogg else pkgs.libogg;
            libvorbis = if pkgs.targetPlatform.isWindows then libvorbis else pkgs.libvorbis;
            mpg123 = if pkgs.targetPlatform.isWindows then mpg123 else pkgs.mpg123;
            openal = if pkgs.targetPlatform.isWindows then openal-soft else pkgs.openal;
            opusfile = if pkgs.targetPlatform.isWindows then opusfile else pkgs.opusfile;
            libopus = if pkgs.targetPlatform.isWindows then opus else pkgs.libopus;
            mcfgthreads = if pkgs.targetPlatform.isWindows
                          then pkgs.windows.mcfgthreads
                          else null;
          };
        };
      }
    );
}
