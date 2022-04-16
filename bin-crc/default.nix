{ stdenv, gnumake, gcc }:

stdenv.mkDerivation rec {
  name = "bin-crc";
  src = ./.;

  nativeBuildInputs = [ gnumake gcc ];

  dontUseCmakeConfigure = true;
  dontPatch = true;
  dontFixup = true;
  dontStrip = true;
  dontPatchELF = true;

  buildPhase = ''
    make
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp ${name} $out/bin
  '';
}
