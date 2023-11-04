# Nokia Innovative Projects

## Extensible system monitoring framework

Projekt zespołowy z udziałem

- Weronika Jakimowicz
- Julia Kiczka
- Łukasz Magnuszewski
- Katarzyna Stasińska


## Building 


### Building from source

Wymagania:
+ git
+ gcc
+ make
+ cmake(3.25+)

```bash
   git clone https://github.com/ronia-jakim/nokia_extensible_system_monitoring_framework
   cd nokia_extensible_system_monitoring_framework 
   chmod +x build.sh
   ./build.sh
```


[comment]: # ( stara wersja tworzenia tego projektu
```bash
   git clone https://github.com/ronia-jakim/nokia_extensible_system_monitoring_framework
   cd nokia_extensible_system_monitoring_framework 
   cmake . 
   make all
```
)

Wynikiem buildowania powinny być 2 pliki binarne: 'node' oraz 'monitor'


### Building with nix 

Wymagania:
    nix(z włączonymi flakes https://nixos.wiki/wiki/Flakes)

Build:
```bash
    git clone https://github.com/ronia-jakim/nokia_extensible_system_monitoring_framework
    nix build .#node
    nix build .#monitor
```
Run:
```bash 
    nix run .#node //lub monitor zamiast node
```
