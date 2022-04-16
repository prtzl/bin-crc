{
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
        jlink-pack.url  = "github:prtzl/jlink-nix";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = inputs: with inputs; flake-utils.lib.eachDefaultSystem (system:
        let
            pkgs = nixpkgs.legacyPackages.${system};
            stdenv = pkgs.stdenv;
            jlink = jlink-pack.defaultPackage.${system};
            bin-crc = pkgs.callPackage ./bin-crc {};
            firmware = pkgs.callPackage ./firmware {};
            fixed-bin = stdenv.mkDerivation {
                name = "fixed-bin";
                nativeBuildInputs = [ bin-crc ];
                src = firmware;
                buildPhase = ''
                    bin-crc ./bin/${firmware.name}.bin ${firmware.name}-crc.bin
                '';
                installPhase = ''
                    mkdir -p $out
                    cp ${firmware.name}-crc.bin $out
                '';
            };
            flash-script = pkgs.writeTextFile {
                name = "flash-script";
                text = ''
                    device ${firmware.device}
                    si 1
                    speed 4000
                    loadfile ${fixed-bin}/${firmware.name}-crc.bin,0x08000000
                    r
                    g
                    qc
                '';
            };
            flash = pkgs.writeShellApplication {
                name = "flash";
                text = "JLinkExe -commanderscript ${flash-script}";
                runtimeInputs = [ jlink ];
            };
        in {
            inherit bin-crc firmware fixed-bin flash;
            defaultPackage = fixed-bin;

            devShell = pkgs.mkShell {
                nativeBuildInputs = (bin-crc.nativeBuildInputs or []) ++ (firmware.nativeBuildInputs or []) ++ [ pkgs.nix pkgs.clang-tools jlink ];
                LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [ pkgs.llvmPackages_11.llvm ];
            };
        });
}
