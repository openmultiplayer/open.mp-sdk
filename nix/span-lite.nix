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
        span-lite = stdenv.mkDerivation rec {
          pname = "span-lite";
          version = "0.10.3";

          src = fetchFromGitHub {
            owner = "martinmoene";
            repo = "span-lite";
            rev = "v${version}";
            hash = "sha256-WfoyyPLBqXSGGATWN/wny6P++3aCmQMOMLCARhB+R3c=";
          };

          nativeBuildInputs = [
            cmake
          ];

          cmakeFlags = [
            "-DSPAN_LITE_OPT_BUILD_TESTS=OFF"
          ];
        };
      };
    };
}
