# Contexte

Une entreprise développant des objets connectés souhaite mettre en place une fonction de transfert d'une image mémoire depuis un détecteur vers une centrale. Le transfert utilise le format texte Motorola S-record. Les données peuvent être transmises en clair ou chiffrées selon une configuration connue du détecteur.

L'objectif de ce test est de réaliser sous Linux une preuve de concept composée de deux programmes : un sender et un receiver.
Vous récupérez une base de code non fonctionnelle, buguée et partiellement implémentée. Seule la couche IPC a été validée. Ce qui est attendu est un code C fonctionnel, sans librairie externe, scalable, versionné, portable sur cible embarquée avec un niveau de maturité industriel pouvant être présenté en revue de code à une équipe développant des produits embarqués dans un environnement normatif de sécurité fort. 

## Objectif fonctionnel

**Le sender doit :**
- Lire un fichier binaire représentant l'image mémoire
- Appliquer le chiffrement configuré aux données
- Encoder les données au format SREC
- Transmettre les enregistrements par le canal IPC défini

**Le receiver doit :**
- Recevoir les enregistrements SREC
- Vérifier leur syntaxe, leur longueur et leur checksum
- Déterminer le chiffrement utilisé à partir de l'enregistrement S0
- Déchiffrer les données
- Reconstruire l'image mémoire dans un fichier de sortie, strictement identique octet par octet au fichier d'entrée du sender
- Afficher sur la sortie standard le message ASCII contenu dans l'image

## Livrable attendu

Le candidat doit nous transmettre son code source en langage C, avec les caractéristiques suivantes :
- **Versionné** l'outil de gestion de version est laissé au choix du candidat.
- **Compilation via CMake**.
- **Maturité industrielle** : le niveau de qualité correspondant au contexte décrit, avec les outils de son choix.
- **Architecture scalable** : l'ajout futur d'un algorithme de chiffrement ne doit pas nécessiter de modifier le code applicatif existant.
- **Documentation** : tout élément permettant de comprendre le fonctionnement du programme

La durée indicative du test est d'au moins 2h ; le candidat reste libre d'aller au-delà s'il le souhaite.
À l'issue du test, une restitution sera organisée sous forme de revue de code par ses pairs, incluant des questions sur le portage du code vers une cible embarquée.

# Format de l'image mémoire

Le fichier d'entrée n'est pas un flux d'octets bruts : il est structuré en une suite d'enregistrements de taille fixe, chacun composé d'une adresse suivie de sa donnée associée.

Chaque enregistrement occupe exactement 5 octets :
`
[ adresse (2 octets) ][ donnée (3 octets) ]
`

- **Adresse** : 2 octets, poids fort en premier (big-endian). Représente l'adresse 16 bits du premier octet de la donnée qui suit.
- **Donnée** : 3 octets (24 bits), qui sont des caractères ASCII. Mis bout à bout dans l'ordre des enregistrements, ils forment le message contenu dans la mémoire du détecteur.

**Exemple** (2 premiers enregistrements d'un fichier contenant le message "Bienvenue...") :
```bash
00 00 42 69 65 ← adresse 0x0000, données "Bie"
00 03 6E 76 65 ← adresse 0x0003, données "nve"
```

# Algorithmes de chiffrement

| CIPHER_TYPE | Nom | Mode | Transformation (sender) | Transformation inverse (receiver) |
|---|---|---|---|---|
| `0x0000` | CIPHER_NONE | Aucun | `sortie[i] = entrée[i]` | `entrée[i] = sortie[i]` |
| `0x0001` | CIPHER_XOR | XOR | `sortie[i] = entrée[i] XOR clé` | `entrée[i] = sortie[i] XOR clé` |
| `0x0002` | CIPHER_MOD | Addition modulo 256 | `sortie[i] = (entrée[i] + clé) mod 256` | `entrée[i] = (sortie[i] − clé) mod 256` |

`CIPHER_TYPE` est porté par le champ adresse de l'enregistrement S0 (16 bits), ce qui explique la largeur `0x0000`-`0x0002` du tableau ci-dessus.

## Clé de chiffrement

- **Taille** : 1 octet (0x00-0xFF).
- **Mode de fourniture** : Connue à l'avance des deux côtés
- **Portée d'application** : La même clé s'applique à tous les octets de données

**Note :** ce chiffrement est une protection légère (obfuscation), pas un mécanisme de sécurité — l'objectif est d'évaluer la manipulation de clé et de buffer, ainsi qu'une conception logicielle scalable (ajout futur d'un algorithme sans modifier le code existant), pas la résistance cryptographique.

# Format SREC

Toutes les lignes sont constituées de caractères ASCII imprimables et se terminent par un caractère de fin de ligne (`LF`). Chaque octet est représenté par deux chiffres hexadécimaux.

`S T CC AAAA DD... KK`

| Champ | Description |
|---|---|
| `S` | Lettre fixe, marque le début de chaque ligne |
| `T` | Type d'enregistrement (1 chiffre) |
| `CC` | Nombre d'octets après `CC` : adresse + données + checksum |
| `AAAA` | Adresse 16 bits, codée poids fort en premier (big-endian) |
| `DD...` | Données transportées (0 à N octets) |
| `KK` | Checksum SREC |

## Types utilisés

| Type | Rôle | Adresse |
|---|---|---|
| S0 | En-tête et indication du chiffrement | Valeur de `CIPHER_TYPE` |
| S1 | Données chiffrées | Adresse mémoire du premier octet de la donnée |
| S9 | Fin du transfert | `0x0000` (non utilisée) |

## Checksum

Le checksum est calculé sur les données chiffrées, telles qu'elles sont transportées sur le canal, jamais sur les données en clair. À la réception, ce calcul est refait et comparé à la valeur transmise. Toute altération d'un seul bit sur la ligne fait échouer cette vérification.


