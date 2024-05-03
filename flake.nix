{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
  };

  outputs =
    inputs@{ flake-parts
    , ...
    }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        ./nix/span-lite.nix
        ./nix/string-view-lite.nix
        ./nix/robin-hood-hashing.nix
      ];

      systems = [
        "x86_64-linux"
      ];

      perSystem = { pkgs, lib, system, self', ... }: {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "open-mp-sdk";
          version = "1.2.0";

          src = lib.sourceByRegex ./. [
            "^cmake.*"
            "^include.*"
            "CMakeLists.txt"
          ];

          nativeBuildInputs = with pkgs; [
            cmake
          ];
          propagatedBuildInputs = with pkgs; [
            glm
            span-lite
            self'.packages.string-view-lite
            self'.packages.robin-hood-hashing
          ];
        };
      };
    };
}
