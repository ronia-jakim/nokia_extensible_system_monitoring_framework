# Nokia Innovative Projects

## Extensible system monitoring framework

Projekt zespołowy z udziałem

- Weronika Jakimowicz
- Julia Kiczka
- Łukasz Magnuszewski
- Katarzyna Stasińska

## TODO list

- [ ] przesyłanie z tagami (GUIDami)
- [x] dwa różne typy danych
- [ ] parser
- [x] monitorowanie jednego dowolnie wybranego  realnego parametru w systemie (odczytywanie przez kolektor i wysyłanie do centrali)
- [x] pierwsza wersja centrali wielozadaniowej (obsługującej wiele kolektorów jednocześnie)
- [ ] rozszerzenie opisu danych o monitorowanie zasobów dynamicznych (takich jak procesy)
- [x] pierwsza wersja formatu danych - na początku jako dwie biblioteki dla węzła i dla głównego serwera
- [x] zalążek systemu budowania
- [x] ~~PoC #include<sys/resource.h>~~
- [x] push czy pull - zostaje push?
- [x] pierwsza wersja serwera i klienta (sukces w komunikacji)


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

<!---
```bash
   git clone https://github.com/ronia-jakim/nokia_extensible_system_monitoring_framework
   cd nokia_extensible_system_monitoring_framework 
   cmake . 
   make all
```
--->

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
