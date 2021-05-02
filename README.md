# AR (MU4IN403)  - PROJET : Implémentation du protocole P2P CHORD

<img align="right" src="/utilities/index.png">

Réalisé par :
  - **Namrata MISTRY M1 SAR**                       
  - **Aliocha AMERGÉ M1 SAR**

**©SU/LMD/MU4IN403**

## Introduction

*CHORD* est une table de hachage distribuée (DHT). 

Cela signifie que l’objectif du système est de stocker des données de manière distribuée et d’associer une clé à chacune d'elles. 
De plus, le système doit fournir des fonctions permettant de retrouver une donnée à partir de sa clé, de stocker une donnée (en construisant sa clé), de supprimer une donnée du système, etc. 
Enfin, vu le contexte des réseaux P2P qui sont par essence hautement dynamiques, le protocole doit intégrer une gestion du départ et de l’arrivée de nœuds dans le système (celle-ci peut être volontaire ou  subie). 
Pour plus de détails, se référer à : [I. Stoica, R. Morris, D. Karger, M. Kaashoek, H. Balakrishnan : Chord : A scalable peer-to-peer lookup service for internet applications, SIGCOMM 2001, pp. 149-160](https://pdos.csail.mit.edu/papers/chord:sigcomm01/chord_sigcomm.pdf "Chord : A Scalable Peer-to-peerLookupServiceforInternetApplications").

Nos implémentations au cours de ce projet seront en langage C et se baseront sur la norme MPI avec l'utilisation de la bibliothèque [Open MPI](https://www.open-mpi.org/ "open-MPI").

Nous supposerons l'envoie de messages FIFO et fiable tel que le garanti MPI.

### Constantes

Au cours de ce projet nous utiliserons souvent les constantes suivantes :

- **NB_PROC** : Le nombre de processus MPI qui seront créer, 
    - cela correspond aux N pairs, + 1 processus inititateur *P0*

- **N** : le nombre de pairs soit NB_PROC - 1

- **M** : la plage de valeurs ( allant de 0 à (2<sup>M</sup>) - 1 )
  - et également le nombre de fingers de chaque pairs

### Méthodes utilitaires

**int f(pair)** : retourne aléatoirement un identifiant CHORD unique parmis l'ensemble des pairs du système.

**int g(pair)** : retourne aléatoirement un identifiant unique pour une donnée (clés) parmis l'ensemble des données du système.

**int app(int k, int a, int b)** : vérifie si la clé k appartient à l'intervalle ]a,b].

### Indication pour l'execution des codes :

Arborescence : 
```
.
├── Exercice_1
│   ├── Makefile
│   ├── obj
│   │   └── lookup.o
│   └── src
│       ├── include
│       │   └── header.h
│       └── key_search.c
├── Exercice_2
│   ├── Makefile
│   ├── obj
│   └── src
│       ├── finger_table.c
│       └── include
│           └── header.h
├── Exercice_3
├── README.md
└── runmpicc.sh

```

Afin de compilé un fichier .c utilisant MPI :

il est nécéssaire d'utiliser `mpicc` pour générer un executable.
et de l'éxecuter avec `mpirun -np $2 --oversubscribe`  { avec $2 le nombre de paramètre }


Chaque exercice dispose de son dossier et de son Makefile.

Du fait que chaque exercice se compose d'un unique fichier `.c`, un script `runmpicc.sh` est présent.
Ce dernier prend 2 paramètre : le chemin vers le fichier source, et le nombre de processus utile.
Il compilera le fichier source, produira un executable, le lancera et le supprimera.

De plus il est nécéssaire d'ajouter `-lm -ldl` au flags pour la compilation.

Le dossier utilities contient


## Exercice 1 : Recherche d’une clé

Fichier : *Exercice_1/src/key_search.c*

### Initialisation de la DHT

Dans cet exercice, un processus simulateur initialisera la DHT CHORD de manière centralisée : il calculera l’ensemble des fingers tables après avoir tiré aléatoirement les identifiants des pairs.

Le processus simulateur construit d'abord un tableau contenant les différents identifiants CHORD de notre DHT, tous unique, et tiré aléatoirement sur l'intervalle [0, 2<sup>M</sup> - 1].

Il construit ensuite, pour chaque processus leurs table de fingers et leurs envoie. La table est construite de la manière suivante :

```
Pour un processus d’identifiant CHORD id :

Pour chaque finger j allant de 0 à M :
    soit la clé = (id_chord + 2**j)
    recherche du plus petit pair dont l’id CHORD est supérieur à la clé 
                                       (en respectant l’ordre cyclique)
```

Une fois les M fingers construits, ils sont envoyer au pair concerné.

### Recherche d'une clé

Implémentation la recherche du pair responsable d’une clé CHORD par un pair quelconque.

Soit un pair quelconque initiateur de la recherche.






- id_chord : le tableau d'id_chord de chaque processus.
    - > Nous enverrons à chaque processus son id_chord

- fingers : tableau à deux dimensions contenant 
     - > Pour chaque pairs : la liste de ses fingers (avec finger[0] son successeur)
       - > Pour chaque finger : 
         - fingers[0] : son MPI_rank
         - fingers[1] : son id_chord

### variable globales propre à chaque processus

Status status;          // MPI_status

int rank;               // MPI_rank du site

int id_chord;           // id_chord du site

int fingers [M][2];     // table de routage du site courant

### lookup :


### compilation :

ajouter `-lm -ldl` lors de la compilation


## Exercice 2

Simulateur : Tire N id_chord, et les donnes à des processus pris aléatoirement.
Défini un certains nombre d'initiateur.

Nous nous baserons sur l'algorithme de Hirschberg & Sinclair.

