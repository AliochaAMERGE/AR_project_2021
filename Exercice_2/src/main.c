#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* TAGS */
#define TAG_INIT 0
#define TAG_IN 1
#define TAG_OUT 2

#define BATTU 0
#define ELU 1

MPI_Status status;
int k, state, rang, leader, right, left, nb_IN, initiateur;

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
  int message[2] = {rang, (int)pow(2, k)};
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
      if (message[0] != rang) {
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
      if (!(initiateur) || (message[0] > rang)) {
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
          // le type de message change en IN et retour ....
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
        if (message[0] == rang) {  // si je suis initiateur
          state = ELU;  // je passe à l'état ELU et je gagne l'élection YOUHOU :)
          // TODO : lance la collecte des id chord
          
        }
      }

      break;

    default:
      break;
  }
}

void collecte(void) {
  // envoie un message à droite (ou a gauche ?) pour recuperer les id chord de tous
}

void fingers_table(void) {}

int main(int argc, char const *argv[]) { return 0; }
// TODO : fusion state <-> initiateur