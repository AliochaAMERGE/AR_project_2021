# AR (MU4IN403)  - PROJET : Implémentation du protocole P2P CHORD

Nous supposerons l'envoie de message FIFO

### define

- On a NB_PROC proc : contenant les N pairs, +1 processus inititateur

- N : le nombre de pair soit NB_PROC - 1

- M : la plage de valeurs (allant de 0 à (2^M)-1)



### Variables dans le processus initiateur :

- id_chord : le tableau d'id_chord de chaque processus.
    - > Nous enverrons à chaque processus son id_chord

- fingers : tableau à trois dimensions contenant 
     - > Pour chaque pairs : la liste de ses fingers (avec finger[0] son successeur)
       - > Pour chaque finger : son id_chord et son rank MPI

