{
  description = "Development and Build environment for nokia extensible system monitoring framework";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
  };

  outputs = { self, nixpkgs }:
    let
      # Systems supported
      allSystems = [
        "x86_64-linux" # 64-bit Intel/AMD Linux
        "aarch64-linux" # 64-bit ARM Linux
        "x86_64-darwin" # 64-bit Intel macOS
        "aarch64-darwin" # 64-bit ARM macOS
      ];

      # Helper to provide system-specific attributes
      forAllSystems = f: nixpkgs.lib.genAttrs allSystems (system: f {
        pkgs = import nixpkgs { inherit system; };
      });
    in
    {
      # Development environment output
      devShells = forAllSystems ({ pkgs }: {
        default = pkgs.mkShell {
          # The Nix packages provided in the environment
          packages = with pkgs; [
            # build tools
            cmake # Build system generator
            gcc # The GNU Compiler Collection
            gnumake # A tool to control the generation of non-source files from sources
            

            # writing tools
            ccls # A c/c++ language server powered by clang
            cmake-language-server # CMake LSP Implementation
          ];
        };
      });
    };
}
