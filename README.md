# AR (MU4IN403)  - PROJET : Implémentation du protocole P2P CHORD

<img align="right" src="/img/index.png">

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

Tout au long de ce projet, nous vérifierons à bien respecter l'ordre cyclique.

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
│   └── src
│       ├── include
│       │   └── header.h
│       └── key_search.c
├── Exercice_2
│   ├── Makefile
│   ├── obj
│   └── src
│       ├── include
│       │   └── header.h
│       └── finger_table.c
├── Exercice_3
│   ├── Makefile
│   ├── obj
│   └── src
│       ├── include
│       │   └── header.h
│       └── insertion_pair.c
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
NB: le finger[0] est le successeur du pair courant dans l'anneau.

Une fois les M fingers construits, ils sont envoyer au pair concerné.

### Recherche d'une clé

Implémentation la recherche du pair responsable d’une clé CHORD par un pair quelconque.

Soit un pair quelconque initiateur de la recherche.
La recherche d'un pair responsable d'un clé se réalise de la manière suivante :

- recherche du plus petit pair inférieur à la clé recherchée.
  - Si ce pair est le successeur du pair courant, il est en charge de la donnée
  - Sinon : on continue la recherche

- Une fois le responsable d'une clé trouvé, nous faisons remonter son identité au pair initiateur, 
qui l'envoie au simulateur et affiche le résultat.


## Exercice 2 - Calcul des finger tables

Fichier : *Exercice 2/src/finger_table.c*

### Introduction

&nbsp;&nbsp;&nbsp;&nbsp;L’objectif de cet exercice est de réaliser l’initialisation de la DHT CHORD avec une complexité en messages sous-quadratique (soit inférieure à *|Π|*<sup>2</sup> ) de manière distribuée. C'est-à-dire, pour chaque pair, le calcul de sa finger table.

&nbsp;&nbsp;&nbsp;&nbsp;Initialement, les pairs sont organisés en anneau bidirectionnel ordonné en fonction du rang MPI et non pas en fonction des identifiants CHORD des pairs. Chaque pair connaît donc son voisin de droite et son voisin de gauche, et a la possibilité d’envoyer des messages uniquement à ses deux voisins.

&nbsp;&nbsp;&nbsp;&nbsp;Le simulateur est encore présent dans cette partie, mais son impact est réduit, il se chargera de donner à tous les pairs leurs identifiants CHORD, les voisins gauche et droite, et leur indiquer s'ils sont initiateur ou non.

### Algorithme

&nbsp;&nbsp;&nbsp;&nbsp;Notre approche afin de résoudre ce problème est quelque peu naïve, nous essayons de reproduire le comportement du simulateur de l’exercice 1 en élisant un pair qui prendra ce rôle.
&nbsp;&nbsp;&nbsp;&nbsp;Nous ne tirons pas totalement partis de la distribution de l’anneau, ni de sa bidirectionnalité dans la deuxième partie.

L’algorithme se divise en quatre étapes :

- Étape 1 : Élection d’un leader ( basé sur l’algorithme de Hirschberg & Sinclair )

- Étape 2 : Le leader sera chargé de collecter les différents identifiants CHORD des pairs, avec d’en établir un tableau.

- Étape 3 : Une fois le tableau d’identifiants CHORD complété, ce dernier sera retransmis à tous les pairs de l’anneau.

- Étape 4 : Chaque pair ayant reçu le tableau précédemment construit se chargera de constituer sa finger table.


#### Étape 1 : Élection d’un leader 

```py
def Début de ronde k (k) :
    if initiateur :
        envoyer <id_chord, 2**k> à voisins de gauche et droite
        k++
# avec 2^k : la distance max à laquelle on envoie le message
# et id_chord : l’identifiant CHORD du pair
```

```py
def réception d’un message TAG_IN (<id_chord_initiateur, TAG_IN> ) :
    if id_chord_initiateur != id_chord :
        if émetteur == voisin droit :
            envoyer <id_chord_initiateur, TAG_IN> à voisins de gauche
        else :
            envoyer <id_chord_initiateur, TAG_IN> à voisins de droite
    else :
        if nous avons reçu 2 messages TAG_IN :
            initier_etape()
```


```py
def réception d’un message TAG_OUT (<id_chord_initiateur, distance, TAG_OUT ) :
  if !initiateur ou id_chord_initiateur > id_chord :
     initiateur = 0
     if distance > 1 :
       # Si le message n’a pas atteint sa distance maximale
        if émetteur == voisin droit :
            envoyer <id_chord_initiateur, distance - 1, TAG_OUT> à voisin de gauche
        else :
            envoyer <id_chord_initiateur, distance - 1, TAG_OUT> à voisin de droit
     else :
      # le message a atteint sa distance maximale, il entame son retour
        if émetteur == voisin droit :
            envoyer <id_chord_initiateur, TAG_IN> à voisin de droit
        else :
            envoyer <id_chord_initiateur, TAG_IN> à voisin de gauche
  else :
   # le message est revenu, nous avons un leader
     if id_chord_initiateur == id_chord : 
       id_chord élu
    # Le processus élu lancera la collecte et la diffusion des identifiants CHORD
```

#### Étape 2 : Collecte des identifiants CHORD

La collecte des identifiants se fera par la diffusion d’un tableau dans lequel chaque pair indiquera son identifiant CHORD. Cette collecte est initiée par le pair précédemment élu.

#### Étape 3 : Diffusion du tableau d’identifiant

La diffusion suit le même fonctionnement que la collecte. Le tableau précédemment constitué est diffusé dans l’anneau et stocké par tous les pairs.

#### Étape 4 : Constitution de la finger table.

Tout d'abord, on crée un tableau temporaire qui sera ordonné en fonction des identifiants CHORD. Une fois le tableau ordonné, on procède au calcul des finger qui est de taille M. 

Voici le pseudo-code pour le calcul des fingers :

```
Pour un processus d’identifiant CHORD id :

Pour chaque finger j allant de 0 à M :
    soit la clé = (id_chord + 2**j)
    recherche du plus petit pair dont l’id CHORD est supérieur à la clé 
                                       (en respectant l’ordre cyclique)
```

### Justification de la correction de notre algorithme

La justification d’un algorithme se classent en deux catégories : la **sûreté** et la **vivacité**. 

Le protocole *MPI* garantit les canaux d’envoie de message *FIFO* et *fiables*. Nous savons donc que les messages arriveront dans le bon ordre, et qu’aucun ne sera perdu en cours de route.

#### Sûreté :

&nbsp;&nbsp;&nbsp;&nbsp;Un processus pair est élu si il est initiateur **et** que son identifiant CHORD est plus grand que les autres pairs. Il est élu lorsque que le tag *TAG_OUT*, qu’il avait envoyé à une distance 2<sup>k</sup>, lui revient. L’unicité des identifiants CHORD garantit la propriété de sûreté et qu’**un** seul des pairs sera élu et deviendra le leader.
&nbsp;&nbsp;&nbsp;&nbsp;Après avoir un leader, nous pouvons garantir que le tableau des identifiants CHORD sera bien construit avec tous les identifiants des pairs existant dans l’anneau car le tag TAG_COLLECTE lancé par le leader, dans le sens de l’aiguille d’une horloge (unidirectionnel), lui sera retourné par son prédécesseur.
&nbsp;&nbsp;&nbsp;&nbsp;Une fois que le tag *TAG_COLLECTE* est revenu au leader, il relance un tour de message de manière unidirectionnel pour diffuser le tableau rempli de tous les identifiants CHORD. C’est ainsi que chacun des pairs de la DHT aura connaissance des identifiants des autres pairs et pourra donc établir le calcul des finger table.

#### Vivacité :

&nbsp;&nbsp;&nbsp;&nbsp;Au total, le leader lance deux messages de type *TAG_COLLECTE*. Le premier tag lancé consiste à collecter les identifiants CHORD et le deuxième permet de faire la diffusion du tableau rempli et aussi de lancer la terminaison de l’algorithme. On peut garantir la terminaison de l’algorithme car à la réception du deuxième tag, chaque processus pair peut procéder au calcul des finger table et de l’afficher. 



### Complexité en nombre de messages

&nbsp;&nbsp;&nbsp;&nbsp;La complexité de l’algorithme se base en nombre de messages envoyés. Supposons **N** le nombre de pairs dans la DHT. Chacun des pairs envoie 2 messages (voisins de droite et gauche) qui parcourent chacun une distance de 2<sup>k</sup>( avec **k** le nombre d’étapes).

&nbsp;&nbsp;&nbsp;&nbsp;Pour l’algorithme de l’élection du leader, on a une complexité en nombre de messages inférieur ou égale à **N*K**, soit une complexité en O(N.log<sub>2</sub>N). Le log<sub>2</sub>N correspond au nombre d’étapes. 

&nbsp;&nbsp;&nbsp;&nbsp;Ensuite lorsque le leader est élu, il lance la collecte des identifiants CHORD en partant par le voisin de droite et fait donc un tour complet dans une seule direction. On aura donc une complexité en *O(N)*. Et après avoir tout collecter, le leader diffuse le tableau remplit des identifiants CHORD en partant du même principe pour la collection. La diffusion se fait donc avec une complexité en *O(N)*.

&nbsp;&nbsp;&nbsp;&nbsp;On a donc une complexité de *N.log<sub>2</sub>(N) + 2N* en nombre de messages, soit une complexité en *O(N.log2(N))*.

## Exercice 3 - Insertion d'un pair

Dans cet exercice, nous supposons avoir une DHT CHORD correctement initialisée. Nous supposons de plus que
tout pair de rang MPI p dispose d’une liste inverse p contenant l’identifiant (et le rang MPI) de tout pair q ayant un
finger sur p (i.e., il existe un k tel que f inger q [k] = id p ).

