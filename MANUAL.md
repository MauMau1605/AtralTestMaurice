# SREC over IPC

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Deux exécutables sont produits : `sender` et `receiver`.

## Lancer

Dans un premier terminal (le sender attend une connexion) :

```bash
./sender <fichier_source> [cipher_type]
```

- `<fichier_source>` : fichier binaire au format `[adresse 16 bits][payload 24 bits] × N` (voir `sample_message.bin` fourni)
- `cipher_type` (optionnel, défaut `0`) :
  - `0` = aucun chiffrement
  - `1` = XOR avec clé pré-partagée
  - `2` = MOD 

Dans un second terminal :

```bash
./receiver
```

## Exemple

```bash
./sender sample_message.bin &
./receiver
```
