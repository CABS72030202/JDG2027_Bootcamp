# Machine JDG2026

Ce répertoire contient toute la documentation et tous les fichiers utilisés pour la solution robotique de la machine des 35e Jeux de Génie pour la délégation de Trois-Rivières.

## Contribution

### Premiers pas

1. Clonez ce dépôt
2. Consultez les configurations dans `doc/` pour comprendre l'utilisation des microcontrôleurs et microprocesseurs
3. Lisez ce `README` pour comprendre l'architecture du projet afin de maintenir une structure propre
4. Utilisez **PlatformIO** pour compiler les programmes ESP32 et Raspberry Pi
5. Consultez `doc/use_platformio.md` pour le guide complet d'utilisation

### Bonnes pratiques

- Respectez la structure de code existante
- Documentez vos modifications
- Testez vos changements avant commit
- Utilisez des messages de commit clairs et concis

### Workflow recommandé

1. Créer une branche pour votre fonctionnalité
2. Développer et tester localement
3. Soumettre une pull request pour revue

## Structure du répertoire
```bash
.
├── doc/
│ ├── use_platformio.md     # Guide PlatformIO complet
│ ├── add_linux_service.md  # Configuration services Linux
│ ├── logitech_f710.md      # Documentation manette F710
│ └── ref/                  # Datasheets et références
├── lib/
│ ├── esp-now/              # Communication ESP-NOW
│ ├── f710/                 # Contrôle manette Logitech F710
│ ├── led/                  # Contrôle LEDs
│ ├── motor/                # Contrôle moteurs
│ ├── servo/                # Contrôle servomoteurs
│ ├── spi/                  # Communication SPI
│ └── pins.h                # Définitions des broches
├── src/
│ ├── controller1.c         # Gestion de la manette 1
│ ├── esp_dam.cpp           # Robot barrage
│ ├── esp_elevator.cpp      # Robot élévateur
│ ├── esp_nuclear.cpp       # Robot nucléaire
│ ├── esp_solar.cpp         # Robot solaire
│ ├── esp_tx.cpp            # Gestion de la transmission ESP-NOW
│ └── esp_windmill.cpp      # Robot éolienne
├── ssh/
│ ├── connect.sh            # Connexion SSH aux Raspberry Pi
│ └── send.sh               # Déploiement automatique
├── test/                   # Tests Arduino IDE
└── platformio.ini          # Configuration PlatformIO
```

## Compilation et Déploiement

### Pour les ESP32 (Microcontrôleurs)
```bash
# Compiler un programme spécifique
pio run -e esp_dam
pio run -e esp_elevator
pio run -e esp_nuclear
pio run -e esp_solar
pio run -e esp_tx
pio run -e esp_windmill

# Compiler et flasher
pio run -e esp_dam -t upload

# Monitorer la sortie série
pio device monitor
```

### Pour les Raspberry Pi (Microprocesseurs)
```bash
# Compiler le programme principal
pio run -e controller1

# Déployer automatiquement sur Raspberry Pi
./ssh/send.sh
```

## Commandes utiles
```bash
# Lister tous les environnements
pio run --list-environments

# Nettoyer les builds
pio run -t clean

# Installer les dépendances
pio pkg install
```

## Architecture du Projet
### Microcontrôleurs ESP32
- ESP32-WROOM-UE sur DevKit C
- Programmes en C++ (.cpp)
- Communication sans fil via ESP-NOW
- Contrôle des robots

### Microprocesseurs Raspberry Pi
- Programme principal en C (.c)
- Communication avec manette Logitech F710
- Coordination des différents ESP32
- Interface de contrôle utilisateur

### Bibliothèques Partagées
Toutes les bibliothèques dans `lib/` sont réutilisables par tous les contrôleurs :
- `esp-now/` : Communication sans fil
- `f710/` : Contrôle manette jeu
- `motor/, servo/, led/` : Contrôle des robots
- `spi/` : Communication série

## Contact
Pour toute question ou demande relative au projet, veuillez contacter Sébastien Cabana de la délégation de Trois-Rivières aux Jeux de Génie 2026.

## Documentation Additionnelle
- `doc/use_platformio.md` - Guide détaillé PlatformIO
- `doc/add_linux_service.md` - Configuration services système
- `doc/logitech_f710.md` - Utilisation de la manette
- `doc/ref/` - Datasheets techniques et références matérielles