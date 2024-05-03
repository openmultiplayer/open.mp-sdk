{ self, ... }:
{
  flake.overlays.default = _final: prev:
    let
      inherit (prev.stdenv.hostPlatform) system;
    in
    self.packages.${system};

  perSystem =
    { pkgs
    , self'
    , lib
    , ...
    }:
    {
      packages = with pkgs; {
        robin-hood-hashing = stdenv.mkDerivation rec {
          pname = "robin-hood-hashing";
          version = "3.11.5";

          src = fetchFromGitHub {
            owner = "martinus";
            repo = "robin-hood-hashing";
            rev = "${version}";
            hash = "sha256-J4u9Q6cXF0SLHbomP42AAn5LSKBYeVgTooOhqxOIpuM=";
          };

          nativeBuildInputs = with pkgs; [
            cmake
          ];

          doCheck = false;

          buildPhase = '''';
          cmakeFlags = [
            "-DRH_STANDALONE_PROJECT=False"
          ];
        };
      };
    };
}

