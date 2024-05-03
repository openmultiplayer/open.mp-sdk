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
        string-view-lite = stdenv.mkDerivation rec {
          pname = "string-view-lite";
          version = "1.6.0";

          src = fetchFromGitHub {
            owner = "martinmoene";
            repo = "string-view-lite";
            rev = "v${version}";
            hash = "sha256-gGWhV7dDBiu8x98mf8pgPdpoarAl8Pd3nqJX3AZIHpg=";
          };

          nativeBuildInputs = with pkgs; [
            cmake
          ];
        };
      };
    };
}
