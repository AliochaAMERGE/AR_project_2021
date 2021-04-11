#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "header.h"

#define NB_PROC 11
#define N (NB_PROC - 1)
// nombre de sites (processus) en comptant le processus initiateur
#define M 6  // nombre de fingers

// ! il faut que N > M

/*LES TAGS */
#define NIL -1
#define TAG_INIT 0
#define TAG_TERM 1
/* TAG LOOKUP */
#define TAG_INIT_LOOKUP 2
#define TAG_LAST_CHANCE 3
#define TAG_LOOKUP 4
#define TAG_SUCC 5

/* les variables globales */
MPI_Status status;
int rank;
int id_chord;
int fingers[M][2];
int* C;  // les données gérée par le site courant
// c'est derniere ne sont pas gérée pour le moment

/* ******************* Fonction d'appartenance à [a,b[ ***********************
 *               app vérifie si la clé k appartient à                         *
 *               l'intervalle [a,b[                                           *
 **************************************************************************** */

int app(int k, int a, int b) {
  if (a < b) return ((k >= a) && (k < b));
  return (((k >= 0) && (k < b)) || ((k >= a) && (k < N)));
}

/******************************Fonctions Aléatoires****************************/

/* ***************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 **************************************************************************** */

// p : rank
// id_already_used : contient les idChord pour les p-1 ranks, p : rank
int f(int* id_already_used, int p) {
  srand(time(NULL));
  int alea_chord;
  // on a 2^M-1 valeurs possibles
  int i = 0;
  do {
    alea_chord = (rand() % ((int)pow(2, M)));  // retire une valeur aléatoire
    if (id_already_used[i] == alea_chord) {
      i = 0;  // repart du début
    } else {
      i++;
    }
  } while (i < p);

  id_already_used[p] = alea_chord;

  return alea_chord;
}
/* ***************************************************************************
 *            g retourne aléatoirement un identifiant unique pour une donnée  *
 *            (clés) parmis l'ensemble des données du système                 *
 **************************************************************************** */

int g(int value) { return value % (int)(pow(2, M) - 1); }

/******************************Fonctions pour trier****************************/
int compare(const void* a, const void* b) {
  int int_a = *((int*)a);
  int int_b = *((int*)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

int cycle_comparator(int a, int b) {
  // ne pas utiliser avec qsort,
  // -> undefined behavior, may infinite loop
  if (a == b)
    return 0;
  else if (abs(b - a) < (pow(2, M)) / 2)
    return 1;
  else
    return -1;
}

// qsort( array, element_count, element_size, *compare_function )

/*************Initialisation de la DHT par le processus simulateur*************/

void simulateur(void) {
  /*initialisation des variables*/
  int id_chord[NB_PROC];  // tableau des identifiants CHORD
  //  int fingers[M][2]; //tableau des M fingers
  // int succ;

  /*Identifiants chord*/

  /*Attribution des identifiants CHORD à chaque processus*/
  /*Sauf le processus 0 qui correspond au simulateur*/
  id_chord[0] = -1;
  for (int p = 1; p < NB_PROC; p++) {
    f(id_chord, p);
  }
  /*Trie dans l'ordre croissant les id_chord pour former l'anneau*/
  qsort(id_chord, NB_PROC - 1, sizeof(int), compare);

  /*Envoi des identifiants chord au pairs du système*/
  for (int p = 1; p < NB_PROC; p++) {
    MPI_Send(&id_chord[p], 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
  }

  /* ***************************************************
   * Initialisation des fingers pour chaque processus  *
   *************************************************** */

  /* ****************************************************
   * envoie successif des informations à la création de *
   * chaque ensemble de finger                          *
   **************************************************** */
  // pour chaque pair du systeme
  for (int p = 1; p < NB_PROC; p++) {
    /* ***************************************************
     * tableau des M fingers                             *
     ************************************************** */
    int fingers[M][2];
    /* ***************************************************
     *  finger[0] = successeur, temp_rank                *
     *  est le rank mpi du successeur                    *
     *************************************************** */

    int idChord = id_chord[p];
    int rank;

    for (int j = 0; j < M; j++)  // pour chaque finger du pair p
    {
      /* ***************************************************
       *                clé                                *
       *************************************************** */
      int cle = (idChord + (int)pow(2, j)) % ((int)pow(2, M));
      for (int i = 1; i < NB_PROC; i++) {
        if (id_chord[i] >= cle) {
          /* ***************************************************
           *                MPI Rank                           *
           *************************************************** */
          fingers[j][0] = rank;
          /* ***************************************************
           *                ID_CHORD                           *
           *************************************************** */
          fingers[j][1] = id_chord[rank];  // le responsable de la cle
          break;
        }
      }  // fin recherche pair associé au finger

    }  // fin for each finger du pair p

    // Envoie du tableau fingers au processus p
    MPI_Send(&fingers, M * 2, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);

  }  // fin for each processus

  /*------------------Fin de l'initialisation du système------------------*/

  printf("Fin du simulateur\n");
}  // fin simulateur

// ! todo commenter
void init() {
  MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  MPI_Recv(&fingers, M * 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
}

/*---------------------- Fin de la fonction simulateur -----------------------*/

/************** Recherche d'une clée (lancée par le simulateur) ***************/

void recherche(int pair, int key) {
  // Envoie d'un message à ce pair pour chercher le responsable de cette donnée

  MPI_Send(&key, 1, MPI_INT, TAG_INIT_LOOKUP, pair, MPI_COMM_WORLD);

  /* Attente de la réponse de la pair le responsable de cette donnée */
  int responsable;
  MPI_Recv(&responsable, 1, MPI_INT, pair, MPI_ANY_TAG, MPI_COMM_WORLD,
           &status);
  if (responsable == NIL) {
    printf("La donnée %d n'existe pas dans la DHT \n", key);
  } else {
    printf("Le responsable de la donnée %d est (<P%d>, <Pair_%d>)\n", key,
           status.MPI_SOURCE, responsable);
  }

  /*Progration du message de terminaison à tous les processus*/
  for (int p = 1; p < NB_PROC; p++) {
    // le contenu du message n'a pas d'importance, on ne recupere pas de valeurs
    // lors de la terminaison (pour le moment)
    MPI_Send(&p, 1, MPI_INT, p, TAG_TERM, MPI_COMM_WORLD);
  }
}

/*************Fonctions permettant de faire la recherche de la clé*************/

/* ****************************************************************************
 *               initiate_lookup lance le début de la recherche               *
 *               lookup gère la suite de la recherche                         *
 *****************************************************************************/

int lookup(int initateur_chord, int k) {
  int message[2] = {initateur_chord, k};
  // MPI_rank du plus grand finger ne dépassant pas k
  int next = findnext(k);

  if (next == NIL) {
    send(message, TAG_LAST_CHANCE, fingers[0][0]);
  } else {
    send(message, TAG_LOOKUP, next);
  }
}

int initiate_lookup(int k) {
  int initiateur_chord = id_chord;
  return lookup(initiateur_chord, k);
}

/* ****************************************************************************
 *               findnext permet de recherche du plan grand finger            *
 *               qui ne dépasse pas k, on fait appel a app()                  *
 *                pour respecter l'ordre cyclique                             *
 *****************************************************************************/

int findnext(int k) {
  // input : clés
  // output : MPI_rank
  for (size_t i = M - 1; i >= 0; i--) {
    if (app(k, fingers[i][1], id_chord)) {
      return fingers[i][0];
    }
  }
  return NIL;
}

int have_data(int k, int* C) { return 1; }

/*---------------------- Fin des fonctions de recherche ----------------------*/

/************** Fonctions d'envoie et de reception des messages ***************/
/* ****************************************************************************
 *               send permet d'envoyer un message a dest                      *
 *               avec le tag passé en paramètre                               *
 ***************************************************************************  */

void send(int* message, int tag, int dest) {
  MPI_Send(message, 2, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

/* ****************************************************************************
 *               receive permet de gérer la réception                         *
 *               des messages en fonction des tag                             *
 *****************************************************************************/

void receive() {
  int* message;
  int next_pair;

  while (1) {
    MPI_Recv(&message, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);

    switch (status.MPI_TAG) {
      case TAG_INIT_LOOKUP:
        initiate_lookup(message[1]);
        break;

      case TAG_LAST_CHANCE:
        if (have_data(message[1], C)) {  // si je suis responsable de la clé
          // -> on cherche a renvoyé un tag SUCC à l'initiateur
          /*Enregistre l'id_chord de l'initiateur*/
          int initiateur = message[0];
          /* mise à jour du contenu du message */
          /* message[0] on laisse l'initiateur
           * le message contenait la clé que nous recherchions,
           * nous n'avons plus besoin de cette information
           * Nous remplacons le champ dédié par l'id_chord du responsable
           *
           * On aura donc :
           *   message[0] = id_chord initiateur
           *   message[1] = id_chord responsable */

          // id chord du pair responsable
          message[1] = id_chord;

          // si initiateur est responsable de la clé
          if (message[0] == message[1]) {
            MPI_Send(&message[1], 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
            // send(message, TAG_SUCC, 0);
          } else {
            next_pair = findnext(message[0]);
            send(message, TAG_SUCC, next_pair);
          }

          /* Nous voulons renvoyer l'identité du responsable à l'initiateur
           * or, s'il ne fait pas partie de notre table de routage, nous ne
           * connaissons son identité nous le recherchons donc de la meme
           * maniere que nous faisions dans lookup() (avec findnext()) */

        } else {
          /* La donnée n'est pas présente dans la table, on notifie l'initiateur
           * Le contenu du message est donc :
           *   message[0] = id_chord initiateur
           *   message[1] = NIL car la donnée n'est pas présente dans la DHT */
          message[1] = NIL;

          // si initiateur est responsable de la clé
          if (message[0] == message[1]) {
            MPI_Send(&message[1], 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
            // send(message, TAG_SUCC, 0);
          } else {
            next_pair = findnext(message[0]);
            send(message, TAG_SUCC, next_pair);
          }
        }
        break;

      case TAG_LOOKUP:
        /*Continue de la recherche du responsable de la clé*/
        lookup(message[0], message[1]);
        break;

      case TAG_SUCC:
        /*Responsable trouvé et on lance la recherche de
         *l'initiateur pour qu'il puisse signaler au simulateur */

        if (id_chord == message[0]) {  // si initiateur
          // notifie le simulateur la fin de la recherche et envoie l'id_chord
          // du responsable
          send(message, TAG_SUCC, 0);
          // le seul à pouvoir envoyé un message au simulateur
        } else {
          // il faut retrouver l'initiateur et lui envoyer le message <=> lookup
          next_pair = findnext(message[0]);
          send(message, next_pair, TAG_SUCC);
        }
        break;

      case TAG_TERM:
        // fin de la recherche pour tous
        // MPI_finalize() dans le main
        return;

      default:
        perror("default in lookup()");
        break;
    }
  }
}

/*--------------- Fin des fonctions d'envoie et reception --------------------*/

/*************************************MAIN*************************************/

int main(int argc, char* argv[]) {
  int nb_proc, rang;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

  if (nb_proc != N + 1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  if (rang == 0) {
    simulateur();

    /*Tire aleatoirement un id pair existant*/
    srand(time(NULL));
    int alea_pair = 1 + rand() % (NB_PROC + 1);  // MPI_rank
    /*Tirage aleatoire d'une clé de donnée*/
    int alea_key = rand() % ((int)pow(2, M));

    recherche(alea_pair, alea_key);
    // envoie une recherche
    // attends la réponse
    // se termine
  } else {
    // recupere ses fingers et id_chord
    init();
    // boucle sur la réception de message
    receive();
  }

  MPI_Finalize();
  return 0;
}