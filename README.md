<img align="right" height="80" width="200" src="/img/index.png">

## [AR (MU4IN403)  - PROJET : Implémentation du protocole P2P CHORD](https://github.com/AliochaAMERGE/AR_project_2021.git "lien du projet")

Réalisé par :

- [**Namrata MISTRY M1 SAR**](https://github.com/mnam9807 "Namrata MISTRY")
- [**Aliocha AMERGÉ M1 SAR**](https://github.com/AliochaAMERGE "Aliocha AMERGÉ")

**©SU/LMD/MU4IN403**

Lien du projet : https://github.com/AliochaAMERGE/AR_project_2021.git

Ce projet a été réalisé dans le cadre de l'UE AR (MU4IN403) du master 1 Informatique mention SAR de *Sorbonne Université*.

<div style="page-break-after: always;"></div>

## Table des matières
<font size="2">
<font color=purple>

- [AR (MU4IN403)  - PROJET : Implémentation du protocole P2P CHORD](#ar-mu4in403----projet--implémentation-du-protocole-p2p-chord)
- [Table des matières](#table-des-matières)
- [Introduction et mise en contexte](#introduction-et-mise-en-contexte)
  - [Constantes](#constantes)
  - [Méthodes utilitaires](#méthodes-utilitaires)
  - [Indication pour l'éxécution des codes :](#indication-pour-léxécution-des-codes-)
- [Exercice 1 : Recherche d’une clé](#exercice-1--recherche-dune-clé)
  - [Initialisation de la DHT](#initialisation-de-la-dht)
  - [Recherche d'une clé](#recherche-dune-clé)
- [Exercice 2 - Calcul des finger tables](#exercice-2---calcul-des-finger-tables)
  - [Introduction](#introduction)
  - [Algorithme](#algorithme)
    - [Étape 1 : Élection d’un leader](#étape-1--élection-dun-leader)
    - [Étape 2 : Collecte des identifiants CHORD](#étape-2--collecte-des-identifiants-chord)
    - [Étape 3 : Diffusion du tableau d’identifiant](#étape-3--diffusion-du-tableau-didentifiant)
    - [Étape 4 : Constitution de la finger table.](#étape-4--constitution-de-la-finger-table)
  - [Justification de la correction de notre algorithme](#justification-de-la-correction-de-notre-algorithme)
    - [Sûreté :](#sûreté-)
    - [Vivacité :](#vivacité-)
  - [Complexité en nombre de messages](#complexité-en-nombre-de-messages)
- [Exercice 3 - Insertion d'un pair](#exercice-3---insertion-dun-pair)
  - [Introduction](#introduction-1)
  - [Algorithme](#algorithme-1)
    - [Étape 0 : Création de la DHT et du nouveau pair par le simulateur](#étape-0--création-de-la-dht-et-du-nouveau-pair-par-le-simulateur)
    - [Étape 1 : Recherche du successeur du nouveau pair](#étape-1--recherche-du-successeur-du-nouveau-pair)
    - [Étape 2 : Mise à jour des inverses du successeur et du nouveau pair](#étape-2--mise-à-jour-des-inverses-du-successeur-et-du-nouveau-pair)
    - [Étape 3  : Recherche des fingers du nouveau pair et mise à jour des inverses pour ces pairs](#étape-3---recherche-des-fingers-du-nouveau-pair-et-mise-à-jour-des-inverses-pour-ces-pairs)
    - [Étape 4 : Phase de terminaison et affichage](#étape-4--phase-de-terminaison-et-affichage)
  - [Justification de la correction de notre algorithme](#justification-de-la-correction-de-notre-algorithme-1)
    - [Sûreté :](#sûreté--1)
    - [Vivacité :](#vivacité--1)
    - [Compléxité](#compléxité)

</font>
<div style="page-break-after: always;"></div>

## Introduction et mise en contexte

*CHORD* est une table de hachage distribuée (DHT). 

&emsp;&emsp;Cela signifie que l’objectif du système est de stocker des données de manière distribuée et d’associer une clé à chacune d'elles. 
De plus, le système doit fournir des fonctions permettant de retrouver une donnée à partir de sa clé, de stocker une donnée (en construisant sa clé), de supprimer une donnée du système, etc. 

&emsp;&emsp;Enfin, vu le contexte des réseaux P2P qui sont par essence hautement dynamiques, le protocole doit intégrer une gestion du départ et de l’arrivée de nœuds dans le système (celle-ci peut être volontaire ou  subie). 
Pour plus de détails, se référer à : [I. Stoica, R. Morris, D. Karger, M. Kaashoek, H. Balakrishnan : Chord : A scalable peer-to-peer lookup service for internet applications, SIGCOMM 2001, pp. 149-160](https://pdos.csail.mit.edu/papers/chord:sigcomm01/chord_sigcomm.pdf "Chord : A Scalable Peer-to-peerLookupServiceforInternetApplications").

Nos implémentations au cours de ce projet seront en langage C et se baseront sur la norme MPI avec l'utilisation de la bibliothèque [Open MPI](https://www.open-mpi.org/ "open-MPI").

Nous supposerons l'envoie de messages FIFO et fiable tel que le garanti MPI.

Tout au long de ce projet, nous vérifierons à bien respecter l'ordre cyclique.

### Constantes

Au cours de ce projet nous utiliserons souvent les constantes suivantes :

&emsp;&emsp;**•** **NB_PROC** : Le nombre de processus MPI qui seront créer, 
&emsp; &emsp; &emsp; &emsp;&emsp;&emsp; cela correspond aux N pairs, + 1 processus inititateur *P0*

&emsp;&emsp;**•** **N** : le nombre de pairs soit NB_PROC - 1

&emsp;&emsp;**•** **M** : la plage de valeurs ( allant de 0 à (2<sup>M</sup>) - 1 )
&emsp; &emsp; &emsp; et également le nombre de fingers de chaque pairs


### Méthodes utilitaires

**int f(pair)** : retourne aléatoirement un identifiant CHORD unique parmis l'ensemble des pairs du système.

**int g(pair)** : retourne aléatoirement un identifiant unique pour une donnée (clés) parmis l'ensemble des données du système.

**int app(int k, int a, int b)** : vérifie si la clé k appartient à l'intervalle ]a,b].

<div style="page-break-after: always;"></div>

### Indication pour l'éxécution des codes :

<details>
  
  <summary>Arborescence : </summary>

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

</details>



Afin de compilé un fichier .c utilisant MPI :

il est nécéssaire d'utiliser `mpicc` pour générer un executable.
et de l'éxecuter avec `mpirun -np $2 --oversubscribe`  { avec $2 le nombre de processus }

Chaque exercice dispose de son dossier et de son Makefile.

Du fait que chaque exercice se compose d'un unique fichier `.c`, un script `runmpicc.sh` est présent afin de faciliter l'éxecution.   
Ce dernier prend 2 paramètre : le chemin vers le fichier source, et le nombre de processus utile.   
Il compilera le fichier source, produira un executable, le lancera et le supprimera.   

De plus il est nécéssaire d'ajouter `-lm -ldl` au flags pour la compilation.

<div style="page-break-after: always;"></div>

## Exercice 1 : Recherche d’une clé

Fichier : [*key_search.c*](Exercice_1/src/key_search.c)


### Initialisation de la DHT

Dans cet exercice, un processus simulateur initialisera la DHT CHORD de manière centralisée : il calculera l’ensemble des fingers tables après avoir tiré aléatoirement les identifiants des pairs.

Le processus simulateur construit d'abord un tableau contenant les différents identifiants CHORD de notre DHT, tous unique, et tiré aléatoirement sur l'intervalle [0, 2<sup>M</sup> - 1].

Il construit ensuite, pour chaque processus leurs table de fingers et leurs envoie. La table est construite de la manière suivante :

```py
for each processus d’identifiant CHORD id :

  for each finger j allant de 0 à M :
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

&emsp;&emsp;**•** recherche du plus grand pair d'identifiant chord inférieur à la clé recherchée.
  &emsp;&emsp;&emsp;&emsp;**•**  Si ce pair est le successeur du pair courant, il est en charge de la donnée
  &emsp;&emsp;&emsp;&emsp;**•**  Sinon : on continue la recherche

&emsp;&emsp;**•** Une fois le responsable d'une clé trouvé, nous faisons remonter son identité au pair initiateur, qui l'envoie au simulateur et affiche le résultat.

<div style="page-break-after: always;"></div>


## Exercice 2 - Calcul des finger tables

Fichier : [*finger_table.c*](Exercice_2/src/finger_table.c)

### Introduction

&emsp;&emsp;L’objectif de cet exercice est de réaliser l’initialisation de la DHT CHORD avec une complexité en messages sous-quadratique (soit inférieure à *|Π|*<sup>2</sup> ) de manière distribuée. C'est-à-dire, pour chaque pair, le calcul de sa finger table.

&emsp;&emsp;Initialement, les pairs sont organisés en anneau bidirectionnel ordonné en fonction du rang MPI et non pas en fonction des identifiants CHORD des pairs. Chaque pair connaît donc son voisin de droite et son voisin de gauche, et a la possibilité d’envoyer des messages uniquement à ses deux voisins.

&emsp;&emsp;Le simulateur est encore présent dans cette partie, mais son impact est réduit, il se chargera de donner à tous les pairs leurs identifiants CHORD, les voisins gauche et droite, et leur indiquer s'ils sont initiateur ou non.

&emsp;&emsp;Pour cet exercice, contrairement au précédent, nous partirons du principe qu'aucun noeud ne connait initialement la taille de l'anneau, cette derniere sera détérminée au cours de l'élection.

### Algorithme

&emsp;&emsp;Notre approche afin de résoudre ce problème est quelque peu naïve, nous essayons de reproduire le comportement du simulateur de l’exercice 1 en élisant un pair qui prendra ce rôle.

&emsp;&emsp;Nous ne tirons pas totalement partis de la distribution de l’anneau, ni de sa bidirectionnalité dans la deuxième partie.

L’algorithme se divise en quatre étapes :

&emsp;**•** **Étape 1** : Élection d’un leader ( basé sur l’algorithme de Hirschberg & Sinclair )

&emsp;**•** **Étape 2** : Le leader sera chargé de collecter les différents identifiants CHORD des pairs, afin d’en établir un tableau.

&emsp;**•** **Étape 3** : Une fois le tableau d’identifiants CHORD complété, ce dernier sera retransmis à tous les pairs de l’anneau.

&emsp;**•** **Étape 4** : Chaque pair ayant reçu le tableau précédemment construit se chargera de constituer sa finger table.


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

A ce moment, nous pouvons déterminé la taille de l'anneau à partir du nombre d'étape ayant eu lieu pour le processus élu, et la distance restante dans le dernier message OUT (étant revenu au leader).

taille de l'anneau = 2<sup>k</sup> - distante restante

Dans notre implémentation nous ajoutons 1 à la taille de l'anneau afin de prendre en compte le processus simulateur et de simplifier l'envoie de message.

<div style="page-break-after: always;"></div>

#### Étape 2 : Collecte des identifiants CHORD

La collecte des identifiants se fera par la diffusion d’un tableau dans lequel chaque pair indiquera son identifiant CHORD. Cette collecte est initiée par le pair précédemment élu.

#### Étape 3 : Diffusion du tableau d’identifiant

La diffusion suit le même fonctionnement que la collecte. Le tableau précédemment constitué est diffusé dans l’anneau et stocké par tous les pairs.

#### Étape 4 : Constitution de la finger table.

Tout d'abord, on crée un tableau temporaire qui sera ordonné en fonction des identifiants CHORD. Une fois le tableau ordonné, on procède au calcul des finger qui est de taille M. 

Voici le pseudo-code pour le calcul des fingers :

```py
for each processus d’identifiant CHORD id :

  for each finger j allant de 0 à M :
      soit la clé = (id_chord + 2**j)
      recherche du plus petit pair dont l’id CHORD est supérieur à la clé 
                                       (en respectant l’ordre cyclique)
```

### Justification de la correction de notre algorithme

La justification d’un algorithme se classent en deux catégories : la **sûreté** et la **vivacité**. 

Le protocole *MPI* garantit les canaux d’envoie de message *FIFO* et *fiables*. Nous savons donc que les messages arriveront dans le bon ordre, et qu’aucun ne sera perdu en cours de route.

#### Sûreté :

&emsp;Un processus pair est élu si il est initiateur **et** que son identifiant CHORD est plus grand que les autres pairs. Il est élu lorsque que le tag *TAG_OUT*, qu’il avait envoyé à une distance 2<sup>k</sup>, lui revient. L’unicité des identifiants CHORD garantit la propriété de sûreté et qu’**un** seul des pairs sera élu et deviendra le leader.

&emsp;&emsp;Après avoir un leader, nous pouvons garantir que le tableau des identifiants CHORD sera bien construit avec tous les identifiants des pairs existant dans l’anneau car le tag TAG_COLLECTE lancé par le leader, dans le sens de l’aiguille d’une horloge (unidirectionnel), lui sera retourné par son prédécesseur.

&emsp;&emsp;Une fois que le tag *TAG_COLLECTE* est revenu au leader, il relance un tour de message de manière unidirectionnel pour diffuser le tableau rempli de tous les identifiants CHORD. C’est ainsi que chacun des pairs de la DHT aura connaissance des identifiants des autres pairs et pourra donc établir le calcul des finger table.

#### Vivacité :

&emsp;&emsp;Au total, le leader lance deux messages de type *TAG_COLLECTE*. Le premier tag lancé consiste à collecter les identifiants CHORD et le deuxième permet de faire la diffusion du tableau rempli et aussi de lancer la terminaison de l’algorithme. On peut garantir la terminaison de l’algorithme car à la réception du deuxième tag, chaque processus pair peut procéder au calcul des finger table et de l’afficher. 



### Complexité en nombre de messages

&emsp;&emsp;La complexité de l’algorithme se base en nombre de messages envoyés. Supposons **N** le nombre de pairs dans la DHT. Chacun des pairs envoie 2 messages (voisins de droite et gauche) qui parcourent chacun une distance de 2<sup>k</sup>( avec **k** le nombre d’étapes).

&emsp;&emsp;Pour l’algorithme de l’élection du leader, on a une complexité en nombre de messages inférieur ou égale à **N*K**, soit une complexité en O(N.log<sub>2</sub>(N)). Le log<sub>2</sub>(N) correspond au nombre d’étapes. 

&emsp;&emsp;Ensuite lorsque le leader est élu, il lance la collecte des identifiants CHORD en partant par le voisin de droite et fait donc un tour complet dans une seule direction. On aura donc une complexité en *O(N)*. Et après avoir tout collecter, le leader diffuse le tableau remplit des identifiants CHORD en partant du même principe pour la collection. La diffusion se fait donc avec une complexité en *O(N)*.

&emsp;&emsp;On a donc une complexité de *N.log<sub>2</sub>(N) + 2N* en nombre de messages, soit une complexité en *O(N.log<sub>2</sub>(N))*.

<div style="page-break-after: always;"></div>

## Exercice 3 - Insertion d'un pair

Fichier : [*insertion_pair.c*](Exercice_3/src/insertion_pair.c)


### Introduction

&emsp;&emsp;L’objectif de cet exercice est de réaliser l'insertion d'un nouveau pair dans la DHT CHORD avec une complexité en messages sous-linéaire (soit inférieure à *|Π|*) de manière distribuée.

&emsp;&emsp;Dans cet exercice, nous supposons avoir une DHT CHORD correctement initialisée. Nous supposons de plus que tout pair de rang MPI p dispose d’une *liste inverse* p contenant l’identifiant (et le rang MPI) de tout pair q ayant un finger sur p (i.e., il existe un k tel que _finger<sub>q</sub> [k] = id<sub>p</sub>_).

&emsp;&emsp;Initialement, le nouveau pair ne peut envoyer des messages qu'à un unique pair choisi par le simulateur. De plus, il ne sera capable initialement de n’envoyer des messages qu’à un unique pair (choisi arbitrairement) de la DHT. Pour envoyer un message à d’autres pairs, il devra être informé de leur existence par un pair déjà présent dans la DHT.

&emsp;&emsp;Nous reprenons le simulateur du premier exercice en le modifiant de manière à ce qu'ils construise la liste des *inverses* des pairs de la DHT (en excluant le nouveau pair). Après avoir initialisé la DHT, le simulateur envoi au nouveau pair son identifiant CHORD ainsi que le rang du pair qui a été élu comme son responsable (choisi arbitrairement). 

&emsp;&emsp;Lorsque le nouveau pair réussi à s'insérer dans la DHT, il notifera le simulateur, et ce dernier lancera la terminaison de l'ensemble des processus de la DHT.

**NB** : Nous utiliserons souvent les termes suivant :

* **nouveau pair** : le pair s'insérant dans la DHT
* **responsable** : le responsable initial de ce nouveau pair
* **successeur** : le successeur du nouveau pair

De plus, l'*ordre cyclique* sera toujours respecté.

### Algorithme

Notre algorithme se déroule sur quatre étapes :

&emsp;&emsp;**•** **Étape 1** : Recherche du successeur du nouveau pair

&emsp;&emsp;**•** **Étape 2** : Mise à jour des inverses du successeur et du nouveau pair

&emsp;&emsp;**•**  **Étape 3** : Recherche des fingers du nouveau pair et mise à jour des inverses pour ces pairs

&emsp;&emsp;**•**  **Étape 4** : Phase de terminaison et affichage

<div style="page-break-after: always;"></div>

#### Étape 0 : Création de la DHT et du nouveau pair par le simulateur

Le simulateur qui créé la DHT, et insére le nouveau pair à l'aide des *TAG_INIT* et *TAG_INSERTION*.

La création de la DHT est identique à l'exercice 1, la table des inverses en plus. De plus nous excluons un pair, ce dernier sera le nouveau pair.

- *TAG_INSERTION*
```py
def réception d’un message TAG_INSERTION(<nouveau_pair_id_chord, id_chord_resp_nouveau, 
                                          MPI_rang_resp_nouveau, TAG_INSERTION>) :
  # recherche du successeur
  envoyer <nouveau_pair_id_chord, nouveau_pair_MPI_rang, TAG_RECHERCHE > à MPI_rang_resp_nouveau
  # attente de la table des inverses 
  réception <inverse, id_chord_sucesseur, MPI_rang_successeur, TAG_MAJ_INVERSE>
  # attente de la table des fingers
  réception <fingers,id_chord_sucesseur, MPI_rang_successeur, TAG_SUCCESSEUR>
  #envoie d'un message pour que ses fingers mettent à jour leur table inverse
  for each unique j ∈ fingers :
    envoyer <nouveau_pair_id_chord, nouveau_pair_MPI_rang, TAG_INVERSE> à j
  # notifie le simulateur la fin de son insertion dans la DHT
  envoyer <TAG_FIN> au simulateur
```

#### Étape 1 : Recherche du successeur du nouveau pair

&emsp;&emsp;Nous cherchons le successeur du nouveau pair car il sera beaucoup plus simple de trouver les fingers et inverses du nouveau pair à l'aide de son successeur, et ce gain en message est supérieur à la dépense liée à la recherche.

&emsp;&emsp;Cette recherche sera réalisée par le pair responsable du nouveau pair, étant le seul connaissant son existence initialement. 

&emsp;&emsp;Pour ce faire, nous utilisons sensiblement le même algorithme que pour l'exercice 1. Nous cherchons le plus grand pair parmis les fingers du pair responsable étant plus petit que l'identifiant CHORD du nouveau pair à l'aide du *TAG_RECHERCHE*, la seule différence est que nous tirons partie de la table des inverses. Si nous cherchons une valeurs dans la premiere moitiée de l'anneau (par rapport au pair lançant la recherche), nous cherchons dans la table de fingers. Si nous cherchons dans la deuxieme moitiée (partie inférieure), nous cherchons dans la table d'inverse.

&emsp;&emsp;Une fois le successeur trouvé, il sera prévenu à l'aide du *TAG_SUCCESSEUR*, nous attendons la réception des différents attribut lié à la DHT (la table des fingers, et la table des inverses).
Le successeur sera en charge de la suite des opérations, le nouveau pair reste uniquement en attente des messages de son successeur.

<div style="page-break-after: always;"></div>

- *TAG_RECHERCHE*
```py
def réception d’un message TAG_RECHERCE (<nouveau_pair_id_chord, 
                                  nouveau_pair_MPI_rang, TAG_RECHERCHE>) :
  # en respectant l'ordre cyclique
  if(nouveau_pair_id_chord > id_chord):
    next = recherche du successeur du nouveau_pair_id_chord dans la *table des fingers*
    if successeur trouver :
      # l'informer de sa responsabilité en tant que successeur
      envoyer <nouveau_pair_id_chord, nouveau_pair_MPI_rang, TAG_SUCCESSEUR> au successeur
      [...] # cf étape suivante
  else if(nouveau_pair_id_chord > id_chord):
    next = recherche du successeur du nouveau_pair_id_chord dans la *table des inverses*
  # continue la recherche du successeur
  envoyer <nouveau_pair_id_chord, nouveau_pair_MPI_rang, TAG_RECHERCHE> à next
```


#### Étape 2 : Mise à jour des inverses du successeur et du nouveau pair

&emsp;&emsp;Tout d'abord, le successeur se charge de mettre à jour sa table des inverses ainsi que celle du nouveau pair. 

&emsp;&emsp;Le successeur était auparavant en charge de toutes les clés entre sont prédécesseur (exclus) et lui même (inclus). Or avec l'arrivé du nouveau pair, cet intervalle est réduit. En effet le nouveau pair sera en charge des données entre le predecesseur (exclus) et lui même (inclus), et le successeur deviens responsable des données situées entre le nouveau pair (exclus) et lui même (inclus).

&emsp;&emsp;Afin de mettre à jours les inverses du nouveau pair, on envoi un message avec un tag *TAG_MAJ_INVERSE* à l'ensemble des inverses du successeur. Chaque pair de cet ensemble devra revérifier le responsable de ses clés dont l'identifiant CHORD est égale à l'identifiant du successeur du nouveau pair. Si ce dernier nécéssite une mise à jour, l'indiquera dans sa table de fingers, et informera le successeur du nouveau pair.

&emsp;&emsp;Ce dernier mettra donc à jour sa table d'inverse, et celle du nouveau pair en fonction des informations qu'il recevra.

<div style="page-break-after: always;"></div>

- *TAG_SUCCESSEUR*

```py
def réception d’un message TAG_SUCCESSEUR (<nouveau_pair_id_chord, 
                                  nouveau_pair_MPI_rang, TAG_SUCCESSEUR>):
  for i in inverses : # parcours de la table d'inverse du successeur
    if(inverse[i] != -1):
      envoyer <nouveau_pair_id_chord, nouveau_pair_MPI_rang,
                            successseur_id_chord, TAG_MAJ_INVERSE > à i
      reception <i, nouveau_pair_id_chord, 
                                successseur_id_chord, TAG_MAJ_INVERSE>
      
      # nous gardons successeur dans la table des inverses
      if(successseur_id_chord != -1): 
        pas de mise à jour
      else :
        # nous retirons *i* de notre table d inverse
        mise à jour de inverse[i] du successeur

      # nous ajoutons i dans la table d'inverse du nouveau pair
      if(nouveau_pair_id_chord != -1):
        mise à jour de inverse[i] du nouveau pair avec les informations de i
  # fin de la mise à jour de la table inverse
  envoyer <inverses, TAG_MAJ_INVERSE> à nouveau_pair_id_chord
  [...] # cf etape 3
```
- *TAG_MAJ_INVERSE*
```py
def réception d’un message TAG_MAJ_INVERSE (<nouveau_pair_id_chord, 
                    nouveau_pair_MPI_rang, MPI_source TAG_MAJ_INVERSE>):

  for each finger pointant vers le successeur du nouveau pair :
    # nous recalculons la clé pour vérifier le besoin de mise à jour
    cle = (id_chord + 2**i)
    if cle ∈ [id_chord_source, nouveau_pair_id_chord[ :
      mise à jour du finger
      ajout du flag dans le message pour prevenir le successeur
    else :
      pas de mise à jour

  envoyer<mise à jour, TAG_MAJ_INVERSE> au successeur du nouveau pair

```

#### Étape 3  : Recherche des fingers du nouveau pair et mise à jour des inverses pour ces pairs

&emsp;&emsp;Après la mise à jour des inverses faites à l'étape précédente, le successeur procéde au cacul de la table des fingers du nouveau pair. Pour ce faire, il calcul les clé des fingers du nouveau pair (connaissant son id_chord) à l'aide de la formule : ( nouveau_pair_id_chord + 2<sup>j</sup> *avec j le numéro de finger*) . Pour chaque clé, le successeur du nouveau pair lance une recherche (avec un tag *TAG_FINGERS*) de la valeurs de la clé pour trouver le pair responsable de cette dernière, et l'ajoute dans la table des fingers du nouveau pair.

**NB** : Certaines clé (comprise entre ]*nouveau_pair*, *successeur*]) seront prise en charge par le successeur, nous n'aurons donc pas besoin d'envoyer de message. De même pour les clé compris entre ]successeur du nouveau pair, successeur du successeur].

- *suite du TAG_SUCCESSEUR**
```py
def réception d’un message TAG_SUCCESSEUR (<nouveau_pair_id_chord, 
                                nouveau_pair_MPI_rang, TAG_SUCCESSEUR>):
  [...] # cf etape 2
  # calcul de la table des fingers du nouveau pair
  for j allant de 0 à M :
    clé = (id_chord + 2**j)
    if cle ∈ [successseur_id_chord, nouveau_pair_id_chord[ :
      successeur est responsable de cette clé
    else:
      next = recherche du responsable de cette clé
      if(next = NIL):
        successeur est responsable de cette clé
      else:
        envoyer <cle, TAG_FINGERS> à next
        reception <responsable_cle, TAG_FINGERS>
        mise à jour de finger[j]
  # fin du calcul des fingers
  envoyer <fingers, TAG_SUCCESSEUR> au nouveau pair
```
- *TAG_FINGERS*

```py
def reception d’un message TAG_FINGERS (<clé recherchée, TAG_FINGERS>):
  next = recherche du responsable de cette clé
  if(next = NIL):
      successeur est responsable de cette clé
      envoie de l’identifiant du responsable de la clé à la source 
          (nous faisons remonter l’information jusqu’au successeur du nouveau pair)
    else:
      envoyer <cle, TAG_FINGERS> à next
      reception <responsable_cle, TAG_FINGERS>
      envoie de l’identifiant du responsable de la clé à la source 
          (nous faisons remonter l’information jusqu’au successeur du nouveau pair)
```

- *TAG_INVERSE*
```py
def réception d’un message TAG_INVERSE (<id_chord_inverse, rang_inverse, TAG_INVERSE>):
  # mise à jour de la table inverse à l'indice rang_inverse avec le contenue du message recu
  inverse[rang_inverse] = {rang_inverse, id_chord_inverse}
```
<div style="page-break-after: always;"></div>


#### Étape 4 : Phase de terminaison et affichage

&emsp;&emsp;Une fois la table des fingers, et la table des inverses du nouveau pair construite, l'état de la DHT équilibré, le successeur envoie ces deux tables au nouveau pair. Ce dernier envoi un message avec un tag *TAG_INVERSE* à tous ses fingers pour qu'ils puissent mettre à jour leur table inverse.

&emsp;&emsp;Une fois ses attributs reçu, le nouveau pair signale au simulateur de la terminaison de son insertion. Ce dernier enverra un *TAG_FIN* à tous les pairs de la DHT, et ces dernier sortiront de leurs boucle de réception, et se termineront proprement.


### Justification de la correction de notre algorithme

La justification d’un algorithme se classent en deux catégories : la **sûreté** et la **vivacité**. 

Le protocole *MPI* garantit les canaux d’envoie de message *FIFO* et *fiables*. Nous savons donc que les messages arriveront dans le bon ordre, et qu’aucun ne sera perdu en cours de route.

#### Sûreté :

&emsp;&emsp;L'algorithme se compose majoritairement d'une majorité de recherche de clé, que ce soit pour trouver le successeur, mettre à jour les fingers, ou les inverses. La recherche étant sûr, nous n'aurons pas de problème à ce niveau là.

&emsp;&emsp;Une fois trouvé, le successeur du nouveau pair se charge d'envoyer et recevoir une grande majorité des messages nécessaires à l'insertion du nouveau pair, ces envoies sont successifs, et controlés.

&emsp;&emsp;Après l'insertion du nouveau pair, nous pouvons garantir qu'il notifiera tous les pairs présent dans sa table de fingers afin qu'ils mettent à jour leurs table d'inverses. Et nous pouvons aussi garantir qu'après cette mise à jour, le nouveau pair fini par notifier le simulateur entrainant la terminaison des différents pairs de la DHT.

<div style="page-break-after: always;"></div>

#### Vivacité :

**Etape 1** : on assure la recherche d'un successeur (cf Exercice 1)

**Etape 2** : on assure la mise à jour de la table de fingers et d'inverse du nouveau pair, cette dernière étant gérée par son successeur, en recalculant l'ensemble des clés.

**Etape 3** : on assure la mise à jour des inverse des différents pair à l'aide du TAG_INVERSE. Chaque pair met à jour sa table inverse en fonction des messages reçu du nouveau pair.

Concernant les fingers, seul ceux pointant vers le successeur du nouveau pair pourrait être impacter par l'insertion. Or le successeur en question s'occupera de notifier chacun de ses inverses afin qu'ils mettent à jour leurs tables. 

On peut donc garantir que la table des fingers et la table des inverses de tous les pairs de la DHT sont bien à jour.

&emsp;&emsp;A la fin de l'insertion du nouveau pair, la DHT sera dans un état global cohérent avec sa spécification, du fait que pour chaque fingers du nouveau pair, les pairs concerné sont informé. De même pour les inverses du nouveau pair, ces derniers recalculent leurs fingers en fonction du nouveau pair. 

#### Compléxité

&emsp;&emsp;**•** Étape 1 : Recherche du successeur du nouveau pair

Recherche simple, pire cas *log(N)* messages.

&emsp;&emsp;**•** Étape 2 : Mise à jour des inverses du successeur et du nouveau pair

Envoie d'un message à tout les inverses sur successeur, il y a **au plus** *N* messages, cela signifierais que l'enseble de la DHT ai un finger pointant sur ce dernier.

&emsp;&emsp;**•** Étape 3 : Recherche des fingers du nouveau pair et mise à jour des inverses pour ces pairs

Nous lançons la recherche de au plus *M clés*, ce qui fait un total d'au plus *M\*log(N)* messages.

Nous avons donc une complexité en nombre de messages de *N* messages.

</font>
