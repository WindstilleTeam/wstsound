{
  description = "Windstille Sound Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    libmodplug-win32.url = "github:grumnix/libmodplug-win32";
    libmodplug-win32.inputs.nixpkgs.follows = "nixpkgs";
    libmodplug-win32.inputs.tinycmmc.follows = "tinycmmc";

    libogg-win32.url = "github:grumnix/libogg-win32";
    libogg-win32.inputs.nixpkgs.follows = "nixpkgs";
    libogg-win32.inputs.tinycmmc.follows = "tinycmmc";

    libvorbis-win32.url = "github:grumnix/libvorbis-win32";
    libvorbis-win32.inputs.nixpkgs.follows = "nixpkgs";
    libvorbis-win32.inputs.tinycmmc.follows = "tinycmmc";
    libvorbis-win32.inputs.libogg.follows = "libogg-win32";

    mpg123-win32.url = "github:grumnix/mpg123-win32";
    mpg123-win32.inputs.nixpkgs.follows = "nixpkgs";
    mpg123-win32.inputs.tinycmmc.follows = "tinycmmc";

    openal-soft-win32.url = "github:grumnix/openal-soft-win32";
    openal-soft-win32.inputs.nixpkgs.follows = "nixpkgs";
    openal-soft-win32.inputs.tinycmmc.follows = "tinycmmc";

    opus-win32.url = "github:grumnix/opus-win32";
    opus-win32.inputs.nixpkgs.follows = "nixpkgs";
    opus-win32.inputs.tinycmmc.follows = "tinycmmc";

    opusfile-win32.url = "github:grumnix/opusfile-win32";
    opusfile-win32.inputs.nixpkgs.follows = "nixpkgs";
    opusfile-win32.inputs.tinycmmc.follows = "tinycmmc";
    opusfile-win32.inputs.libogg.follows = "libogg-win32";
    opusfile-win32.inputs.opus.follows = "opus-win32";
  };

  outputs = { self, nixpkgs, flake-utils,
              tinycmmc, libmodplug-win32, libogg-win32, libvorbis-win32,
              mpg123-win32, openal-soft-win32, opusfile-win32, opus-win32 }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = wstsound;

          wstsound = pkgs.callPackage ./wstsound.nix {
            tinycmmc = tinycmmc.packages.${pkgs.system}.default;

            libmodplug = if pkgs.targetPlatform.isWindows
                         then libmodplug-win32.packages.${pkgs.system}.default
                         else pkgs.libmodplug;

            libogg = if pkgs.targetPlatform.isWindows
                     then libogg-win32.packages.${pkgs.system}.default
                     else pkgs.libogg;

            libvorbis = if pkgs.targetPlatform.isWindows
                        then libvorbis-win32.packages.${pkgs.system}.default
                        else pkgs.libvorbis;

            mpg123 = if pkgs.targetPlatform.isWindows
                     then mpg123-win32.packages.${pkgs.system}.default
                     else pkgs.mpg123;

            openal = if pkgs.targetPlatform.isWindows
                     then openal-soft-win32.packages.${pkgs.system}.default
                     else pkgs.openal;

            opusfile = if pkgs.targetPlatform.isWindows
                       then opusfile-win32.packages.${pkgs.system}.default
                       else pkgs.opusfile;

            libopus = if pkgs.targetPlatform.isWindows
                      then opus-win32.packages.${pkgs.system}.default
                      else pkgs.libopus;

            mcfgthreads = if pkgs.targetPlatform.isWindows
                          then pkgs.windows.mcfgthreads
                          else null;
          };
        };
      }
    );
}
