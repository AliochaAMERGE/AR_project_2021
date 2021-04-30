#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "./include/header.h"

#define NB_PROC 11     // inconnu des pairs
#define N NB_PROC - 1  // inconnu des pairs
#define M 6

/* TAGS */
#define TAG_INIT 0
#define TAG_IN 1
#define TAG_OUT 2
#define TAG_COLLECTE 3

#define BATTU 0
#define ELU 1

MPI_Status status;
int state, rang, leader, right, left, initiateur, id_chord, size;

int nb_OUT = 0;
int nb_IN = 0;

int k = 0;
int *id_chord_table;

// id_already_used : contient les idChord pour les p-1 ranks, p : rank
int f(int *id_already_used, int p) {
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

void simulateur(void) {
  // fait par le processus 0
  // donne les id chord à chaque noeuds, ainsi que leurs voisins
  // todo : a verifier
  id_chord_table = malloc(sizeof(int) * NB_PROC);
  id_chord_table[0] = -1;
  for (int p = 1; p < NB_PROC; p++) {
    id_chord_table[p] = f(id_chord_table, p);
    MPI_Send(&id_chord_table[p], 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
  }
}

void init(void) {
  MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  left = ((rang - 1) == 0) ? N : (rang - 1);
  right = ((rang + 1) > N) ? 1 : (rang + 1);
  initiateur = rand() % 2;  // todo a définir

  leader = id_chord;

  if (initiateur) {
    initier_etape();
  }
}

void initier_etape(void) {
  // k : etape
  // 2^k : distance

  nb_IN = 0;
  nb_OUT = 0;
  /* Le contenue du message :
   * message[0] = MPI_rank
   * message[1] = distance
   */

  int message[2] = {id_chord, (int)pow(2, k)};
  // envoie d'un message de type OUT au voisin de droit et gauche
  MPI_Send(message, 2, MPI_INT, right, TAG_OUT, MPI_COMM_WORLD);
  printf("-\n");
  MPI_Send(message, 2, MPI_INT, left, TAG_OUT, MPI_COMM_WORLD);
  printf("-\n");
  k++;
}
/*
 * step = pow(2, k) initialement au debut d une etape
 * step-- à chaque message
 * k++ à chaque étape
 */
void receive(void) {
  /*
   * message[0] : id initiateur (leader)
   * message[1] : TTL
   */
  int message[2];

  for (;;) {
    MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);

    switch (status.MPI_TAG) {
      case TAG_IN:

        if (message[0] != id_chord) {
          // si le message n'est pas revenu à bon port
          if (status.MPI_SOURCE == right) {
            // on transmet le message au voisin gauche
            printf("-\n");
            MPI_Send(message, 2, MPI_INT, left, TAG_IN, MPI_COMM_WORLD);
          } else {
            // resp droit
            printf("-\n");
            MPI_Send(message, 2, MPI_INT, right, TAG_IN, MPI_COMM_WORLD);
          }
        } else {
          nb_IN++;
          if (nb_IN == 2) {
            // étape suivante
            // ! le step sera incrémenter dans la fonction initier_etape
            initier_etape();
          }
        }
        break;

      case TAG_OUT:

        if (!(initiateur) || (message[0] > id_chord)) {
          state = BATTU;  // <=> initiateur = 0
          if (message[1] > 1) {
            // si la distance pour les messages de type OUT n'est pas encore
            // arrivé au bout
            message[1] -= 1;  // diminue la distance de 1
            if (status.MPI_SOURCE == right) {
              // s'il s'agit d'un message venant du voisin de droite
              // transmet le message de type OUT au voisin de gauche
              MPI_Send(message, 2, MPI_INT, left, TAG_OUT, MPI_COMM_WORLD);
              printf("-\n");
            } else {
              // transmet de le message de type OUT au voisin de droit
              MPI_Send(message, 2, MPI_INT, right, TAG_OUT, MPI_COMM_WORLD);
              printf("-\n");
            }
          } else {  // la distance est atteinte
            // le type de message change en IN et fait demi tour
            if (status.MPI_SOURCE == right) {
              // s'il s'agit d'un message venant du voisin de droite
              // transmet le message de type IN au voisin de gauche
              MPI_Send(message, 2, MPI_INT, left, TAG_IN, MPI_COMM_WORLD);
              printf("-\n");
            } else {
              // transmet de le message de type IN au voisin de droit
              MPI_Send(message, 2, MPI_INT, right, TAG_IN, MPI_COMM_WORLD);
              printf("-\n");
            }
          }
        } else {
          nb_OUT++;
          if ((message[0] == id_chord) && (nb_OUT == 2)) {
            // si je suis initiateur
            // je passe à l'état ELU et je gagne l'élection YOUHOU :)
            state = ELU;

            size = (int)pow(2, k - 1) - message[1] + 2;

            /* Le contenu du message est :
             * message[0] = id_chord (leader)
             * message[1] = size (size de l'anneau)
             */
            message[0] = rang;
            message[1] = size;
            // je préviens
            printf("-\n");
            MPI_Send(message, 2, MPI_INT, right, TAG_COLLECTE, MPI_COMM_WORLD);

            // Le tableau est ordonné en fonction de MPI_Rank
            id_chord_table = malloc(size * sizeof(int));
            id_chord_table[0] = -1;  // simulateur
            // ajout de son id_chord dans le tableau à l'indice rang
            id_chord_table[rang] = id_chord;
            // envoi au voisin de droit le tableau pour qu'il puisse ajouter son
            // id_chord
            printf("-\n");
            MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                     MPI_COMM_WORLD);
            // attente du tableau rempli de tous les id_Chord
            MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                     MPI_COMM_WORLD, &status);
            printf("-\n");
            // envoi du tableau rempli au voisin de droite
            MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                     MPI_COMM_WORLD);
            // le message est revenu, tout le monde connait les id_chord et donc
            // sa table de fingers
            MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                     MPI_COMM_WORLD, &status);
            // calcul de ma table de finger
            fingers_table();
            return;
            // affichage de ma table de finger
          }
        }

        break;

      case TAG_COLLECTE:
        leader = message[0];
        size = message[1];

        id_chord_table = malloc(size * sizeof(int));
        if (right != leader) {
          // envoie au voisin de droit le leader et la size de l'anneau
          printf("-\n");
          MPI_Send(message, 2, MPI_INT, right, TAG_COLLECTE, MPI_COMM_WORLD);
        }
        // attente de la reception du tableau idChord envoyer par le voisin de
        // gauche
        MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                 MPI_COMM_WORLD, &status);
        // ajout de son id_chord dans le tableau à l'indice rang
        id_chord_table[rang] = id_chord;

        // envoi le tableau modifié au voisin de droite
        printf("-\n");
        MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                 MPI_COMM_WORLD);

        // attente du tableau rempli d'id_chord
        MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                 MPI_COMM_WORLD, &status);
        printf("-\n");
        MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                 MPI_COMM_WORLD);

        fingers_table();

        return;
        // sort de receive puis se termine
        break;

      default:
        printf("Ne doit pas être dans le default \n");
        break;
    }
  }
}

int compare(const void *a, const void *b) {
  int int_a = *((int *)a);
  int int_b = *((int *)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

void fingers_table(void) {
  int fingers[M];
  int tmp_id_chord[size];
  int cle;

  for (int i = 0; i < size; i++) {
    tmp_id_chord[i] = id_chord_table[i];
  }
  qsort(tmp_id_chord, size, sizeof(int), compare);

  for (int j = 0; j < M; j++) {
    cle = (id_chord + (int)pow(2, j)) % ((int)pow(2, M));

    for (int k = 1; k < size; k++) {
      // ordre cyclique
      if (k == size - 1) {
        if (tmp_id_chord[k] >= cle) {
          fingers[j] = tmp_id_chord[k];
        } else {
          fingers[j] = tmp_id_chord[1];
        }
      } else {
        if (tmp_id_chord[k] >= cle) {
          fingers[j] = tmp_id_chord[k];
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
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
    // rien d'autre à faire P0 se termine
  } else {
    // recupere ses fingers et id_chord
    init();
    // boucle sur la réception de message
    receive();
  }

  MPI_Finalize();
  return 0;
}
// TODO : fusion state <-> initiateur