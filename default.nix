{ stdenv
, cmake
, SDL2
, version
}:

stdenv.mkDerivation(finalAttrs: {
  inherit version;

  pname = "plush";

  src = ./.;

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    SDL2
  ];
})
