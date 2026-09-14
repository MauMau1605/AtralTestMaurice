# Manuel Technique & Guide d'Utilisation - SREC over IPC

Ce document constitue le manuel de référence du projet **SREC over IPC**. Il détaille l'architecture logicielle, les spécifications fonctionnelles et protocolaires, les procédures de compilation et d'exécution, ainsi que les analyses de sûreté et de portabilité sur cible embarquée.

---

## Sommaire

1. [Présentation du Système](#1-présentation-du-système)
2. [Compilation & Exécution](#2-compilation--exécution)
3. [Format de l'Image Mémoire](#3-format-de-limage-mémoire)
4. [Protocole de Communication & Format SREC](#4-protocole-de-communication--format-srec)
5. [Mécanismes de Chiffrement (Obfuscation)](#5-mécanismes-de-chiffrement-obfuscation)
6. [Architecture Logicielle & Scalabilité](#6-architecture-logicielle--scalabilité)
7. [Analyse de Maturité Industrielle & Portage Embarqué](#7-analyse-de-maturité-industrielle--portage-embarqué)
8. [Matrice de Conformité aux Instructions](#8-matrice-de-conformité-aux-instructions)

---

## 1. Présentation du Système

Dans le cadre d'un système connecté d'alarme/détection, ce projet constitue la preuve de concept (PoC) sous Linux d'un transfert sécurisé d'une image mémoire depuis un **détecteur** (`sender`) vers une **centrale** (`receiver`).

```
+-------------------+                    +--------------------+
|      SENDER       |   Socket UNIX      |      RECEIVER      |
|    (Détecteur)    | -----------------> |     (Centrale)     |
|                   |  /tmp/srec_ipc.sock|                    |
| - Lecture image   |                    | - Décodage SREC    |
| - Chiffrement     |                    | - Déchiffrement    |
| - Encodage SREC   |                    | - Écriture fichier |
| - Émission IPC    |                    | - Affichage ASCII  |
+-------------------+                    +--------------------+
```

### Rôle des Composants

- **`sender` (Détecteur)** :
  - Ouvre et lit le fichier binaire d'entrée structuré.
  - Applique l'algorithme d'obfuscation configuré (NONE, XOR, MOD).
  - Formate les trames SREC (en-tête S0 portant l'algorithme, trames de données S1, trame de fin S9).
  - Émet les trames sur le canal IPC via socket UNIX.
- **`receiver` (Centrale)** :
  - Se connecte au canal IPC.
  - Réceptionne et décode les enregistrements ligne par ligne.
  - Identifie le type de chiffrement via la trame S0.
  - Déchiffre les données des trames S1 à l'aide de la clé pré-partagée.
  - Reconstruit l'image mémoire exacte dans `received_image.bin`.
  - Affiche le message ASCII extrait sur la sortie standard.

---

## 2. Compilation & Exécution

### Prérequis

- Compilateur C standard C11 (`gcc` ou `clang`)
- `cmake` (version 3.10 ou supérieure)
- Environnement POSIX / Linux

### Procédure de Compilation

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

Deux exécutables sont générés :
- `build/sender`
- `build/receiver`

### Utilisation

#### 1. Lancement du Sender

Le `sender` crée le serveur IPC et se met en attente de connexion.

```bash
./sender <fichier_source> [cipher_type]
```

- `<fichier_source>` *(obligatoire)* : chemin vers le fichier d'image mémoire binaire (ex: `sample_message.bin`).
- `cipher_type` *(optionnel, défaut : 0)* :
  - `0` : `CIPHER_NONE` (en clair)
  - `1` : `CIPHER_XOR` (XOR avec clé pré-partagée)
  - `2` : `CIPHER_MOD` (Addition modulo 256 avec clé pré-partagée)

#### 2. Lancement du Receiver

Dans un second terminal (ou en tâche de fond) :

```bash
./receiver
```

Le `receiver` se connecte au `sender`, traite le flux, écrit `received_image.bin` et affiche le texte décodé sur `stdout`.

### Exemple de Test Complet

```bash
# Test avec chiffrement XOR (cipher_type = 1)
./sender ../sample_message.bin 1 &
./receiver

# Vérification de l'intégrité binaire octet par octet
cmp ../sample_message.bin received_image.bin && echo "Test OK : Fichiers strictement identiques"
```

### Automatisation des Tests & Tâches VS Code

Un script de test automatisé et des tâches VS Code préconfigurées sont disponibles :

- **Via script Bash** :
  ```bash
  ./scripts/run_test.sh all   # Exécute et valide les 3 modes (0: No cipher, 1: XOR, 2: MOD)
  ./scripts/run_test.sh 0     # Teste uniquement sans chiffrement
  ./scripts/run_test.sh 1     # Teste uniquement avec chiffrement XOR
  ./scripts/run_test.sh 2     # Teste uniquement avec chiffrement MOD
  ```

- **Via VS Code / IDE (Ctrl+Shift+B ou Menu `Terminal > Run Task`)** :
  - **Build Project** : Compilation CMake (`Ctrl+Shift+B` par défaut)
  - **Test: All (No Cipher, Cipher 1, Cipher 2)** : Lance automatiquement le build puis les 3 tests avec comparaison binaire
  - **Test: Sender No Cipher (0)** : Teste le mode sans chiffrement
  - **Test: Sender Cipher 1 (XOR)** : Teste le mode XOR
  - **Test: Sender Cipher 2 (MOD)** : Teste le mode MOD
  - **Clean Project** : Nettoie le répertoire de build et les fichiers générés


---

## 3. Format de l'Image Mémoire

Le fichier binaire source n'est pas un flux plat, mais une collection d'enregistrements mémoires contigus de taille fixe de **5 octets** :

```
+-------------------------+-------------------------+
|     Adresse (2 octets)  |     Donnée (3 octets)   |
|       [Big-Endian]      |         ASCII           |
+-------------------------+-------------------------+
  octet 0       octet 1     octet 2   octet 3  octet 4
```

- **Adresse (16 bits)** : Poids fort en premier (MSB first, Big-Endian). Indique l'adresse mémoire du premier octet de la donnée.
- **Données (24 bits)** : 3 octets représentant des caractères ASCII imprimables.
- **Reconstruction** : Les enregistrements assemblés dans l'ordre reconstituent l'image mémoire du microcontrôleur émetteur.

---

## 4. Protocole de Communication & Format SREC

Les échanges reposent sur le format standard **Motorola S-record (SREC)**, constitué de lignes ASCII terminées par un caractère `LF` (`\n`).

### Structure d'une Ligne SREC

```
S | T | CC | AAAA | DD ... DD | KK
```

| Champ | Longueur | Description |
|---|---|---|
| `S` | 1 car. | Marqueur de début de trame |
| `T` | 1 car. | Type d'enregistrement (`0`, `1`, `9`) |
| `CC` | 2 hex | Compteur d'octets (Nombre d'octets du payload : Adresse + Données + Checksum) |
| `AAAA` | 4 hex | Adresse 16 bits (Big-Endian) |
| `DD...` | 2×N hex | Données utiles (0 à 32 octets) |
| `KK` | 2 hex | Checksum SREC (complément à un de la somme des octets CC + AAAA + DD) |

### Types d'Enregistrements Utilisés

1. **S0 (En-tête et Configuration)** :
   - `Type` : `'0'`
   - `Adresse` : `0x0000`, `0x0001` ou `0x0002` (transportant le code `CIPHER_TYPE`).
   - `Données` : Aucune (`data_len = 0`).
   - Exemple : `S0030001FB` (chiffrement XOR configuré, checksum 0xFB).

2. **S1 (Données Utiles)** :
   - `Type` : `'1'`
   - `Adresse` : Adresse mémoire 16 bits de l'enregistrement source.
   - `Données` : 3 octets chiffrés (obfusqués) correspondant au payload.
   - Exemple : `S1068000E7CCC006` (`CC=06`, adresse `0x8000`, 3 octets chiffrés, checksum `0x06`).

3. **S9 (Fin de Transmission)** :
   - `Type` : `'9'`
   - `Adresse` : `0x0000`
   - `Données` : Aucune (`data_len = 0`).
   - Exemple : `S9030000FC`.

### Règle du Checksum

Le checksum SREC est calculé sur les octets **tels qu'émis sur le canal** (c'est-à-dire après chiffrement des données) :
$$\text{Checksum} = \sim \left( \text{CC} + \sum \text{Octets d'adresse} + \sum \text{Octets de données chiffrées} \right) \ \& \ 0\text{xFF}$$

---

## 5. Mécanismes de Chiffrement (Obfuscation)

Une clé pré-partagée (`PRESHARED_KEY = 0xA5`) est connue statiquement par les deux entités et n'est jamais transmise sur le réseau.

| `CIPHER_TYPE` | Nom | Émetteur (`sender`) | Récepteur (`receiver`) |
|---|---|---|---|
| `0x0000` | `CIPHER_NONE` | $D_{\text{tx}}[i] = D_{\text{src}}[i]$ | $D_{\text{rx}}[i] = D_{\text{tx}}[i]$ |
| `0x0001` | `CIPHER_XOR` | $D_{\text{tx}}[i] = D_{\text{src}}[i] \oplus \text{Key}$ | $D_{\text{rx}}[i] = D_{\text{tx}}[i] \oplus \text{Key}$ |
| `0x0002` | `CIPHER_MOD` | $D_{\text{tx}}[i] = (D_{\text{src}}[i] + \text{Key}) \bmod 256$ | $D_{\text{rx}}[i] = (D_{\text{tx}}[i] - \text{Key}) \bmod 256$ |

> **Note de Sécurité** : Ces transformations constituent une obfuscation légère destinée à masquer les données contre une lecture passive directe. Dans un système de sécurité normé, ce mécanisme sera remplacé par un chiffrement authentifié standard (ex: AES-128-CCM ou AES-128-GCM).

---

## 6. Architecture Logicielle

### Arborescence du Code

```
├── CMakeLists.txt         # Configuration de build CMake
├── INSTRUCTIONS.md        # Cahier des charges initial
├── MANUAL.md              # Manuel technique et documentation (ce document)
├── sample_message.bin     # Image mémoire de test
├── sender.c               # Application émettrice (détecteur)
├── receiver.c             # Application réceptrice (centrale)
└── common/
    ├── ipc.h / ipc.c      # Abstraction de la couche IPC (sockets UNIX)
    ├── protocol.h         # Constantes partagées (clés, ciphers, tailles)
    └── srec.h / srec.c    # Encodeur / décodeur de trames SREC
```

### Abstraction de la Couche IPC

Les applications `sender.c` et `receiver.c` ne manipulent aucun détail d'implémentation de la socket UNIX. Elles s'interfacent uniquement via l'API définie dans `common/ipc.h` :
- `ipc_server_open()` / `ipc_server_accept()`
- `ipc_client_connect()`
- `ipc_close()`