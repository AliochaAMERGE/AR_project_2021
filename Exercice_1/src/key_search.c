#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
// [0] : MPI_Rank ~ [1] : Id_chord
int fingers[M][2];
int* C;  // les données gérée par le site courant
// c'est derniere ne sont pas gérée pour le moment

/* ******************* Fonction d'appartenance à [a,b[ ***********************
 *               app vérifie si la clé k appartient à                         *
 *               l'intervalle [a,b[                                           *
 **************************************************************************** */

// k € ] a , b ]
int app(int k, int a, int b) {
  if (a < b) return ((k > a) && (k <= b));
  return (((k >= 0) && (k <= b)) || ((k > a) && (k <= pow(2, M))));
}

/******************************Fonctions Aléatoires****************************/

/* ***************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 **************************************************************************** */

// p : rank
// id_already_used : contient les idChord pour les p-1 ranks, p : rank
int f(int* id_already_used, int p) {
  int alea_chord;
  int used;
  do {
    used = 1;
    alea_chord = (rand() % ((int)pow(2, M)));  // retire une valeur aléatoire

    for (int i = 0; i < p; i++) {
      if (id_already_used[i] == alea_chord) {
        used = 0;
      }
    }
  } while (!used);

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

/*************Initialisation de la DHT par le processus simulateur*************/

void simulateurTD(void) {
  int id_chord[N] = {2, 7, 13, 14, 21, 38, 42, 48, 51, 59};
  int fingers[N][M][2] = {
      {{7, 2}, {7, 2}, {7, 2}, {13, 3}, {21, 5}, {38, 6}},      // 2
      {{13, 3}, {13, 3}, {13, 3}, {21, 5}, {38, 6}, {42, 7}},   // 7
      {{14, 4}, {21, 5}, {21, 5}, {38, 6}, {38, 6}, {48, 8}},   // 13
      {{21, 5}, {21, 5}, {21, 5}, {38, 6}, {38, 6}, {48, 8}},   // 14
      {{38, 6}, {38, 6}, {38, 6}, {38, 6}, {38, 6}, {59, 10}},  // 21
      {{42, 7}, {42, 7}, {42, 7}, {48, 8}, {59, 10}, {7, 2}},   // 38
      {{48, 8}, {48, 8}, {48, 8}, {51, 9}, {59, 10}, {13, 3}},  // 42
      {{51, 9}, {51, 9}, {59, 10}, {59, 10}, {2, 1}, {21, 5}},  // 48
      {{59, 10}, {59, 10}, {59, 10}, {2, 1}, {7, 2}, {21, 5}},  // 51
      {{2, 1}, {2, 1}, {2, 1}, {7, 2}, {13, 3}, {38, 6}}        // 59
  };

  for (int p = 0; p < NB_PROC - 1; p++) {
    MPI_Send(&id_chord[p], 1, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);
    MPI_Send(fingers[p], M * 2, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);
  }
}

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
    id_chord[p] = f(id_chord, p);
  }

  qsort(id_chord, NB_PROC, sizeof(int), compare);
  for (int p = 0; p < NB_PROC; p++) {
    printf("[%d] = %d \t", p, id_chord[p]);
  }
  printf("\n");
  /*Trie dans l'ordre croissant les id_chord pour former l'anneau*/
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
    // pour chaque finger du pair p
    for (int j = 0; j < M; j++) {
      /* clé */
      int cle = (idChord + (int)pow(2, j)) % ((int)pow(2, M));
      int ok = 0;
      for (int i = 1; i < NB_PROC; i++) {
        if (id_chord[i] >= cle) {
          // * MPI RANK
          fingers[j][0] = i;
          // * ID_CHORD
          // le responsable de la cle
          fingers[j][1] = id_chord[i];
          ok = 1;
          break;
        }
      }  // fin recherche pair associé au finger
      if (!ok) {
        fingers[j][0] = 1;
        fingers[j][1] = id_chord[1];
      }
      printf("[%d] = %d \t", p, fingers[j][1]);

    }  // fin for each finger du pair p
    printf("\n");

    // Envoie du tableau fingers au processus p
    MPI_Send(fingers, M * 2, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);

  }  // fin for each processus

  /*------------------Fin de l'initialisation du système------------------*/

}  // fin simulateur

void init() {
  MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  MPI_Recv(fingers, M * 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
}

/*---------------------- Fin de la fonction simulateur -----------------------*/

/************** Recherche d'une clée (lancée par le simulateur) ***************/

void recherche(int pair, int key) {
  // Envoie d'un message à ce pair pour chercher le responsable de cette donnée

  MPI_Send(&key, 1, MPI_INT, pair, TAG_INIT_LOOKUP, MPI_COMM_WORLD);

  /* Attente de la réponse de la pair le responsable de cette donnée */
  int responsable;
  MPI_Recv(&responsable, 1, MPI_INT, pair, TAG_SUCC, MPI_COMM_WORLD, &status);
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

void lookup(int initateur_chord, int k) {
  int message[2] = {initateur_chord, k};
  // MPI_rank du plus grand finger ne dépassant pas k
  int next = findnext(k);

  if (next == NIL) {
    // on doit envoyer un message contenant ;
    // * message[0] = id_chord initiateur
    // * message[1] = id_chord responsable
    MPI_Send(message, 2, MPI_INT, fingers[0][0], TAG_LAST_CHANCE,
             MPI_COMM_WORLD);
    // send(message, TAG_LAST_CHANCE, fingers[0][0]);
  } else {
    // * message[0] = id_chord initiateur
    // * message[1] = clé recherchée
    MPI_Send(message, 2, MPI_INT, next, TAG_LOOKUP, MPI_COMM_WORLD);
  }
}

void initiate_lookup(int k) {
  int initiateur_chord = id_chord;
  lookup(initiateur_chord, k);
}

/* ****************************************************************************
 *               findnext permet de recherche du plan grand finger            *
 *               qui ne dépasse pas k, on fait appel a app()                  *
 *                pour respecter l'ordre cyclique                             *
 *****************************************************************************/

int findnext(int k) {
  // input : clés
  // output : MPI_rank
  for (int i = M - 1; i >= 0; i--) {
    if (app(k, fingers[i][1], id_chord)) {
      return fingers[i][0];
    }
  }
  return NIL;
}

int find_initiateur(int initiateur) {
  // INPUT : id_chord de l'initiateur
  // OUTPUT :retourne le MPI_rank de l'initiateur si connu du processus courant,
  // sinon le fingers le plus proche

  int i = M - 1;

  while (i >= 0) {
    // si initiateur est un finger connu
    // on retourne le finger
    if (fingers[i][1] == initiateur) {
      return fingers[i][0];
    } else {
      // sinon,
      // si nous avons dépassé l'initiateur
      if (app(initiateur, fingers[i][1], id_chord)) {
        return fingers[i - 1][0];
      }
    }
    i--;
  }
}

int have_data(int k, int* C) { return 1; }

/*---------------------- Fin des fonctions de recherche
 * ----------------------*/

/* ****************************************************************************
 *               receive permet de gérer la réception * des messages en
 *fonction des tag                             *
 *****************************************************************************/

void receive() {
  int message[2];
  int next_pair;

  while (1) {
    MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);
    switch (status.MPI_TAG) {
      case TAG_INIT_LOOKUP:
        // initialement le message contient seulement la clé et rien d'autre
        initiate_lookup(message[0]);
        break;

      case TAG_LAST_CHANCE:
        if (have_data(message[1], C)) {  // si je suis responsable de la clé
          // -> on cherche a renvoyé un tag SUCC à l'initiateur
          /*Enregistre l'id_chord de l'initiateur*/
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
          if (message[0] == id_chord) {
            // message[1] = id_chord;
            // MPI_Send(&message[1], 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
            MPI_Send(&id_chord, 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
            // send(message, TAG_SUCC, 0);
          } else {
            // next_pair = findnext(message[0]);
            // message[1] = id_chord;
            next_pair = findnext(message[0]);
            if (next_pair == NIL) {
              MPI_Send(message, 2, MPI_INT, fingers[0][0], TAG_SUCC,
                       MPI_COMM_WORLD);
            } else {
              MPI_Send(message, 2, MPI_INT, next_pair, TAG_SUCC,
                       MPI_COMM_WORLD);
            }
          }

          /* Nous voulons renvoyer l'identité du responsable à l'initiateur
           * or, s'il ne fait pas partie de notre table de routage, nous ne
           * connaissons son identité nous le recherchons donc de la meme
           * maniere que nous faisions dans lookup() (avec findnext()) */

        } else {
          /* La donnée n'est pas présente dans la table, on notifie
           * l'initiateur Le contenu du message est donc : message[0] =
           * id_chord initiateur message[1] = NIL car la donnée n'est pas
           * présente dans la DHT */

          // si initiateur est responsable de la clé
          if (message[0] == message[1]) {
            message[1] = NIL;
            MPI_Send(&message[1], 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
            // send(message, TAG_SUCC, 0);
          } else {
            // next_pair = findnext(message[0]);
            next_pair = findnext(message[1]);
            // send(message, TAG_SUCC, next_pair);
            if (next_pair == NIL) {
              MPI_Send(message, 2, MPI_INT, fingers[0][0], TAG_SUCC,
                       MPI_COMM_WORLD);
            } else {
              MPI_Send(message, 2, MPI_INT, next_pair, TAG_SUCC,
                       MPI_COMM_WORLD);
            }
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

        // ici message = {id_chord de la personne en charge, id_chord}

        if (id_chord == message[0]) {  // si initiateur
          // notifie le simulateur la fin de la recherche et envoie l'id_chord
          // du responsable
          // send(message, TAG_SUCC, 0);
          MPI_Send(&message[1], 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
          // le seul à pouvoir envoyé un message au simulateur
        } else {
          // il faut retrouver l'initiateur et lui envoyer le message <=>
          // lookup
          next_pair = find_initiateur(message[0]);
          MPI_Send(message, 2, MPI_INT, next_pair, TAG_SUCC, MPI_COMM_WORLD);
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

/*--------------- Fin des fonctions d'envoie et reception
 * --------------------*/

/*************************************MAIN*************************************/

int main(int argc, char* argv[]) {
  int nb_proc;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  if (nb_proc != N + 1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  srand(time(NULL));

  if (rank == 0) {
    simulateur();

    /*Tire aleatoirement un id pair existant*/
    int alea_pair = 1 + rand() % (N);  // MPI_rank
    /*Tirage aleatoire d'une clé de donnée*/
    int alea_key = rand() % ((int)pow(2, M) - 1);

    printf("Recherche de : %d\n", alea_key);
    printf("Initiateur : %d\n", alea_pair);

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