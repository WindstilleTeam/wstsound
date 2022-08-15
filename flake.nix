{
  description = "Windstille Sound Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    nix-cross.url = "github:grumbel/nix-cross";
    nix-cross.inputs.nixpkgs.follows = "nixpkgs";
    nix-cross.inputs.flake-utils.follows = "flake-utils";
    nix-cross.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc, nix-cross }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = wstsound;

          wstsound = pkgs.callPackage ./wstsound.nix {
            tinycmmc = tinycmmc.packages.${pkgs.system}.default;

            libmodplug = if pkgs.targetPlatform.isWindows
                         then nix-cross.packages.${pkgs.system}.libmodplug
                         else pkgs.libmodplug;

            libogg = if pkgs.targetPlatform.isWindows
                     then nix-cross.packages.${pkgs.system}.libogg
                     else pkgs.libogg;

            libvorbis = if pkgs.targetPlatform.isWindows
                        then nix-cross.packages.${pkgs.system}.libvorbis
                        else pkgs.libvorbis;

            mpg123 = if pkgs.targetPlatform.isWindows
                     then nix-cross.packages.${pkgs.system}.mpg123
                     else pkgs.mpg123;

            openal = if pkgs.targetPlatform.isWindows
                     then nix-cross.packages.${pkgs.system}.openal-soft
                     else pkgs.openal;

            opusfile = if pkgs.targetPlatform.isWindows
                       then nix-cross.packages.${pkgs.system}.opusfile
                       else pkgs.opusfile;

            libopus = if pkgs.targetPlatform.isWindows
                      then nix-cross.packages.${pkgs.system}.opus
                      else pkgs.libopus;

            mcfgthreads = if pkgs.targetPlatform.isWindows
                          then pkgs.windows.mcfgthreads
                          else null;
          };
        };
      }
    );
}
