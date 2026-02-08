# N64-USB Gamepad Adapter

Adaptateur Raspberry Pi Pico convertissant une manette Nintendo 64 en gamepad USB HID standard compatible Windows/Linux/macOS.

## Fonctionnalités

- Compatible avec toutes les manettes N64 officielles et clones
- Reconnaissance automatique comme gamepad USB standard (pas de drivers)
- Polling rate 125Hz (8ms de latence)
- Hot-plug de la manette N64 supporté
- LED de statut intégrée
- Outil de test web inclus

## Matériel requis

- Raspberry Pi Pico (RP2040)
- Connecteur manette N64 (ou câble extension coupé)
- Résistance 1KΩ (pull-up)
- Fils de connexion

## Câblage

```
N64 Connector (vue de face)
  _________
 /  1 2 3  \
|           |
 \_________/

Pin 1 (GND)  → Pico GND
Pin 2 (Data) → Pico GP18 + résistance 1KΩ vers 3.3V
Pin 3 (3.3V) → Pico 3V3(OUT)
```

**Important** : La résistance pull-up de 1KΩ entre GP18 et 3.3V est obligatoire.

## Compilation

### Prérequis

- [Pico SDK](https://github.com/raspberrypi/pico-sdk) (v2.0+)
- [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- CMake 3.13+
- Ninja ou Make

### Build

```bash
# Linux/macOS
export PICO_SDK_PATH=/chemin/vers/pico-sdk

# Windows (Git Bash)
export PICO_SDK_PATH="C:/pico-sdk"

# Compilation
cmake -B build -G Ninja
cmake --build build

# Résultat: build/src/pico_n64.uf2
```

## Installation

1. Maintenir le bouton **BOOTSEL** sur le Pico
2. Brancher le câble USB
3. Copier `pico_n64.uf2` sur le disque **RPI-RP2**
4. Le Pico redémarre automatiquement

## LED de statut

| État LED | Signification |
|----------|---------------|
| Éteinte | USB non connecté au PC |
| Clignotement lent (500ms) | Attente manette N64 |
| Clignotement rapide (100ms) | Erreur PIO |
| Allumée fixe | Fonctionnement normal |

## Mapping des boutons

| Bouton N64 | USB Button | Index |
|------------|------------|-------|
| A | Button 1 | 0 |
| B | Button 2 | 1 |
| Z | Button 3 | 2 |
| C-Up | Button 4 | 3 |
| L | Button 5 | 4 |
| R | Button 6 | 5 |
| C-Down | Button 7 | 6 |
| C-Left | Button 8 | 7 |
| C-Right | Button 9 | 8 |
| Start | Button 10 | 9 |
| D-Pad | Hat Switch | 0-7 |
| Stick | Axes X/Y | 0-255 |

## Test

Ouvrir `tools/gamepad_tester.html` dans un navigateur (Chrome, Firefox, Edge) pour tester tous les boutons et axes en temps réel.

## Architecture du projet

```
pico-n64/
├── include/
│   ├── tusb_config.h        # Configuration TinyUSB
│   ├── n64_protocol.h       # Constantes protocole N64
│   ├── n64_controller.h     # Interface contrôleur N64
│   ├── usb_descriptors.h    # Descripteurs USB HID
│   └── usb_gamepad.h        # Interface gamepad USB
├── src/
│   ├── main.c               # Point d'entrée, boucle principale
│   ├── n64/
│   │   ├── n64_controller.pio   # Programme PIO (protocole N64)
│   │   └── n64_controller.c     # Communication manette
│   └── usb/
│       ├── usb_descriptors.c    # Descripteurs USB
│       └── usb_gamepad.c        # Conversion N64 → USB HID
├── tools/
│   └── gamepad_tester.html  # Outil de test web
├── CMakeLists.txt
└── README.md
```

## Configuration

Les paramètres modifiables sont dans les fichiers suivants :

| Paramètre | Fichier | Valeur par défaut |
|-----------|---------|-------------------|
| Pin données N64 | `include/n64_controller.h` | GP18 |
| Polling rate | `src/main.c` | 8ms (125Hz) |
| USB VID | `include/usb_descriptors.h` | 0x1209 |
| USB PID | `include/usb_descriptors.h` | 0x6E34 |

## Protocole N64

Le protocole N64 utilise une ligne de données unique (open-drain) :
- Vitesse : 1 Mbps
- Timing : 4µs par bit (1µs low + 3µs high pour '1', 3µs low + 1µs high pour '0')
- Commandes : 0x00 (info), 0x01 (status)

L'implémentation utilise le PIO du RP2040 pour un timing précis.

## Dépannage

### La LED clignote lentement
- Vérifier que la manette N64 est bien branchée
- Vérifier le câblage (GND, Data, 3.3V)
- Vérifier la résistance pull-up de 1KΩ

### La LED clignote rapidement
- Erreur d'initialisation PIO
- Reflasher le firmware

### Le stick analogique dérive
- Normal sur les vieilles manettes N64 (usure mécanique)
- Envisager un remplacement du module stick

### D-Pad ne fonctionne pas dans certains jeux
- Certains jeux ne supportent que les axes ou les boutons
- Utiliser un remapper comme JoyToKey

## Licence

Ce projet utilise le VID/PID de [pid.codes](https://pid.codes/) (0x1209/0x6E34).

## Liens utiles

- [Pico SDK Documentation](https://raspberrypi.github.io/pico-sdk-doxygen/)
- [TinyUSB](https://docs.tinyusb.org/)
- [N64 Controller Protocol](http://n64devkit.square7.ch/pro-man/pro26/26-01.htm)
- [Web Gamepad API](https://developer.mozilla.org/en-US/docs/Web/API/Gamepad_API)
