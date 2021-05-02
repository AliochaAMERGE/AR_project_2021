# AR (MU4IN403)  - PROJET : Implémentation du protocole P2P CHORD

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

### Constante

Au cours de ce projet nous utiliserons souvent les constantes suivantes :

- NB_PROC : Le nombre de processus MPI qui seront créer, 
    - cela correspond aux N pairs, + 1 processus inititateur *P0*

- N : le nombre de pairs soit NB_PROC - 1

- M : la plage de valeurs (allant de 0 à (2^M)-1)


## Exercice 1 : Recherche d’une clé




### Variables dans le processus initiateur :

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

