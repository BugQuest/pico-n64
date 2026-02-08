# N64-USB Dual Gamepad Adapter

Adaptateur Raspberry Pi Pico convertissant jusqu'à 2 manettes Nintendo 64 en gamepads USB HID standard compatible Windows/Linux/macOS.

## Fonctionnalités

- Support de 1 ou 2 manettes N64 simultanément
- Compatible avec toutes les manettes N64 officielles et clones
- Reconnaissance automatique comme gamepad(s) USB standard (pas de drivers)
- Détection dynamique : 0, 1 ou 2 manettes
- Polling rate 125Hz (8ms de latence)
- Hot-plug des manettes N64 supporté
- LED de statut intégrée
- LEDs externes optionnelles (1 par manette)
- Outil de test web inclus

## Matériel requis

- Raspberry Pi Pico (RP2040)
- 1 ou 2 connecteurs manette N64 (ou câbles extension coupés)
- 1 ou 2 résistances 1KΩ (pull-up)
- Fils de connexion
- (Optionnel) 1 ou 2 LEDs + résistances 220Ω-330Ω pour indicateurs externes

## Câblage

### Manette 1 (obligatoire)
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

### Manette 2 (optionnelle)
```
Pin 1 (GND)  → Pico GND (partagé)
Pin 2 (Data) → Pico GP19 + résistance 1KΩ vers 3.3V
Pin 3 (3.3V) → Pico 3V3(OUT) (partagé)
```

**Important** : Chaque manette nécessite sa propre résistance pull-up de 1KΩ.

### LEDs externes (optionnelles)
```
LED 1 (manette 1) : GP16 → résistance 220-330Ω → LED → GND
LED 2 (manette 2) : GP17 → résistance 220-330Ω → LED → GND
```

Les LEDs s'allument quand la manette correspondante est connectée. Pour désactiver, mettre le pin à 0 dans `include/n64_controller.h`.

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

### LED intégrée (GP25)

| État LED | Signification |
|----------|---------------|
| Éteinte | USB non connecté au PC |
| Clignotement lent (1s) | Aucune manette N64 connectée |
| Clignotement moyen (300ms) | 1 manette N64 connectée |
| Allumée fixe | 2 manettes N64 connectées |

### LEDs externes (optionnelles)

| LED | État | Signification |
|-----|------|---------------|
| LED 1 (GP16) | Allumée | Manette 1 connectée |
| LED 1 (GP16) | Éteinte | Manette 1 déconnectée |
| LED 2 (GP17) | Allumée | Manette 2 connectée |
| LED 2 (GP17) | Éteinte | Manette 2 déconnectée |

## Mapping des boutons

Chaque manette est exposée comme un gamepad USB séparé avec le même mapping :

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

**Note** : Avec 2 manettes, le navigateur détectera 2 gamepads séparés.

## Architecture du projet

```
pico-n64/
├── include/
│   ├── tusb_config.h        # Configuration TinyUSB
│   ├── n64_protocol.h       # Constantes protocole N64
│   ├── n64_controller.h     # Interface contrôleur N64 (dual)
│   ├── usb_descriptors.h    # Descripteurs USB HID (dual)
│   └── usb_gamepad.h        # Interface gamepad USB (dual)
├── src/
│   ├── main.c               # Point d'entrée, gestion 2 manettes
│   ├── n64/
│   │   ├── n64_controller.pio   # Programme PIO (protocole N64)
│   │   └── n64_controller.c     # Communication manette
│   └── usb/
│       ├── usb_descriptors.c    # Descripteurs USB (Report IDs)
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
| Pin manette 1 | `include/n64_controller.h` | GP18 |
| Pin manette 2 | `include/n64_controller.h` | GP19 |
| LED externe 1 | `include/n64_controller.h` | GP16 (0 = désactivée) |
| LED externe 2 | `include/n64_controller.h` | GP17 (0 = désactivée) |
| Polling rate | `src/main.c` | 8ms (125Hz) |
| USB VID | `include/usb_descriptors.h` | 0x1209 |
| USB PID | `include/usb_descriptors.h` | 0x6E34 |

## Protocole N64

Le protocole N64 utilise une ligne de données unique (open-drain) :
- Vitesse : 1 Mbps
- Timing : 4µs par bit (1µs low + 3µs high pour '1', 3µs low + 1µs high pour '0')
- Commandes : 0x00 (info), 0x01 (status)

L'implémentation utilise le PIO du RP2040 pour un timing précis. Chaque manette utilise un state machine PIO dédié.

## Dépannage

### La LED clignote lentement (aucune manette)
- Vérifier que la/les manette(s) N64 sont bien branchées
- Vérifier le câblage (GND, Data, 3.3V)
- Vérifier les résistances pull-up de 1KΩ

### La LED clignote moyennement (1 seule manette détectée)
- Normal si une seule manette est branchée
- Si 2 manettes sont branchées, vérifier le câblage de la 2ème

### Le stick analogique dérive
- Normal sur les vieilles manettes N64 (usure mécanique)
- Envisager un remplacement du module stick

### D-Pad ne fonctionne pas dans certains jeux
- Certains jeux ne supportent que les axes ou les boutons
- Utiliser un remapper comme JoyToKey

### Une seule manette apparaît dans Windows
- Débrancher et rebrancher l'USB
- Vérifier dans "Configurer les contrôleurs de jeu USB"

## Licence

Ce projet utilise le VID/PID de [pid.codes](https://pid.codes/) (0x1209/0x6E34).

## Liens utiles

- [Pico SDK Documentation](https://raspberrypi.github.io/pico-sdk-doxygen/)
- [TinyUSB](https://docs.tinyusb.org/)
- [N64 Controller Protocol](http://n64devkit.square7.ch/pro-man/pro26/26-01.htm)
- [Web Gamepad API](https://developer.mozilla.org/en-US/docs/Web/API/Gamepad_API)
