#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NB_PROC 11     // inconnu des pairs
#define N NB_PROC - 1  // inconnu des pairs
#define M 6

/* TAGS */
#define TAG_INIT 0
#define TAG_IN 1
#define TAG_OUT 2

#define BATTU 0
#define ELU 1

MPI_Status status;
int k, state, rang, leader, right, left, nb_IN, initiateur, id_chord;
int *id_chord_table;

void simulateur(void) {
  // donne les id chord à chaque noeuds, ainsi que leurs voisins
}

void initier_etape(void) {
  // k : etape
  // 2^k : distance
  if (k == 0) {
    initiateur = 1;
  }
  nb_IN = 0;
  /* Le contenue du message :
   * message[0] = MPI_rank
   * message[1] = distance
   */
  int message[2] = {id_chord, (int)pow(2, k)};
  // envoie d'un message de type OUT au voisin de droit et gauche
  MPI_Send(message, 2, MPI_INT, right, TAG_OUT, MPI_COMM_WORLD);
  MPI_Send(message, 2, MPI_INT, left, TAG_OUT, MPI_COMM_WORLD);
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
  MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
           &status);
  switch (status.MPI_TAG) {
    case TAG_IN:
      if (message[0] != id_chord) {
        // si le message n'est pas revenu à bon port
        if (status.MPI_SOURCE == right) {
          // on transmet le message au voisin gauche
          MPI_Send(message, 2, MPI_INT, left, TAG_IN, MPI_COMM_WORLD);
        } else {
          // resp droit
          MPI_Send(message, 2, MPI_INT, right, TAG_IN, MPI_COMM_WORLD);
        }
      } else {
        nb_IN++;
        if (nb_IN == 2) {
          // étape suivante
          // ! le step sera incrémenter dans la fonction initier_etape
          initier_etape();
        }
      };
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
          } else {
            // transmet de le message de type OUT au voisin de droit
            MPI_Send(message, 2, MPI_INT, right, TAG_OUT, MPI_COMM_WORLD);
          }
        } else {  // la distance est atteinte
          // le type de message change en IN et fait demi tour
          if (status.MPI_SOURCE == right) {
            // s'il s'agit d'un message venant du voisin de droite
            // transmet le message de type IN au voisin de gauche
            MPI_Send(message, 2, MPI_INT, left, TAG_IN, MPI_COMM_WORLD);
          } else {
            // transmet de le message de type IN au voisin de droit
            MPI_Send(message, 2, MPI_INT, right, TAG_IN, MPI_COMM_WORLD);
          }
        }
      } else {
        if (message[0] == id_chord) {  // si je suis initiateur
          state =
              ELU;  // je passe à l'état ELU et je gagne l'élection YOUHOU :)
          // TODO : on sauvegarde le k pour l'allocation du tableau et la
          // recupération
          // TODO : des id  chord
          // TODO : lance la collecte des id chord

          // ! objectif déterminé une taille ou une approximation de la taille
          // ! de l'anneau (pour gérer a taille des messages et du tablea envoyé)
        }
      }

      break;

    default:
      break;
  }
}

void collecte(void) {
  // envoie un message à droite (ou a gauche ?) pour recuperer les id chord de
  // tous
  int id_chord_table[];
}

void fingers_table(void) {
  int fingers[M][2];

  /* finger[0] = successeur,
   * temp_rank est le rank MPI du successeur */

  printf("%2d > ", id_chord);

  // pour chaque finger du pair p
  for (int j = 0; j < M; j++) {
    /* clé */
    int cle = (id_chord + (int)pow(2, j)) % ((int)pow(2, M));
    int ok = 0;
    for (int i = 1; i < NB_PROC; i++) {
      if (id_chord_table[i] >= cle) {
        // MPI RANK
        fingers[j][0] = i;
        // ID_CHORD
        fingers[j][1] = id_chord_table[i];
        ok = 1;
        break;
      }
    }  // fin recherche pair associé au finger
    if (!ok) {
      fingers[j][0] = 1;
      fingers[j][1] = id_chord_table[1];
    }
    printf("%5d", fingers[j][1]);

  }  // fin for each finger du pair p
}

int main(int argc, char const *argv[]) { return 0; }
// TODO : fusion state <-> initiateur