#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 11
#define M 6

#define NIL -1

/* les TAGS */
#define TAG_INIT_LOOKUP 0
#define TAG_LAST_CHANCE 1
#define TAG_LOOKUP 2
#define TAG_SUCC 3
#define TAG_TERM 4

MPI_Status status;
int rank = 0;      //! changer
int id_chord = 0;  // ! changer

int fingers[M][2];

int *C;  // ! changer les données detenu

// * message
// 0 : initiateur (id_chord)
// 1 : key ou responsable(MPI_RANK)

// Routine : send(m) to q :
// send the message m to the pair of CHORD identifier q

void send(int *message, int tag, int dest) {
  MPI_Send(message, 2, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

void receive() {
  int *message;
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
            MPI_Send(message[1], 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
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
            MPI_Send(message[1], 1, MPI_INT, 0, TAG_SUCC, MPI_COMM_WORLD);
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

        if (id_chord == message[0]) { //si initiateur
          // notifie le simulateur la fin de la recherche et envoie l'id_chord du responsable
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

int initiate_lookup(int k) {
  int initiateur_chord = id_chord;
  lookup(initiateur_chord, k);
}

int lookup(int initateur_chord, int k) {
  int message = {initateur_chord, k};
  // MPI_rank du plus grand finger ne dépassant pas k
  int next = findnext(k);

  if (next == NIL) {
    send(message, TAG_LAST_CHANCE, fingers[0][0]);
  } else {
    send(message, TAG_LOOKUP, next);
  }
}

// recherche du plan grand finger qui ne dépasse pas k, on fait appel a app()
// pour respecter l'ordre cyclique.
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

int have_data(int k, int *C) { return 1; }
