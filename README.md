# AR (MU4IN403)  - PROJET : Implémentation du protocole P2P CHORD

Nous supposerons l'envoie des messages FIFO et fiable.

## Exercice 1 : Recherche d’une clé

### define

- On a NB_PROC proc : contenant les N pairs, +1 processus inititateur

- N : le nombre de pair soit NB_PROC - 1

- M : la plage de valeurs (allant de 0 à (2^M)-1)



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

