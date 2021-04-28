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

int fingers [M][2];     // table de routage du site

### lookup :

lookup(k,initiator p) => prend en parametre la clé rechercher et l'initiateur de la requete
    => il faut qu'on memorise l'initiateur pour qu'on puisse le repondre à la fin
    next = recupere la pair càd le prochain à qui faut envoyer 
            (calculer par une fonction findnext => recherche du plus grand finger qui ne depasse pas la valeur rechercher )
    si next = nul alors envoie au successeur qui est donc finger[0] 
            (on l'appelle lastchance car c'est la derniere chance de trouver la clé)
                on lui rappelle qui est l'initiateur et la clé rechercher
    sinon envoi à next avec un message de type lookup 
        et lorsque le next recoit ce type de message il fera lui appel à lookup


Reception du type message "lastchance"
    si k app à mon ensemble 
        alors envoi type de message succes et p à l'initiateur
    sinon nil en cas d'echec de recherche




il a parler de initiator il a dit qu'on devrait pas envoyer à initiateur car pas obliger 
qu'il soit dans la table de routage.... j'ai pas trop compris ce qu'il a expliquer ... 

### compilation :

ajouter `-lm -ldl` lors de la compilation