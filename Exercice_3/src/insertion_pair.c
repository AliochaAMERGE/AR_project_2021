#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "./include/header.h"

#define NB_PROC 11     // inconnu des pairs
#define N NB_PROC - 1  // inconnu des pairs
#define M 6

/* TAGS */
#define TAG_INIT 0

/* les variables globales */
MPI_Status status;
int rang;
int id_chord;
// [0] : MPI_Rank ~ [1] : Id_chord
int fingers[M][2];
int* C;  // les données gérée par le site courant
// Ces dernieres ne sont pas gérée pour le moment

/* ******************* Fonction d'appartenance à ]a,b] ***********************
 *               app vérifie si la clé k appartient à                         *
 *               l'intervalle ]a,b]                                           *
 **************************************************************************** */

int app(int k, int a, int b) {
  if (a < b) return ((k > a) && (k <= b));
  return (((k >= 0) && (k <= b)) || ((k > a) && (k <= pow(2, M))));
}

/***************************** Fonctions de hachage **************************/

/* ***************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 **************************************************************************** */

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

/****************** Fonction utilitaire pour le tri *************************/

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

/************ Initialisation de la DHT par le processus simulateur ************/

void simulateur(void) {
  /* initialisation des variables */
  int id_chord[NB_PROC];  // tableau des identifiants CHORD

  int inverse[NB_PROC][NB_PROC][2];

  for (int i = 0; i < NB_PROC; i++) {
    for (int j = 0; j < NB_PROC; j++) {
      inverse[i][j][0] = -1;  // Rang_MPI
      inverse[i][j][1] = -1;  // id_chord
    }
  }

  /* Attribution des identifiants CHORD à chaque processus */
  /* Sauf le processus 0 qui correspond au simulateur */

  id_chord[0] = -1;  // simulateur

  for (int p = 1; p < NB_PROC; p++) {
    id_chord[p] = f(id_chord, p);
  }

  /* Trie dans l'ordre croissant les id_chord afin de former l'anneau */
  qsort(id_chord, NB_PROC, sizeof(int), compare);
  printf("(rang, pair) : ");
  for (int p = 0; p < NB_PROC; p++) {
    // affichage des id_chord de chaque proche
    printf("(%d, %d)\t", p, id_chord[p]);
  }
  printf("\n");
  /* Envoi des identifiants chord au pairs du système */
  for (int p = 1; p < NB_PROC; p++) {
    MPI_Send(&id_chord[p], 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
  }

  /* ***************************************************
   * Initialisation des fingers pour chaque processus  *
   *************************************************** */

  /* envoie successif des informations à la création de *
   * chaque ensemble de finger   */

  // pour chaque pairs du systeme (simulateur exclu)
  for (int p = 1; p < NB_PROC; p++) {
    /* tableau des M fingrs */
    int fingers[M][2];

    int idChord = id_chord[p];

    printf("%2d > ", id_chord[p]);

    //! a refaire
    // pour chaque finger du pair p
    for (int j = 0; j < M; j++) {
      /* clé */
      int cle = (idChord + (int)pow(2, j)) % ((int)pow(2, M));
      int ok = 0;
      int i_tmp;
      // recherche pair assicué au finger
      for (int i = 1; i < NB_PROC; i++) {
        if (id_chord[i] >= cle) {
          // MPI RANK
          fingers[j][0] = i;
          // ID_CHORD
          fingers[j][1] = id_chord[i];

          inverse[i][p][0] = p;
          inverse[i][p][1] = id_chord[p];

          ok = 1;
          break;
        }
      }  // fin recherche pair associé au finger
      if (!ok) {
        fingers[j][0] = 1;
        fingers[j][1] = id_chord[1];

        inverse[1][p][0] = p;
        inverse[1][p][1] = id_chord[p];
      }
      printf("%5d", fingers[j][1]);

    }  // fin for each finger du pair p
    printf("\n");

    // Envoie du tableau fingers au processus p
    MPI_Send(fingers, M * 2, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);

  }  // fin for each processus

  for (int p = 1; p < NB_PROC; p++) {
  }

}  // fin simulateur

/********** reception des id_chord et de la liste des fingers  ************/
/*                  sauf pour le noeud que nous insérons                  */

void init() {
  MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  MPI_Recv(fingers, M * 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
}
void receive(void) {}

int main(int argc, char* argv[]) {
  int nb_proc;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  if (nb_proc != N + 1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  srand(getpid());

  if (rang == 0) {
    simulateur();
  } else {
    init();
    receive();
  }
  MPI_Finalize();
  return 0;
}
